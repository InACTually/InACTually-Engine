/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "LLMModule.hpp"
#include "cinder/CinderImGui.h"
#include "Design.hpp"
#include "llm\LLMConnector.hpp"

act::mod::LLMModule::LLMModule() {
	setName("LLM");
	m_statusText = "ready";
}

act::mod::LLMModule::~LLMModule() {
};

void act::mod::LLMModule::setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) {
	m_roomMgrs       = roomMgrs;
	m_networkMgr     = networkMgr;
	m_llmConnector   = llm::LLMConnector::create();

	m_llmConnector->fetchAvailableModels();
	m_llmConnector->setup(roomMgrs);

	fs::path path = app::getAssetPath("recentLLM.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "recentLLM.json";
		ci::writeJson(path, "");
		saveToFile(path);
	}
	loadFromFile(path);
}

void act::mod::LLMModule::cleanUp() {
	saveToFile(app::getAssetPath("recentLLM.json"));
}

void act::mod::LLMModule::update() {
	if (m_isProcessingDone.exchange(false)) {
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_llmConnector->updateLastAssistant(m_streamBuffer, m_streamThinking);
		m_scrollToBottom = true;
		m_statusText = m_streamError.empty() ? "ready" : m_streamError;
	}

	if (m_llmConnector->isProcessing()) {
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_llmConnector->updateLastAssistant(m_streamBuffer, m_streamThinking);
		m_scrollToBottom = true;
	}
}

void act::mod::LLMModule::draw() {
}

void act::mod::LLMModule::drawGUI() {
	ImGui::Begin("LLM");

	auto   models     = m_llmConnector->getAvailableModels();
	int    modelIndex = m_llmConnector->getSelectedModelIndex();
	const std::string& model = m_llmConnector->getModel();

	// Model selector
	ImGui::SetNextItemWidth(-30);
	if (ImGui::BeginCombo("##llmModel", model.c_str())) {
		for (int i = 0; i < (int)models.size(); i++) {
			bool sel = (modelIndex == i);
			if (ImGui::Selectable(models[i].c_str(), sel)) {
				m_llmConnector->setModel(models[i]);
				m_llmConnector->syncModelIndex();
			}
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_SYNC "##llmRefresh"))
		m_llmConnector->fetchAvailableModels();

	// Host & Port
	{
		std::string host = m_llmConnector->getHost();
		int         port = m_llmConnector->getPort();
		if (ImGui::InputText("Host##llmHost", &host))
			m_llmConnector->setHost(host);
		ImGui::SameLine();
		if (ImGui::InputInt("Port##llmPort", &port, 0, 0))
			m_llmConnector->setPort(port);
	}

	// System Prompt
	{
		std::string sysPrompt = m_llmConnector->getSystemPrompt();
		ImGui::Text("System Prompt");
		if (ImGui::InputTextMultiline("##llmSystem", &sysPrompt, ImVec2(-1, 170)))
			m_llmConnector->setSystemPrompt(sysPrompt);
	}

	ImGui::Separator();

	// Chat History
	auto& history = m_llmConnector->getChatHistory();
	ImGui::BeginChild("##llmChat", ImVec2(-1, -270), true);
	for (int i = 0; i < (int)history.size(); i++) {
		auto& msg = history[i];
		if (msg.role == "user") {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			ImGui::TextWrapped("You: %s", msg.content.c_str());
			ImGui::PopStyleColor();
		}
		else if (msg.role == "tool") {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
			std::string treeLabel = ICON_FA_WRENCH " " + msg.toolName + "##llmTool" + std::to_string(i);
			ImGui::SetNextItemOpen(m_showToolUse, ImGuiCond_Always);
			if (ImGui::TreeNode(treeLabel.c_str())) {
				ImGui::TextWrapped("%s", msg.content.c_str());
				ImGui::TreePop();
			}
			ImGui::PopStyleColor();
		}
		else if (msg.role == "assistant" && !msg.displayOnly) {
			for (int s = 0; s < (int)msg.segments.size(); s++) {
				auto& seg = msg.segments[s];
				std::string segId = "##seg" + std::to_string(i) + "_" + std::to_string(s);
				if (seg.type == "thinking" && !seg.text.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
					std::string treeLabel = ICON_FA_BRAIN " Thinking" + segId;
					ImGui::SetNextItemOpen(m_showThinking, ImGuiCond_Always);
					if (ImGui::TreeNode(treeLabel.c_str())) {
						ImGui::TextWrapped("%s", seg.text.c_str());
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();
				}
				else if (seg.type == "content" && !seg.text.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, util::Design::primaryColor());
					ImGui::TextWrapped("LLM: %s", seg.text.c_str());
					ImGui::PopStyleColor();
				}
			}
			// Fallback for messages without segments (e.g. loaded from file)
			if (msg.segments.empty()) {
				if (!msg.thinking.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
					std::string treeLabel = ICON_FA_BRAIN " Thinking##llmThink" + std::to_string(i);
					ImGui::SetNextItemOpen(m_showThinking, ImGuiCond_Always);
					if (ImGui::TreeNode(treeLabel.c_str())) {
						ImGui::TextWrapped("%s", msg.thinking.c_str());
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();
				}
				if (!msg.content.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, util::Design::primaryColor());
					ImGui::TextWrapped("LLM: %s", msg.content.c_str());
					ImGui::PopStyleColor();
				}
			}
		}
		else if (msg.role == "assistant" && msg.displayOnly && !msg.toolCallsJsonText.empty()) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
			std::string treeLabel = ICON_FA_WRENCH " tool call##llmTcReq" + std::to_string(i);
			ImGui::SetNextItemOpen(m_showToolUse, ImGuiCond_Always);
			if (ImGui::TreeNode(treeLabel.c_str())) {
				ImGui::TextWrapped("%s", msg.toolCallsJsonText.c_str());
				ImGui::TreePop();
			}
			ImGui::PopStyleColor();
		}
	}
	if (m_llmConnector->isProcessing()) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
		ImGui::TextWrapped("...");
		ImGui::PopStyleColor();
	}
	if (m_scrollToBottom) {
		ImGui::SetScrollHereY(1.0f);
		m_scrollToBottom = false;
	}
	ImGui::EndChild();

	// Input
	{
		ImGui::SetNextItemWidth(-60);
		bool enter = ImGui::InputText("##llmInput", &m_inputBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		bool canSend = !m_llmConnector->isProcessing() && !m_inputBuffer.empty();
		if (!canSend) ImGui::BeginDisabled();
		if (ImGui::Button("Send") || (enter && canSend)) {
			sendRequest(m_inputBuffer);
			m_inputBuffer.clear();
		}
		if (!canSend) ImGui::EndDisabled();
	}

	if (ImGui::Button("Clear Chat"))
		m_llmConnector->clearChatHistory();
	ImGui::SameLine();
	ImGui::Checkbox(ICON_FA_BRAIN " show thinking##llmShowThink", &m_showThinking);
	ImGui::SameLine();
	ImGui::Checkbox(ICON_FA_WRENCH " expand tools##llmShowTools", &m_showToolUse);
	ImGui::SameLine();
	ImGui::Checkbox(ICON_FA_BUG " wire log##llmShowWireLog", &m_showDebugLog);

	ImGui::TextDisabled("Status: %s", m_statusText.c_str());

	ImGui::End();

	// debug log panel
	if (m_showDebugLog) {
		ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(ICON_FA_BUG " LLM Wire Log", &m_showDebugLog)) {
			if (ImGui::Button("Clear##wireLogClear"))
				m_llmConnector->clearDebugLog();
			ImGui::SameLine();
			ImGui::Checkbox("Auto-scroll##wireLogScroll", &m_debugLogScrollToBottom);

			ImGui::Separator();

			ImGui::BeginChild("##wireLogContent", ImVec2(-1, -1), false);

			auto& log = m_llmConnector->getDebugLog();
			for (int i = 0; i < (int)log.size(); i++) {
				auto& entry = log[i];

				ImVec4 col;
				switch (entry.direction) {
					case act::llm::LLMConnector::DebugLogEntry::MsgDirection::MD_SEND:
						col = util::Design::secondaryColor();
						break;
					case act::llm::LLMConnector::DebugLogEntry::MsgDirection::MD_RECV_STREAM:
						col = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
						break;
					case act::llm::LLMConnector::DebugLogEntry::MsgDirection::MD_RECV_EVENT:
						col = util::Design::primaryColor();
						break;
					default:
						col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				}

				std::string nodeId = entry.label + "##wl" + std::to_string(i);

				ImGui::PushStyleColor(ImGuiCol_Text, col);
				bool open = ImGui::TreeNode(nodeId.c_str());
				ImGui::PopStyleColor();

				if (open) {
					std::string display;
					if (entry.direction == act::llm::LLMConnector::DebugLogEntry::MsgDirection::MD_RECV_STREAM) {
						display = entry.text;
					} else {
						try { display = ci::Json::parse(entry.text).dump(2); }
						catch (...) { display = entry.text; }
					}

					float lineCount = (float)(std::count(display.begin(), display.end(), '\n') + 2);
					float height    = std::min(lineCount * ImGui::GetTextLineHeightWithSpacing(), 400.0f);
					ImGui::InputTextMultiline(
						("##wltext" + std::to_string(i)).c_str(),
						const_cast<char*>(display.c_str()),
						display.size() + 1,
						ImVec2(-1, height),
						ImGuiInputTextFlags_ReadOnly);
					ImGui::TreePop();
				}
			}

			if (m_debugLogScrollToBottom)
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
		}
		ImGui::End();
	}
}

void act::mod::LLMModule::sendRequest(const std::string& userMessage) {
	if (m_llmConnector->isProcessing())
		return;

	m_statusText     = "generating...";
	m_scrollToBottom = true;
	m_llmConnector->pushUserMessage(userMessage);
	m_llmConnector->pushAssistantPlaceholder();

	{
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_streamBuffer.clear();
		m_streamThinking.clear();
		m_streamError.clear();
	}

	m_llmConnector->sendRequest(
		userMessage,
		[this](const act::llm::LLMResponse& token) {
			std::lock_guard<std::mutex> lock(m_streamMutex);
			if (token.reset) {
				m_streamBuffer.clear();
				m_streamThinking.clear();
				return;
			}
			m_streamBuffer   += token.content;
			m_streamThinking += token.thinking;
		},
		[this](const act::llm::LLMResponse& final) {
			std::lock_guard<std::mutex> lock(m_streamMutex);
			m_streamBuffer     = final.content;
			m_streamThinking   = final.thinking;
			m_streamError      = final.error;
			m_isProcessingDone = true;
		}
	);
}

ci::Json act::mod::LLMModule::getFullDescription() {
	return m_llmConnector->toJson();
}

void act::mod::LLMModule::load(std::filesystem::path path) {
	loadFromFile(path);
}

void act::mod::LLMModule::save(std::filesystem::path path) {
	saveToFile(path);
}

void act::mod::LLMModule::loadFromFile(std::filesystem::path path) {
	try {
		m_llmConnector->fromJson(ci::loadJson(loadFile(path)));
	}
	catch (...) {
		CI_LOG_W("[LLMModule] could not load from file: " << path.string());
	}
}

void act::mod::LLMModule::saveToFile(std::filesystem::path path) {
	ci::writeJson(path, getFullDescription());
}
