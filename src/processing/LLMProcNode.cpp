/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars.engeln.de
*/

#include "procpch.hpp"
#include "LLMProcNode.hpp"

act::proc::LLMProcNode::LLMProcNode() : ProcNodeBase("LLM") {
	m_drawSize   = ci::ivec2(500, 700);
	m_statusText = "ready";

	m_llmConnector  = act::llm::LLMConnector::create();

	createTextInput("prompt", [&](std::string msg) {
		if (!m_llmConnector->isProcessing() && !msg.empty())
			sendRequest(msg);
	});

	m_responsePort = createTextOutput("response");
	m_busyPort     = createBoolOutput("busy");
}

act::proc::LLMProcNode::~LLMProcNode() {
	m_llmConnector->waitForCompletion();
}

void act::proc::LLMProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_llmConnector->setup(roomMgrs);
}

void act::proc::LLMProcNode::update() {
	m_busyPort->send(m_llmConnector->isProcessing());

	if (m_isProcessingDone.exchange(false)) {
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_llmConnector->updateLastAssistant(m_streamBuffer, m_streamThinking);
		m_lastResponse = m_streamBuffer;
		m_responsePort->send(m_streamBuffer);
		m_scrollToBottom = true;
		m_statusText = m_streamError.empty() ? "ready" : m_streamError;
	}

	if (m_llmConnector->isProcessing()) {
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_llmConnector->updateLastAssistant(m_streamBuffer, m_streamThinking);
		m_scrollToBottom = true;
	}
}

void act::proc::LLMProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;
	auto& models     = m_llmConnector->getAvailableModels();
	int   modelIndex = m_llmConnector->getSelectedModelIndex();
	const std::string& currentModel = m_llmConnector->getModel();

	// Model selector
	{
		ImGui::SetNextItemWidth(m_drawSize.x - 30);
		if (ImGui::BeginCombo("##model", currentModel.c_str())) {
			for (int i = 0; i < (int)models.size(); i++) {
				bool isSelected = (modelIndex == i);
				if (ImGui::Selectable(models[i].c_str(), isSelected)) {
					m_llmConnector->setModel(models[i]);
					m_llmConnector->syncModelIndex();
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_SYNC "##refreshModels", ImVec2(22, 0)))
			m_llmConnector->fetchAvailableModels();
	}

	{
		std::string host = m_llmConnector->getHost();
		int         port = m_llmConnector->getPort();
		ImGui::SetNextItemWidth(m_drawSize.x - 80);
		if (ImGui::InputText("##host", &host))
			m_llmConnector->setHost(host);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		if (ImGui::InputInt("##port", &port, 0, 0))
			m_llmConnector->setPort(port);
	}

	{
		std::string sysPrompt = m_llmConnector->getSystemPrompt();
		ImGui::SetNextItemWidth(m_drawSize.x);
		if (ImGui::InputTextMultiline("system", &sysPrompt, ImVec2(m_drawSize.x, 60))) {
			m_llmConnector->setSystemPrompt(sysPrompt);
			prvntDrag = true;
		}
	}

	{
		auto& history = m_llmConnector->getChatHistory();
		ImGui::BeginChild("##chatHistory", ImVec2(m_drawSize.x, 200), true);
		for (int idx = 0; idx < (int)history.size(); idx++) {
			auto& msg = history[idx];
			if (msg.role == "user") {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
				ImGui::TextWrapped("You: %s", msg.content.c_str());
				ImGui::PopStyleColor();
			}
			else if (msg.role == "tool") {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
				std::string treeLabel = ICON_FA_WRENCH " " + msg.toolName + "##llmTool" + std::to_string(idx);
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
					std::string segId = "##seg" + std::to_string(idx) + "_" + std::to_string(s);
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
						std::string treeLabel = ICON_FA_BRAIN " Thinking##" + std::to_string(idx);
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
				std::string treeLabel = ICON_FA_WRENCH " tool call##llmTcReq" + std::to_string(idx);
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
	}

	// Input
	{
		ImGui::SetNextItemWidth(m_drawSize.x - 60);
		bool enterPressed = ImGui::InputText("##input", &m_inputBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		bool canSend = !m_llmConnector->isProcessing() && !m_inputBuffer.empty();
		if (!canSend) ImGui::BeginDisabled();
		if (ImGui::Button("Send", ImVec2(50, 0)) || (enterPressed && canSend)) {
			sendRequest(m_inputBuffer);
			m_inputBuffer.clear();
		}
		if (!canSend) ImGui::EndDisabled();
	}

	{
		if (ImGui::Button("Clear Chat", ImVec2(m_drawSize.x - 300, 0))) {
			m_llmConnector->clearChatHistory();
			m_lastResponse.clear();
		}
		ImGui::SameLine();
		ImGui::Checkbox(ICON_FA_BRAIN " show thinking##showThinking", &m_showThinking);
		ImGui::SameLine();
		ImGui::Checkbox(ICON_FA_WRENCH " expand tools##showTools", &m_showToolUse);
	}

	ImGui::TextDisabled("Status: %s", m_statusText.c_str());

	preventDrag(prvntDrag);
	endNodeDraw();
}

void act::proc::LLMProcNode::sendRequest(const std::string& userMessage) {
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

ci::Json act::proc::LLMProcNode::toParams() {
	return m_llmConnector->toJson();
}

void act::proc::LLMProcNode::fromParams(ci::Json json) {
	m_llmConnector->fromJson(json);
}
