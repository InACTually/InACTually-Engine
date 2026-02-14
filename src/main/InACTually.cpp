
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "main/InACTually.hpp"
#include "Logger.hpp"

#include <opencv2/core/ocl.hpp>
#include <cinder/CinderImGui.h>
#include <imgui_impl_opengl3.h>
#include "implot.h"
#include "imnodes.h"

#include "ModuleBase.hpp"
#include "WindowData.hpp"

using namespace act;

GeneralAppState AppState::m_state = AS_STARTUP;

InACTually::InACTually(ci::app::App* app)
	: m_app(app)
{
	AppState::set(AS_STARTUP);
	getWindow()->setUserData(new WindowData());
	m_mainWindowUID = getWindow()->getUserData<WindowData>()->getUID();
	getWindow()->setBorderless();
	ivec2 size = ivec2(400, 200);
	getWindow()->setSize(size);
	getWindow()->setPos((m_app->getDisplay()->getSize() / 2) - (size / 2));
	getWindow()->setTitle("InACTually");

	getWindow()->getSignalClose().connect([&]() {
		onClose();
	});

	m_splashScreenTex = gl::Texture::create(*Surface::create(loadImage(getAssetPath("design/splash.png"))));
}

InACTually::~InACTually()
{
}

void InACTually::init()
{
	util::Logger::initialize();

	CI_LOG_I("\n##############################\n\n" << "InACTually has been started :)" << "\n\n##############################\n\n");

	
	CI_LOG_I("# InACTually "
	<< "has OpenCl " << cv::ocl::haveOpenCL() << "has CUDA " << cv::cuda::getCudaEnabledDeviceCount() << "\n"
	<< "has AMD FFT " << cv::ocl::haveAmdFft() << ", has AMD BLAS " << cv::ocl::haveAmdBlas() << ", has SVM " << cv::ocl::haveSVM() 
	<< "\n");

	if (cv::ocl::haveOpenCL()) {
		cv::ocl::setUseOpenCL(cv::ocl::haveOpenCL());

		std::vector<cv::ocl::PlatformInfo> plattformInfo;
		cv::ocl::getPlatfomsInfo(plattformInfo);
		CI_LOG_D("OpenCL Platform: " << plattformInfo[0].deviceNumber() << " - " << plattformInfo[0].name());
	}

	m_drawGUI = Settings::get().showDebugGUI;
	m_prevDrawGUI = m_drawGUI;
	m_drawDebug = false;



	auto options = ImGui::Options().window(app::getWindow());
	//options(true);
	Initialize(options);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	initStyle();

	ImNodes::CreateContext();
	//ImNodes::LoadCurrentEditorStateFromIniFile(app::getAssetPath("editor.ini").string().c_str());

	ImPlot::CreateContext();
	//ImPlot::GetStyle().AntiAliasedLines = true;
	ImPlot::PushColormap(ImPlotColormap_Deep);

	m_roomMgrs.positionMgr		= room::PositionManager::create();
	m_roomMgrs.cameraMgr		= room::CameraManager::create();
	m_roomMgrs.dmxMgr			= room::DMXManager::create();
	m_roomMgrs.kinectMgr		= room::KinectManager::create();
	m_roomMgrs.markerMgr		= room::MarkerManager::create(m_roomMgrs.cameraMgr);
	m_roomMgrs.objectMgr		= room::ObjectManager::create(m_roomMgrs.cameraMgr);
	m_roomMgrs.audioMgr			= room::AudioManager::create();
	m_roomMgrs.displayMgr		= room::DisplayManager::create();
	m_roomMgrs.computerMgr		= room::ComputerManager::create();
	m_roomMgrs.actionspaceMgr	= room::ActionspaceManager::create();
	m_roomMgrs.projectorMgr		= room::ProjectorManager::create();

	m_roomMgrs.bodyTrackingMgr	= room::BodyTrackingManager::create(m_roomMgrs.kinectMgr);

	m_roomMgrs.list.push_back(m_roomMgrs.actionspaceMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.positionMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.cameraMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.projectorMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.dmxMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.kinectMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.bodyTrackingMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.markerMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.objectMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.audioMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.displayMgr);
	m_roomMgrs.list.push_back(m_roomMgrs.computerMgr);

	m_networkMgr = net::NetworkManager::create(m_roomMgrs);
	m_networkMgr->setup();

	for (auto&& module : reg_modules) {
		module->setup(m_roomMgrs, m_networkMgr);
	}

	//getWindow()->setSize(getDisplay()->getSize() - ivec2(0, 70));
	//getWindow()->setPos(ivec2(0, 70));

	//getWindow()->setFullScreen(true);

	ci::ivec2 size = Settings::get().debugGUISize;
	if (size.x == 0 || size.y == 0) {
		size = vec2(m_app->getDisplay()->getSize()) * 0.85f;
		Settings::get().debugGUISize = size;
		Settings::save();
	}
	
	if (!m_drawGUI) {
		size = Settings::get().guiSize;
	}

	getWindow()->setSize(size);
	getWindow()->setPos((m_app->getDisplay()->getSize() / 2) - (size / 2));
	getWindow()->setBorderless(false);

	//m_interactionMgr = make_shared<ia::InteractionManager>();
	//m_inputMgr = make_shared<input::InputManager>(m_interactionMgr);

	//m_mouseRawListener->addListener(m_inputMgr->getMouseRawListener());
	//m_keyRawListener->addListener(m_inputMgr->getKeyRawListener());
	//m_touchRawListener->addListener(m_inputMgr->getTouchRawListener());


	//m_moduleMgr = make_shared<mod::ModuleManager>();

	AppState::set(AS_RUNNING);

	resize();	

	m_initCallback();
}

void act::InACTually::onClose()
{
	AppState::set(AS_CLEANUP);

	for (auto&& mod : reg_modules)
		mod->cleanUp();

	ImPlot::DestroyContext();

	//ImNodes::SaveCurrentEditorStateToIniFile(app::getAssetPath("editor.ini").string().c_str());
	ImNodes::DestroyContext();

	for (auto&& mgr : m_roomMgrs.list)
		mgr->cleanUp();
}

void InACTually::cleanup()
{
}

void InACTually::update()
{
	auto windowData = getWindow()->getUserData<WindowData>();
	if (windowData->getUID() != m_mainWindowUID) {
		windowData->update();
		return;
	}

	if (AppState::get() == AS_RUNNING) {
		m_networkMgr->update();
		
		for (auto&& mgr : m_roomMgrs.list)
			mgr->update();

		for (auto module : reg_modules) {
			if (module->getIsActive()) {
				module->update();
			}
		}

		if (m_isGettingOpenPath) { // callback needs to be set, optimistic
			m_isGettingOpenPath = false;  
			m_getPathCallback(ci::app::getOpenFilePath("", { "json" }));
		}
		if (m_isGettingSavePath) {// callback needs to be set, optimistic
			m_isGettingSavePath = false;
			m_getPathCallback(ci::app::getSaveFilePath("", { "json" }));
		}
	}
	if (AppState::get() == AS_FEATURETEST) {

	}
}

void InACTually::draw()
{
	getWindow()->getRenderer()->makeCurrentContext(true);

	if (AppState::get() == AS_INITIALISING || AppState::get() == AS_STARTUP) {
		gl::clear(Color::gray(0.0f));
		gl::color(Color::white());
		Rectf destRect = Rectf(m_splashScreenTex->getBounds()).getCenteredFit(getWindowBounds(), true).scaledCentered(1.0f);
		gl::draw(m_splashScreenTex, destRect);

		if (AppState::get() == AS_STARTUP && getElapsedFrames() > 2) {
			AppState::set(AS_INITIALISING);
			init();
		}
		return;
	}

	auto windowData = getWindow()->getUserData<WindowData>();
	if (windowData->getUID() != m_mainWindowUID) {
		
		windowData->draw();
		return;
	}


	gl::clear(util::Design::backgroundColor());
	gl::color(Color::white());
	gl::enableAlphaBlending();

	gl::enableDepth();
	gl::enable(GL_VERTEX_PROGRAM_POINT_SIZE);

	gl::pushMatrices();
	ImGui::PushFont(m_font);

	if (m_drawGUI) {
		drawFullGUI();
	}
	else {
		drawMinimalGUI();
	}

	ImGui::PopFont();
	gl::popMatrices();
}

void act::InACTually::drawMinimalGUI()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu(ICON_FA_COG " Settings")) {
		int fontSize = act::Settings::get().fontSize;
		ImGui::SetNextItemWidth(190 - (90 - (3 * fontSize)));

		drawShowGUIMenuEntry();

		if (ImGui::InputInt("FontSize", &fontSize, 1)) {
			fontSize = std::clamp(fontSize, 10, 30);
			act::Settings::get().fontSize = fontSize;
			act::Settings::save();
			m_font->Scale = (float)act::Settings::get().fontSize / 30.0f;
		}

		ImGui::EndMenu();
	}

	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100);
	std::stringstream fps;
	fps << "fps: " << (int)m_app->getAverageFps();
	ImGui::Text(fps.str().c_str(), vec2(10, 10));

	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
	m_networkMgr->drawStatusBar();

	ImGui::EndMainMenuBar();
}

void act::InACTually::drawFullGUI()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	if (AppState::get() == AS_FEATURETEST) {
		gl::pushMatrices();
		// test something

		gl::popMatrices();
	}
	else {

		{
			gl::pushMatrices();
			for (auto module : reg_modules) {
				if (module->getIsActive()) {
					module->draw();
				}
			}
			gl::popMatrices();
		}

		gl::color(Color::white());


		if (m_drawDebug) {
			gl::pushMatrices();
			// getCurrentModule()->drawDebug();

			gl::color(Color::white());
			gl::drawLine(vec2(0, 0), vec2(50, 0));
			gl::drawLine(vec2(0, 0), vec2(0, 50));

			gl::popMatrices();
		}


		// draw "gui"

		if (m_drawDebug) {
			//m_inputMgr->drawDebug();
		}
	}



	// Menu Bar
	//ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu(ICON_FA_FILE_ALT " File")) {
		int fontSize = act::Settings::get().fontSize;
		ImGui::SetNextItemWidth(190 - (90 - (3 * fontSize)));

		if (ImGui::Button(ICON_FA_UPLOAD " open Room")) {
			m_getPathCallback = [&](std::filesystem::path path) {
				ModuleRegistry::getModuleByName("Room")->load(path);
				};
			m_isGettingOpenPath = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_UPLOAD " open Project")) {
			m_getPathCallback = [&](std::filesystem::path path) {
				ModuleRegistry::getModuleByName("Processing")->load(path);
				};
			m_isGettingOpenPath = true;
		}

		if (ImGui::Button(ICON_FA_DOWNLOAD " save Room")) {
			m_getPathCallback = [&](std::filesystem::path path) {
				ModuleRegistry::getModuleByName("Room")->save(path);
				};
			m_isGettingSavePath = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_DOWNLOAD " save Project")) {
			m_getPathCallback = [&](std::filesystem::path path) {
				ModuleRegistry::getModuleByName("Processing")->save(path);
				};
			m_isGettingSavePath = true;
		}

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu(ICON_FA_COG " Settings")) {
		int fontSize = act::Settings::get().fontSize;
		ImGui::SetNextItemWidth(190 - (90 - (3 * fontSize)));

		drawShowGUIMenuEntry();

		if (ImGui::InputInt("FontSize", &fontSize, 1)) {
			fontSize = std::clamp(fontSize, 10, 30);
			act::Settings::get().fontSize = fontSize;
			act::Settings::save();
			m_font->Scale = (float)act::Settings::get().fontSize / 30.0f;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu(ICON_FA_TASKS " Module")) {
		for (int i = 0; i < reg_modules.size(); i++) {
			bool checked = reg_modules[i]->getIsActive();
			if (ImGui::MenuItem(reg_modules[i]->getName().c_str(), nullptr, &checked)) {
				reg_modules[i]->setIsActive(checked);
			}
		}

		ImGui::EndMenu();
	}

	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100);
	std::stringstream fps;
	fps << "fps: " << (int)m_app->getAverageFps();
	ImGui::Text(fps.str().c_str(), vec2(10, 10));

	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
	m_networkMgr->drawStatusBar();

	ImGui::EndMainMenuBar();

	ImGui::SetNextWindowPos(ImVec2(.0f, act::Settings::get().fontSize + 12.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(getWindowWidth(), getWindowHeight() - (act::Settings::get().fontSize + 12.0f)), ImGuiCond_Always);
	ImGui::Begin("main", 0, ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));


	for (auto module : reg_modules) {
		if (module->getIsActive()) {
			module->drawGUI();
		}
	}

	ImGui::End();
}

void act::InACTually::drawShowGUIMenuEntry()
{
	ImGui::Checkbox("show GUI", &m_drawGUI);
	if (m_prevDrawGUI != m_drawGUI) {
		m_prevDrawGUI = m_drawGUI;
		Settings::get().showDebugGUI = m_drawGUI;
		Settings::save();

		ci::ivec2 size = Settings::get().guiSize;
		if (m_drawGUI) {
			size = Settings::get().debugGUISize;
		}
		getWindow()->setSize(size);
		getWindow()->setPos((m_app->getDisplay()->getSize() / 2) - (size / 2));
	}
}

void InACTually::fileDrop(FileDropEvent event)
{
	ci::fs::path				filePath = event.getFile(0);

	ci::audio::SourceFileRef	source;
	ci::Channel32fRef			channel;

	try {
		source = audio::load(ci::loadFile(filePath), audio::Context::master()->getSampleRate());
	}
	catch (...) {
		try {
			channel = ci::Channel32f::create(ci::loadImage(filePath));
		}
		catch (...) {
			return;
		}
	};

	if (source) {

	}
	else if (channel) {

	}

	// do something with the file

	auto windowData = getWindow()->getUserData<WindowData>();
	if (windowData->getUID() != m_mainWindowUID) {
		//windowData->fileDrop();
		return;
	}
}

void InACTually::resize() const
{
	if (AppState::get() == AS_RUNNING) {

		auto windowData = getWindow()->getUserData<WindowData>();
		if (!windowData)
			return;
		if (windowData->getUID() != m_mainWindowUID) {
			windowData->resize();
			return;
		}

		if (m_drawGUI) {
			Settings::get().debugGUISize = app::getWindowSize();
			Settings::save();
		}
	}
}

void InACTually::initStyle() {

	

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	ImFontConfig config;
	config.SizePixels = 30;
	config.OversampleH = config.OversampleV = 2;
	config.PixelSnapH = true;

	io.Fonts->AddFontFromFileTTF(app::getAssetPath("3rd//fonts//OpenSans-Medium.ttf").string().c_str(), config.SizePixels, &config);
	
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; 
	icons_config.MergeMode = true; 
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = 22.0f;
	m_font = io.Fonts->AddFontFromFileTTF(app::getAssetPath("3rd//fonts//fa-solid-900.ttf").string().c_str(), config.SizePixels-8, &icons_config, icons_ranges);

	m_font->Scale = (float)act::Settings::get().fontSize / 30.0f;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 0;

	ImVec4 bg = ImVec4(util::Design::backgroundColor().r, util::Design::backgroundColor().g, util::Design::backgroundColor().b, 1.000f);
	ImVec4 primary = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 1.000f);
	ImVec4 highlight = ImVec4(util::Design::highlightColor().r, util::Design::highlightColor().g, util::Design::highlightColor().b, 1.000f);
	ImVec4 secondary = ImVec4(util::Design::secondaryColor().r, util::Design::secondaryColor().g, util::Design::secondaryColor().b, 1.000f);
	ImVec4 additional = ImVec4(util::Design::additionalColor().r, util::Design::additionalColor().g, util::Design::additionalColor().b, 1.000f);

	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = bg;
	colors[ImGuiCol_FrameBgHovered] = bg;
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = primary;
	colors[ImGuiCol_CheckMark] = primary;
	colors[ImGuiCol_SliderGrab] = primary;
	colors[ImGuiCol_SliderGrabActive] = highlight;
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.45f, 0.49f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.78f);
	colors[ImGuiCol_SeparatorActive] = primary;
	colors[ImGuiCol_ResizeGrip] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.45f, 0.49f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.35f, 0.39f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = secondary;
	colors[ImGuiCol_PlotHistogram] = primary;
	colors[ImGuiCol_PlotHistogramHovered] = highlight;
	colors[ImGuiCol_TextSelectedBg] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.35f);
	colors[ImGuiCol_DragDropTarget] = secondary;
	colors[ImGuiCol_NavHighlight] = primary;
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(util::Design::primaryColor().r, util::Design::primaryColor().g, util::Design::primaryColor().b, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.PopupRounding = 3;
	style.WindowPadding = ImVec2(4, 4);

	int paddingX = round(act::Settings::get().fontSize * (2.0f / 3.0f));
	int paddingY = round(paddingX * 0.625f);

	style.FramePadding = ImVec2(paddingX, paddingY);
	style.ItemSpacing = ImVec2(6, 2);
	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = is3D;

	style.WindowRounding = 3;
	style.ChildRounding = 3;
	style.FrameRounding = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK
	style.TabBorderSize = is3D;
	style.TabRounding = 3;

	colors[ImGuiCol_DockingEmptyBg] = bg;
	colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif
}