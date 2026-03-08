
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

#include "RoomModule.hpp"
#include "cinder/CinderImGui.h"
#include "imnodes.h"
#include "Design.hpp"

act::mod::RoomModule::RoomModule() {
	setName("Room");

	m_iaHelper = ia::InteractionHelper::create();
	m_iaHelper->getMouseListener()->addListener((act::input::MouseListener*)this);


	auto colorShader = ci::gl::getStockShader(ci::gl::ShaderDef().color());

	m_stage = room::Stage::create();

	m_fboSize = vec2(600, 400);
	setupFbo();

	m_lookAt = vec3(0.0f);
	m_camera.setEyePoint(vec3(5.0f, 5.0f, 10.0f));
	m_camera.lookAt(m_lookAt);
	m_camUi = CameraUi(&m_camera);
};

act::mod::RoomModule::~RoomModule() {

}

void act::mod::RoomModule::setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) {
	m_roomMgrs = roomMgrs;
	m_networkMgr = networkMgr;

	act::room::RoomNodeBase::setPublisher(m_networkMgr);

	for (auto&& mgr : m_roomMgrs.list) {
		mgr->setup();
	}

	m_stage->setup(m_roomMgrs);
	fs::path path = app::getAssetPath("recentRoom.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "recentRoom.json";
		writeJson(path, ""); // touch
		saveToFile(path);
	}
	loadFromFile(path);
}

void act::mod::RoomModule::cleanUp() {
	saveToFile(app::getAssetPath("recentRoom.json"));
	m_stage.reset();
}

void act::mod::RoomModule::update() {
	m_stage->update();
}

void act::mod::RoomModule::draw() {
	gl::ScopedFramebuffer fbScp(m_fbo);
	gl::ScopedViewport scpVp(ivec2(0), m_fbo->getSize());

	gl::ScopedMatrices scpMatrices;
	//gl::setMatricesWindow(m_fbo->getSize(), true);
	gl::setMatrices(m_camera);

	gl::clear(util::Design::backgroundColor());
	ci::gl::ScopedColor scpColor(1.0f, 1.0f, 1.0f);
	ci::gl::ScopedLineWidth scpLW(1);

	m_stage->draw();
}

void act::mod::RoomModule::drawGUI() {
	auto darkPrimaryColor = IM_COL32(util::Design::darkPrimaryColor().r * 255, util::Design::darkPrimaryColor().g * 255, util::Design::darkPrimaryColor().b * 255, 255);
	auto primaryColor = IM_COL32(util::Design::primaryColor().r * 255, util::Design::primaryColor().g * 255, util::Design::primaryColor().b * 255, 255);
	auto highlightColor = IM_COL32(util::Design::highlightColor().r * 255, util::Design::highlightColor().g * 255, util::Design::highlightColor().b * 255, 255);

	auto selectedNode = m_stage->getSelectedNode();

	

	ImGui::Begin("RoomNode");
	if (selectedNode) {
		auto nodes = m_stage->getAllNodes();
		std::vector<std::string> nodeNames;
		std::vector<act::UID> nodeUIDs;

		for (auto node : nodes) {
			if(node->getUID()!= selectedNode->getUID()){
				nodeNames.push_back(node->getName());
				nodeUIDs.push_back(node->getUID());
			}
		}
		nodeNames.push_back("None");
		nodeUIDs.push_back("null");
		auto it = find(nodeUIDs.begin(), nodeUIDs.end(), selectedNode->getCopyPosUID());
		m_selectedCopyPos = it - nodeUIDs.begin();
		if (ImGui::Button("remove")) {
			m_stage->removeNode(selectedNode->getUID());
			if (m_node)
				m_node = nullptr;
		}
		/*if (ImGui::Combo("Copy position", &m_selectedCopyPos, nodeNames)) {

			auto uidNode = m_stage->getNodeByUID(nodeUIDs[m_selectedCopyPos]);
			if (uidNode != nullptr) {
				selectedNode->connectPositionPort(uidNode);
				uidNode->sendCurrentPosition();
			}
			else {
				auto followedNode = m_stage->getNodeByUID(selectedNode->getCopyPosUID());
				followedNode->disconnectPositionOutPort(selectedNode->getInputPort());
				selectedNode->setCopyPosUid(m_currentFollowUID);
			}

		}*/
		selectedNode->drawSettings();

	}
	else {
		ImGui::Text("Select a node.");
	}
	ImGui::End();

	drawDevicePool();

	ImGui::Begin("Room Editor");
	handleResize();

	ImGui::Image(m_fbo->getColorTexture(), m_fbo->getSize(), vec2(0, 1), vec2(1, 0));
	isRoomEditorHovered = ImGui::IsItemHovered();

	m_iaHelper->evaluate();

	ImGui::End();
}

void act::mod::RoomModule::load(std::filesystem::path path)
{
	m_stage->clear();
	loadFromFile(path);
}

void act::mod::RoomModule::save(std::filesystem::path path)
{
	saveToFile(path);
}

void act::mod::RoomModule::onMouseDown(ci::app::MouseEvent event)
{
	m_camUi.mouseDown(event);
	updateNodeAtMouse();
}

void act::mod::RoomModule::onMouseMove(ci::app::MouseEvent event)
{
	m_mousePos = event.getPos();
	updateNodeAtMouse();
}

void act::mod::RoomModule::onMouseDrag(ci::app::MouseEvent event)
{
	m_mousePos = event.getPos();

	if (m_node) {
		Ray ray = getMouseRay();
		vec3 pos = ray.calcPosition(ci::distance(m_node->getPosition(), ray.getOrigin()));
		m_node->setPosition(pos);
	}
	else {
		m_camUi.mouseDrag(event);
	}
}

void act::mod::RoomModule::onMouseUp(ci::app::MouseEvent event)
{
	m_camUi.mouseUp(event);
	m_stage->setSelectedNode(m_node);
}

void act::mod::RoomModule::onMouseWheel(ci::app::MouseEvent event)
{
	if (isRoomEditorHovered)
		m_camUi.mouseWheel(event);
}

ci::Json act::mod::RoomModule::toParams()
{
	ci::Json roomJson = ci::Json::object();
	ci::Json cameraJson = ci::Json::object();
	cameraJson["eyepoint"] = util::valueToJson(m_camera.getEyePoint());
	cameraJson["lookAt"] = util::valueToJson(m_lookAt);
	roomJson["camera"] = cameraJson;
	return roomJson;
}

void act::mod::RoomModule::fromParams(ci::Json json)
{
	if (json.contains("camera")) {
		ci::Json cameraJson = json["camera"];

		vec3 vec = m_camera.getEyePoint();
		util::setValueFromJson(cameraJson, "eyepoint", vec);
		m_camera.setEyePoint(vec);

		util::setValueFromJson(cameraJson, "lookAt", m_lookAt);
		m_camera.lookAt(m_lookAt);
	}
}

ci::Ray act::mod::RoomModule::getMouseRay()
{
	vec2 mouse = m_iaHelper->getNormalizedMousePos();
	return m_camera.generateRay(mouse.x, 1.0f - mouse.y, m_camera.getAspectRatio());
}

void act::mod::RoomModule::updateNodeAtMouse()
{
	ci::Ray ray = getMouseRay();

	if (m_node) {
		m_node->setIsHovered(false);
	}
	m_node = m_stage->getNodeOnRay(ray);

	if (m_node) {
		m_node->setIsHovered(true);
	}
}
/*
int act::mod::RoomModule::getNodeIndexByUID(act::UID uid)
{
	auto nodes = m_stage->getNodes();
	auto it = find(nodes.begin(), nodes.end(), [&](act::room::RoomNodeBaseRef node) {	return node->getUID() == uid; });

	if (it != nodes.end())
	{
		return it - nodes.begin();
	}
	else {
		return -1;
	}
}*/

void act::mod::RoomModule::setupFbo()
{
	if (m_fboSize.x == 0 || m_fboSize.y == 0)
		return;

	m_camera.setPerspective(60, m_fboSize.x / m_fboSize.y, 1, 1000);

	gl::Fbo::Format format;
	format.setSamples(4);

	try {
		m_fbo = gl::Fbo::create(m_fboSize.x, m_fboSize.y, format);
	}
	catch (gl::FboExceptionInvalidSpecification exc) {
		CI_LOG_F("Fbo creation failed: " << exc.what());
	}
}

void act::mod::RoomModule::drawDevicePool() {

	ImGui::Begin("RoomNode Pool");

	if (ImGui::Button("clear room")) {

	}

	ImGui::NewLine();
	for (auto&& mgr : m_roomMgrs.list) {
		if (mgr->getName() != "bodyTrackingManager") {

			if (ImGui::CollapsingHeader(mgr->getName().c_str())) {
				//drawCreateButton("RGB-Camera"); // for instance
				auto node = mgr->drawMenu();
				if (node) {
					// m_stage->addNode(node); // Manager do keep track themselves
					m_stage->setSelectedNode(node);
				}
				ImGui::NewLine();
			}
		}
	}
	ImGui::End();
}

void act::mod::RoomModule::drawCreateButton(std::string nodeName) {
	if (ImGui::Button(nodeName.c_str())) {
		auto node = act::room::RoomNodeBaseRegistry::create(nodeName);
		if (node)
			m_stage->addNode(node);
		else {
			CI_LOG_W("Node '" << nodeName << "' was not registered correctly!");
		}
	}
}

void act::mod::RoomModule::handleResize() {
	ImVec2 max = ImGui::GetWindowContentRegionMax();
	ImVec2 min = ImGui::GetWindowContentRegionMin();
	ci::vec2 size = ci::vec2(max.x - min.x, max.y - min.y);
	if (m_fboSize.x != size.x || m_fboSize.y != size.y) {
		m_fboSize = size;
		setupFbo();
		draw();
	}
}


void act::mod::RoomModule::saveToFile(fs::path path) {

	ci::writeJson(path, getFullDescription());
}

ci::Json act::mod::RoomModule::getFullDescription()
{
	auto roomConfiguration = ci::Json::object();

	roomConfiguration["stage"] = m_stage->toJson();
	for (auto&& mgr : m_roomMgrs.list)
		roomConfiguration[mgr->getName()] = mgr->toJson();

	roomConfiguration["roomParams"] = toParams();

	return roomConfiguration;
}

std::vector<std::string> act::mod::RoomModule::getNodeNames()
{
	std::vector<std::string> nodeNames;
	for (auto node : m_stage->getNodes()) {
		nodeNames.push_back(node->getName());
	}
	return nodeNames;
}

void act::mod::RoomModule::loadFromFile(fs::path path) {
	ci::Json roomConfiguration = ci::loadJson(loadFile(path));

	m_stage->fromJson(roomConfiguration["stage"]);
	for (auto&& mgr : m_roomMgrs.list) {
		if (roomConfiguration.contains(mgr->getName()))
			mgr->fromJson(roomConfiguration[mgr->getName()]);
	}
	if (roomConfiguration.contains("roomParams"))
		fromParams(roomConfiguration["roomParams"]);
}


bool act::mod::RoomModule::hasNodeWithUID(act::UID uid)
{
	return !!m_roomMgrs.getRoomNodeByUID(uid);
}

act::room::RoomNodeBaseRef act::mod::RoomModule::createRoomNode(ci::Json data, act::UID replyUID) {
	
	std::string roomNodeName = "";
	util::setValueFromJson(data, "name", roomNodeName);

	act::room::RoomNodeBaseRef node;
	node = roomNodeFactory(roomNodeName, data["params"]);

	if (node) {
		node->fromJson(data, replyUID);
	}
	
	return node;

}
act::room::RoomNodeBaseRef act::mod::RoomModule::roomNodeFactory(std::string roomNodeName, cinder::Json params)
{
	act::room::RoomNodeBaseRef node;

	std::string name = roomNodeName;
	util::setValueFromJson(params, "name", name);

	std::string deviceName = "";
	util::setValueFromJson(params, "deviceName", deviceName);

	if (roomNodeName == "actionspace") {
		node = m_roomMgrs.actionspaceMgr->addActionspace(name);
	}

	if (roomNodeName == "camera") {
		node = m_roomMgrs.cameraMgr->addDevice(deviceName, name);
	}

	if (roomNodeName == "kinect") {
		node = m_roomMgrs.kinectMgr->addDevice(deviceName, name);
	}

	if (roomNodeName == "speaker") {
		int channel = 0;
		util::setValueFromJson(params, "channel", channel);

		node = m_roomMgrs.audioMgr->addSpeaker(channel);
	}
	if (roomNodeName == "subwoofer") {
		int channel = 0;
		util::setValueFromJson(params, "channel", channel);

		node = m_roomMgrs.audioMgr->addSubwoofer(channel);
	}
	if (roomNodeName == "microphone") {
		int channel = 0;
		util::setValueFromJson(params, "channel", channel);

		node = m_roomMgrs.audioMgr->addMicrophone(channel);
	}

	if (roomNodeName == "light") {
		int fixtureIndex = 0;
		util::setValueFromJson(params, "fixtureIndex", fixtureIndex);

		int startAdress = 0;
		util::setValueFromJson(params, "startAdress", startAdress);

		node = m_roomMgrs.dmxMgr->addDevice(name, fixtureIndex, startAdress);
	}

	if (roomNodeName == "lidar") {
		std::string fixtureName = "";
		util::setValueFromJson(params, "fixtureName", fixtureName);

		node = m_roomMgrs.lidarMgr->addDevice(fixtureName);
	}

	return node;
}
bool act::mod::RoomModule::updateRoomNode(ci::Json data, act::UID replyUID) {
	act::UID uid = data["uid"];
	ci::Json params = data["params"];

	// refactor into roomMod (that uses the stage)
	auto roomNode = m_roomMgrs.getRoomNodeByUID(uid);
	if (!roomNode) {
		CI_LOG_E("Could not find RoomNode with UID: " << uid);
		return false;
	}
	roomNode->fromJson(data, replyUID);

	return true;
}
bool act::mod::RoomModule::deleteRoomNode(act::UID uid, act::UID replyUID) {
	bool somethingDeleted = false;
	for (auto&& mgr : m_roomMgrs.list) {
		if (mgr->getNodeByUID(uid) != nullptr) {
			mgr->removeNode(uid);
			somethingDeleted = true;
		}
	}
	return somethingDeleted;
}

bool act::mod::RoomModule::callRPC(act::UID uid, std::string functionName)
{
	auto node = m_roomMgrs.getRoomNodeByUID(uid);
	if (node) {
		return node->call(functionName);
	}
	return false;
}
