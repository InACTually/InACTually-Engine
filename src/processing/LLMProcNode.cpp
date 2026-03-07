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

#include <asio/asio.hpp>

act::proc::LLMProcNode::LLMProcNode() : ProcNodeBase("LLM") {
	m_drawSize = ci::ivec2(500, 700);
	m_model = "";
	m_systemPrompt = "You are a helpful assistant.";
	m_ollamaHost = "127.0.0.1";
	m_ollamaPort = 11434;
	m_isProcessing = false;
	m_isProcessingDone = false;
	m_statusText = "ready";
	m_selectedModelIndex = -1;
	m_showThinking = false;
	m_scrollToBottom = false;

	fetchAvailableModels();

	createTextInput("prompt", [&](std::string msg) {
		if (!m_isProcessing && !msg.empty()) {
			sendRequest(msg);
		}
	});

	m_responsePort = createTextOutput("response");
	m_busyPort = createBoolOutput("busy");
}

act::proc::LLMProcNode::~LLMProcNode() {
	if (m_thread.joinable())
		m_thread.join();
}

void act::proc::LLMProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::LLMProcNode::update() {
	m_busyPort->send(m_isProcessing.load());

	if (m_isProcessing) {
		std::lock_guard<std::mutex> lock(m_streamMutex);
		if (!m_chatHistory.empty() && m_chatHistory.back().role == "assistant") {
			m_chatHistory.back().content = m_streamBuffer;
			m_chatHistory.back().thinking = m_streamThinking;
			m_scrollToBottom = true;
		}
	}

	if (m_isProcessingDone) {
		m_thread.join();
		m_isProcessingDone = false;

		{
			std::lock_guard<std::mutex> lock(m_streamMutex);
			if (!m_chatHistory.empty() && m_chatHistory.back().role == "assistant") {
				m_chatHistory.back().content = m_streamBuffer;
				m_chatHistory.back().thinking = m_streamThinking;
				m_lastResponse = m_streamBuffer;
				m_responsePort->send(m_streamBuffer);
				m_scrollToBottom = true;
			}
			if (!m_streamError.empty()) {
				m_statusText = m_streamError;
			}
			else {
				m_statusText = "ready";
			}
		}

		m_isProcessing = false;
	}
}

void act::proc::LLMProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;

	// Model
	{
		ImGui::SetNextItemWidth(m_drawSize.x - 30);
		if (ImGui::BeginCombo("##model", m_model.c_str())) {
			for (int i = 0; i < (int)m_availableModels.size(); i++) {
				bool isSelected = (m_selectedModelIndex == i);
				if (ImGui::Selectable(m_availableModels[i].c_str(), isSelected)) {
					m_selectedModelIndex = i;
					m_model = m_availableModels[i];
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_SYNC "##refreshModels", ImVec2(22, 0))) {
			fetchAvailableModels();
		}
	}

	// Host & Port
	{
		ImGui::SetNextItemWidth(m_drawSize.x - 80);
		if (ImGui::InputText("##host", &m_ollamaHost)) {
			prvntDrag = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		if (ImGui::InputInt("##port", &m_ollamaPort, 0, 0)) {
			prvntDrag = true;
		}
	}

	// System Prompt
	{
		ImGui::SetNextItemWidth(m_drawSize.x);
		if (ImGui::InputTextMultiline("system", &m_systemPrompt, ImVec2(m_drawSize.x, 60))) {
			prvntDrag = true;
		}
	}

	// Chat History
	{
		ImGui::BeginChild("##chatHistory", ImVec2(m_drawSize.x, 200), true);
		for (int msgIdx = 0; msgIdx < (int)m_chatHistory.size(); msgIdx++) {
			auto& msg = m_chatHistory[msgIdx];
			if (msg.role == "user") {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
				ImGui::TextWrapped("You: %s", msg.content.c_str());
				ImGui::PopStyleColor();
			}
			else if (msg.role == "assistant") {
				if (!msg.thinking.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
					std::string treeLabel = ICON_FA_BRAIN " Thinking##" + std::to_string(msgIdx);
					ImGui::SetNextItemOpen(m_showThinking);
					if (ImGui::TreeNode(treeLabel.c_str())) {
						ImGui::TextWrapped("%s", msg.thinking.c_str());
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, util::Design::primaryColor());
				ImGui::TextWrapped("LLM: %s", msg.content.c_str());
				ImGui::PopStyleColor();
			}
		}
		if (m_isProcessing) {
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
		if (enterPressed || ImGui::IsItemDeactivatedAfterEdit()) {
			prvntDrag = true;
		}
		ImGui::SameLine();
		bool canSend = !m_isProcessing && !m_inputBuffer.empty();
		if (!canSend) ImGui::BeginDisabled();
		if (ImGui::Button("Send", ImVec2(50, 0)) || (enterPressed && canSend)) {
			sendRequest(m_inputBuffer);
			m_inputBuffer.clear();
		}
		if (!canSend) ImGui::EndDisabled();
	}

	// Clear & options
	{
		if (ImGui::Button("Clear Chat", ImVec2(m_drawSize.x - 300, 0))) {
			m_chatHistory.clear();
			m_lastResponse.clear();
		}
		ImGui::SameLine();
		ImGui::Checkbox(ICON_FA_BRAIN " show thinking##showThinking", &m_showThinking);
	}

	// Status
	ImGui::TextDisabled("Status: %s", m_statusText.c_str());

	preventDrag(prvntDrag);

	endNodeDraw();
}

void act::proc::LLMProcNode::sendRequest(const std::string& userMessage) {
	if (m_isProcessing)
		return;

	if (m_thread.joinable())
		m_thread.join();

	m_chatHistory.push_back({ "user", userMessage });
	m_chatHistory.push_back({ "assistant", "" });
	m_isProcessing = true;
	m_statusText = "generating...";
	m_scrollToBottom = true;

	{
		std::lock_guard<std::mutex> lock(m_streamMutex);
		m_streamBuffer.clear();
		m_streamThinking.clear();
		m_streamError.clear();
		m_isThinking = false;
	}

	// Build the JSON request body
	ci::Json messagesJson = ci::Json::array();

	// System prompt
	if (!m_systemPrompt.empty()) {
		ci::Json sysMsg = ci::Json::object();
		sysMsg["role"] = "system";
		sysMsg["content"] = m_systemPrompt;
		messagesJson.push_back(sysMsg);
	}

	// Chat history (skip the empty assistant placeholder at the end)
	for (size_t i = 0; i < m_chatHistory.size() - 1; i++) {
		ci::Json chatMsg = ci::Json::object();
		chatMsg["role"] = m_chatHistory[i].role;
		chatMsg["content"] = m_chatHistory[i].content;
		messagesJson.push_back(chatMsg);
	}

	ci::Json requestBody = ci::Json::object();
	requestBody["model"] = m_model;
	requestBody["messages"] = messagesJson;
requestBody["stream"] = true;

	std::string body = requestBody.dump();
	std::string host = m_ollamaHost;
	int port = m_ollamaPort;

	m_thread = std::thread([this, body, host, port]() {
		try {
			httpPostStreaming(host, port, "/api/chat", body);
		}
		catch (const std::exception& e) {
			std::lock_guard<std::mutex> lock(m_streamMutex);
			m_streamError = std::string("[error] ") + e.what();
		}
		m_isProcessingDone = true;
	});
}

void act::proc::LLMProcNode::httpPostStreaming(const std::string& host, int port, const std::string& path, const std::string& body) {
	asio::io_context io_context;
	asio::ip::tcp::resolver resolver(io_context);
	asio::ip::tcp::socket socket(io_context);

	auto endpoints = resolver.resolve(host, std::to_string(port));
	asio::connect(socket, endpoints);

	std::string request =
		"POST " + path + " HTTP/1.1\r\n"
		"Host: " + host + ":" + std::to_string(port) + "\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: " + std::to_string(body.size()) + "\r\n"
		"Connection: close\r\n"
		"\r\n"
		+ body;

	asio::write(socket, asio::buffer(request));

	std::string rawBuffer;
	asio::error_code ec;
	std::array<char, 8192> buf;
	bool headersSkipped = false;
	bool isChunked = false;
	

	// Buffers for chunked decoding and line assembly
	std::string chunkRemainder;	// leftover raw chunked data
	std::string lineBuffer;		// decoded data waiting for newline

	while (true) {
		size_t len = socket.read_some(asio::buffer(buf), ec);
		if (len > 0)
			rawBuffer.append(buf.data(), len);

		// Skip HTTP headers once
		if (!headersSkipped) {
			auto headerEnd = rawBuffer.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				if (ec) break;
				continue;
			}
			std::string headers = rawBuffer.substr(0, headerEnd);
			// case-insensitive check for chunked
			std::string headersLower = headers;
			std::transform(headersLower.begin(), headersLower.end(), headersLower.begin(), ::tolower);
			isChunked = (headersLower.find("transfer-encoding: chunked") != std::string::npos);
			rawBuffer = rawBuffer.substr(headerEnd + 4);
			headersSkipped = true;
		}

		if (!headersSkipped) {
			if (ec) break;
			continue;
		}

		// Decode data: if chunked, decode chunks; otherwise use raw
		std::string decoded;
		if (isChunked) {
			chunkRemainder += rawBuffer;
			rawBuffer.clear();

			// Decode as many complete chunks as possible
			while (true) {
				auto lineEnd = chunkRemainder.find("\r\n");
				if (lineEnd == std::string::npos)
					break;

				std::string sizeStr = chunkRemainder.substr(0, lineEnd);
				// Trim whitespace
				while (!sizeStr.empty() && (sizeStr.front() == ' ' || sizeStr.front() == '\t'))
					sizeStr.erase(sizeStr.begin());

				if (sizeStr.empty()) {
					// Skip empty lines between chunks
					chunkRemainder = chunkRemainder.substr(lineEnd + 2);
					continue;
				}

				size_t chunkSize = 0;
				try {
				chunkSize = std::stoul(sizeStr, nullptr, 16);
				}
				catch (...) {
					// Not a valid chunk size, skip this line
					chunkRemainder = chunkRemainder.substr(lineEnd + 2);
					continue;
				}

				if (chunkSize == 0) {
					// Final chunk - we're done
					chunkRemainder.clear();
					break;
				}

				// Need: sizeStr\r\n + chunkSize bytes + \r\n
				size_t totalNeeded = lineEnd + 2 + chunkSize + 2;
				if (chunkRemainder.size() < totalNeeded)
					break; // wait for more data

				decoded.append(chunkRemainder, lineEnd + 2, chunkSize);
				chunkRemainder = chunkRemainder.substr(totalNeeded);
			}
		}
		else {
			decoded = rawBuffer;
			rawBuffer.clear();
		}

		// Append decoded data to line buffer and process complete lines
		lineBuffer += decoded;

		bool thinking = false;

		size_t pos;
		while ((pos = lineBuffer.find('\n')) != std::string::npos) {
			std::string line = lineBuffer.substr(0, pos);
			lineBuffer = lineBuffer.substr(pos + 1);

			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			if (line.empty())
				continue;

			try {
				ci::Json chunk = ci::Json::parse(line);

				if (chunk.contains("error")) {
					std::lock_guard<std::mutex> lock(m_streamMutex);
					m_streamError = "[error] " + chunk["error"].get<std::string>();
					return;
				}

				if (chunk.contains("message") && chunk["message"].contains("content")) {
					std::string token = chunk["message"]["content"].get<std::string>();

					std::lock_guard<std::mutex> lock(m_streamMutex);

					if (chunk["message"].contains("thinking")) {
						std::string thinkToken = chunk["message"]["thinking"].get<std::string>();
						if (!thinkToken.empty()) {
							m_isThinking = true;
							m_streamThinking += thinkToken;
						}
						if (!token.empty())
							m_isThinking = false;
					}
					// Handle <think>...</think> tags for thinking models
					if (!thinking && token.find("<think>") != std::string::npos) {
						thinking = true;
						m_isThinking = true;
						auto tagPos = token.find("<think>");
						token = token.substr(tagPos + 7);
					}

					if (thinking && token.find("</think>") != std::string::npos) {
						auto tagPos = token.find("</think>");
						std::string thinkPart = token.substr(0, tagPos);
						std::string afterThink = token.substr(tagPos + 8);
						if (!thinkPart.empty())
							m_streamThinking += thinkPart;
						thinking = false;
						m_isThinking = false;
						if (!afterThink.empty())
							m_streamBuffer += afterThink;
					}
					else if (thinking) {
						m_streamThinking += token;
					}
					else {
						m_streamBuffer += token;
					}
				}

				if (chunk.contains("done") && chunk["done"].get<bool>()) {
					return;
				}
			}
			catch (...) {
				// skip malformed JSON lines
			}
		}

		if (ec == asio::error::eof)
			break;
		if (ec)
			throw asio::system_error(ec);
	}
}

std::string act::proc::LLMProcNode::httpGet(const std::string& host, int port, const std::string& path) {
	asio::io_context io_context;
	asio::ip::tcp::resolver resolver(io_context);
	asio::ip::tcp::socket socket(io_context);

	auto endpoints = resolver.resolve(host, std::to_string(port));
	asio::connect(socket, endpoints);

	std::string request =
		"GET " + path + " HTTP/1.1\r\n"
		"Host: " + host + ":" + std::to_string(port) + "\r\n"
		"Connection: close\r\n"
		"\r\n";

	asio::write(socket, asio::buffer(request));

	std::string response;
	asio::error_code ec;
	std::array<char, 4096> buf;

	while (true) {
		size_t len = socket.read_some(asio::buffer(buf), ec);
		if (len > 0)
			response.append(buf.data(), len);
		if (ec == asio::error::eof)
			break;
		if (ec)
			throw asio::system_error(ec);
	}

	auto headerEnd = response.find("\r\n\r\n");
	if (headerEnd != std::string::npos)
		return response.substr(headerEnd + 4);

	return response;
}

void act::proc::LLMProcNode::fetchAvailableModels() {
	m_availableModels.clear();
	m_selectedModelIndex = -1;

	try {
		std::string response = httpGet(m_ollamaHost, m_ollamaPort, "/api/tags");
		ci::Json json = ci::Json::parse(response);

		if (json.contains("models") && json["models"].is_array()) {
			for (auto& entry : json["models"]) {
				if (entry.contains("name")) {
					m_availableModels.push_back(entry["name"].get<std::string>());
				}
			}
		}
	}
	catch (const std::exception& e) {
		CI_LOG_W("LLMProcNode: failed to fetch models: " << e.what());
	}

	// Try to match current model to the list
	for (int i = 0; i < (int)m_availableModels.size(); i++) {
		if (m_availableModels[i] == m_model) {
			m_selectedModelIndex = i;
			break;
		}
	}

	// If current model not found but list is non-empty, select first
	if (m_selectedModelIndex < 0 && !m_availableModels.empty()) {
		m_selectedModelIndex = 0;
		m_model = m_availableModels[0];
	}
}

ci::Json act::proc::LLMProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["model"] = m_model;
	json["systemPrompt"] = m_systemPrompt;
	json["ollamaHost"] = m_ollamaHost;
	json["ollamaPort"] = m_ollamaPort;

	ci::Json historyJson = ci::Json::array();
	for (auto& msg : m_chatHistory) {
		ci::Json entry = ci::Json::object();
		entry["role"] = msg.role;
		entry["content"] = msg.content;
		if (!msg.thinking.empty())
			entry["thinking"] = msg.thinking;
		historyJson.push_back(entry);
	}
	json["chatHistory"] = historyJson;

	return json;
}

void act::proc::LLMProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "model", m_model);
	util::setValueFromJson(json, "systemPrompt", m_systemPrompt);
	util::setValueFromJson(json, "ollamaHost", m_ollamaHost);
	util::setValueFromJson(json, "ollamaPort", m_ollamaPort);

	m_chatHistory.clear();
	if (json.contains("chatHistory") && json["chatHistory"].is_array()) {
		for (auto& entry : json["chatHistory"]) {
			ChatMessage msg;
			msg.role = entry.value("role", "");
			msg.content = entry.value("content", "");
			msg.thinking = entry.value("thinking", "");
			if (!msg.role.empty())
				m_chatHistory.push_back(msg);
		}
	}
}
