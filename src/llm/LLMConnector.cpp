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

#include "llm/LLMConnector.hpp"
#include "llm/LLMLightTools.hpp"
#include "RoomManagers.hpp"

#include <asio/asio.hpp>
#include <algorithm>
#include <array>
#include <stdexcept>


namespace {

// Removes or replaces incomplete/invalid UTF-8 byte sequences
static std::string sanitizeUtf8(const std::string& s) {
	std::string out;
	out.reserve(s.size());
	size_t i = 0;
	while (i < s.size()) {
		unsigned char c = static_cast<unsigned char>(s[i]);
		int seqLen = 0;
		if      (c < 0x80)              seqLen = 1;  // ASCII
		else if ((c & 0xE0) == 0xC0)    seqLen = 2;
		else if ((c & 0xF0) == 0xE0)    seqLen = 3;
		else if ((c & 0xF8) == 0xF0)    seqLen = 4;
		else { ++i; continue; }          // invalid lead byte

		// check that all continuation bytes are present and valid
		if (i + seqLen > s.size()) break; // incomplete sequence at end
		bool valid = true;
		for (int k = 1; k < seqLen; ++k) {
			if ((static_cast<unsigned char>(s[i + k]) & 0xC0) != 0x80) {
				valid = false;
				break;
			}
		}
		if (valid) {
			out.append(s, i, seqLen);
			i += seqLen;
		} else {
			++i; // skip bad lead byte
		}
	}
	return out;
}

static std::string safeDump(const ci::Json& j) {
	return j.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
}

} 

static ci::Json createMessage(
	const std::string&                      systemPrompt,
	const std::vector<act::llm::ChatMessage>& history,
	bool                                    excludeLast)
{
	ci::Json msgs = ci::Json::array();

	if (!systemPrompt.empty()) {
		ci::Json sys   = ci::Json::object();
		sys["role"]    = "system";
		sys["content"] = systemPrompt;
		msgs.push_back(sys);
	}

	size_t count = excludeLast && !history.empty() ? history.size() - 1 : history.size();
	for (size_t i = 0; i < count; i++) {
		auto& msg = history[i];
		if (msg.displayOnly) continue;

		ci::Json entry   = ci::Json::object();
		entry["role"]    = msg.role;
		entry["content"] = msg.content;
		if (!msg.toolCallId.empty())
			entry["tool_call_id"] = msg.toolCallId;
		if (!msg.toolCallsJson.empty()) {
			try {
				ci::Json toolCalls = ci::Json::parse(msg.toolCallsJson);
				for (auto& tc : toolCalls) {
					if (!tc.contains("result"))
						tc["result"] = nullptr;
				}
				entry["tool_calls"] = toolCalls;
			}
			catch (...) {}
		}
		msgs.push_back(entry);
	}
	return msgs;
}

act::llm::LLMConnector::LLMConnector() = default;
act::llm::LLMConnector::~LLMConnector() {
	waitForCompletion();
}

void act::llm::LLMConnector::waitForCompletion() {
	if (m_thread.joinable())
		m_thread.join();
}

void act::llm::LLMConnector::syncModelIndex() {
	m_selectedModelIndex = -1;
	for (int i = 0; i < (int)m_availableModels.size(); i++) {
		if (m_availableModels[i] == m_model) {
			m_selectedModelIndex = i;
			break;
		}
	}
	if (m_selectedModelIndex < 0 && !m_availableModels.empty())
		m_selectedModelIndex = 0;
}

void act::llm::LLMConnector::updateLastAssistant(const std::string& content, const std::string& thinking) {
	if (m_chatHistory.empty() || m_chatHistory.back().role != "assistant")
		return;

	auto& msg = m_chatHistory.back();
	msg.content  = sanitizeUtf8(content);
	msg.thinking = sanitizeUtf8(thinking);

	// Update segments incrementally:
	// - If the last segment is of the same type, replace its text (accumulation tracked externally).
	// - If the type changes (e.g. thinking → content), append a new segment.
	// - Never clear segments here – tool-call boundaries already separated them into distinct entries.

	auto upsertSegment = [](std::vector<ChatMessage::Segment>& segs,
	                        const std::string& type, const std::string& text) {
		if (text.empty()) return;
		if (!segs.empty() && segs.back().type == type)
			segs.back().text = text;
		else
			segs.push_back({ type, text });
	};

	if (!msg.thinking.empty()) upsertSegment(msg.segments, "thinking", msg.thinking);
	if (!msg.content.empty())  upsertSegment(msg.segments, "content",  msg.content);
}

void act::llm::LLMConnector::setup(act::room::RoomManagers roomMgrs) {
	auto lightTools = LLMLightTools::getToolDefinitions(roomMgrs);
	m_toolManager.add(lightTools);
}

void act::llm::LLMConnector::addTools(const std::vector<ToolEntry>& entries) {
	m_toolManager.add(entries);
}

void act::llm::LLMConnector::handleToolCalls(
	const ci::Json&		toolCallsJsonText,
	const std::string&	host, 
	int					port,
	StreamCallback		onStream,
	DoneCallback		onDone,
	ThinkState&			thinkState)
{

	std::string toolCallsStr = safeDump(toolCallsJsonText);

	ChatMessage assistantMsg;
	assistantMsg.role              = "assistant";
	assistantMsg.toolCallsJson = toolCallsStr;
	m_chatHistory.push_back(assistantMsg);

	ChatMessage assistantDisplayMsg;
	assistantDisplayMsg.role          = "assistant";
	assistantDisplayMsg.toolCallsJsonText = toolCallsStr;
	assistantDisplayMsg.displayOnly   = true;
	assistantDisplayMsg.segments.push_back({ "toolcall", toolCallsStr });
	m_chatHistory.push_back(assistantDisplayMsg);

	auto dispatchResults = m_toolManager.dispatch(toolCallsJsonText);

	for (auto& dispatchResult : dispatchResults) {
		ChatMessage toolMsg;
		toolMsg.role       = "tool";
		toolMsg.content    = dispatchResult.result;
		toolMsg.toolCallId = dispatchResult.id;
		toolMsg.toolName   = dispatchResult.name;
		toolMsg.segments.push_back({ "content", dispatchResult.result });
		m_chatHistory.push_back(toolMsg);
	}

	ChatMessage postToolPlaceholder;
	postToolPlaceholder.role = "assistant";
	m_chatHistory.push_back(postToolPlaceholder);

	// tokens don't get appended to pre-tool content.
	if (onStream) {
		LLMResponse resetToken;
		resetToken.reset = true;
		onStream(resetToken);
	}

	ci::Json body    = ci::Json::object();
	body["model"]    = m_model;
	body["messages"] = createMessage(m_systemPrompt, m_chatHistory, true);
	body["stream"]   = true;

	ci::Json allTools = m_toolManager.getToolsDescription();
	if (!allTools.empty())
		body["tools"] = allTools;

	std::string bodyStr = safeDump(body);

	LLMResponse finalResponse;
	try {
		httpPostStreamingImpl(host, port, "/api/chat", bodyStr, onStream,
			[&](const LLMResponse& r) { finalResponse = r; }, thinkState);
	}
	catch (const std::exception& e) {
		finalResponse.error = std::string("[error] ") + e.what();
	}
	if (onDone) 
		onDone(finalResponse);
}

void act::llm::LLMConnector::fetchAvailableModels() {
	m_availableModels.clear();
	try {
		std::string response = httpGet(m_host, m_port, "/api/tags");
		ci::Json json = ci::Json::parse(response);
		if (json.contains("models") && json["models"].is_array()) {
			for (auto& entry : json["models"]) {
				if (entry.contains("name"))
					m_availableModels.push_back(entry["name"].get<std::string>());
			}
		}
	}
	catch (const std::exception& e) {
		CI_LOG_W("[LLMConnector] fetchAvailableModels failed: " << e.what());
	}

	bool found = false;
	for (auto& name : m_availableModels) {
		if (name == m_model) { found = true; break; }
	}
	if (!found && !m_availableModels.empty())
		m_model = m_availableModels[0];

	syncModelIndex();
}

void act::llm::LLMConnector::sendRequest(
	const std::string& userMessage,
	StreamCallback     onStream,
	DoneCallback       onDone)
{
	if (m_isProcessing)
		return;

	waitForCompletion();
	m_isProcessing = true;

	ci::Json body    = ci::Json::object();
	body["model"]    = m_model;
	body["messages"] = createMessage(m_systemPrompt, m_chatHistory, true);
	body["stream"]   = true;

	{
		ci::Json allTools = m_toolManager.getToolsDescription();
		if (!allTools.empty())
			body["tools"] = allTools;
	}

	std::string bodyStr = safeDump(body);
	std::string host    = m_host;
	int         port    = m_port;

	m_thread = std::thread([this, bodyStr, host, port, onStream, onDone]() {
		LLMResponse finalResponse;
		try {
			httpPostStreaming(host, port, "/api/chat", bodyStr, onStream, [&](const LLMResponse& r) {
				finalResponse = r;
			});
		}
		catch (const std::exception& e) {
			finalResponse.error = std::string("[error] ") + e.what();
		}
		if (onDone)
			onDone(finalResponse);
		m_isProcessing = false;
	});
}

void act::llm::LLMConnector::httpPostStreaming(
	const std::string& host, 
	int port,
	const std::string& path,
	const std::string& body,
	StreamCallback onStream,
	DoneCallback   onDone)
{
	ThinkState thinkState;
	httpPostStreamingImpl(host, port, path, body, onStream, onDone, thinkState);
}

void act::llm::LLMConnector::httpPostStreamingImpl(
	const std::string& host, 
	int port,
	const std::string& path,
	const std::string& body,
	StreamCallback onStream,
	DoneCallback   onDone,
	ThinkState&    thinkState)
{
	{
		std::string label = ">> SEND  " + host + ":" + std::to_string(port) + path;
		m_debugLog.push_back({ DebugLogEntry::MsgDirection::MD_SEND, body, label });
	}

	asio::io_context io;
	asio::ip::tcp::resolver resolver(io);
	asio::ip::tcp::socket   socket(io);

	asio::connect(socket, resolver.resolve(host, std::to_string(port)));

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
	bool isChunked      = false;
	std::string chunkRemainder;
	std::string lineBuffer;
	LLMResponse accumulated;

	while (true) {
		size_t len = socket.read_some(asio::buffer(buf), ec);
		if (len > 0)
			rawBuffer.append(buf.data(), len);

		if (!headersSkipped) {
			auto headerEnd = rawBuffer.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				if (ec) break;
				continue;
			}
			std::string headers = rawBuffer.substr(0, headerEnd);
			std::string headersLower = headers;
			std::transform(headersLower.begin(), headersLower.end(), headersLower.begin(), ::tolower);
			isChunked  = (headersLower.find("transfer-encoding: chunked") != std::string::npos);
			rawBuffer  = rawBuffer.substr(headerEnd + 4);
			headersSkipped = true;
		}

		std::string decoded;
		if (isChunked) {
			chunkRemainder += rawBuffer;
			rawBuffer.clear();
			while (true) {
				auto lineEnd = chunkRemainder.find("\r\n");
				if (lineEnd == std::string::npos) break;
				std::string sizeStr = chunkRemainder.substr(0, lineEnd);
				while (!sizeStr.empty() && (sizeStr.front() == ' ' || sizeStr.front() == '\t'))
					sizeStr.erase(sizeStr.begin());
				if (sizeStr.empty()) {
					chunkRemainder = chunkRemainder.substr(lineEnd + 2);
					continue;
				}
				size_t chunkSize = 0;
				try { chunkSize = std::stoul(sizeStr, nullptr, 16); }
				catch (...) { chunkRemainder = chunkRemainder.substr(lineEnd + 2); continue; }
				if (chunkSize == 0) { chunkRemainder.clear(); break; }
				size_t totalNeeded = lineEnd + 2 + chunkSize + 2;
				if (chunkRemainder.size() < totalNeeded) break;
				decoded.append(chunkRemainder, lineEnd + 2, chunkSize);
				chunkRemainder = chunkRemainder.substr(totalNeeded);
			}
		}
		else {
			decoded   = rawBuffer;
			rawBuffer.clear();
		}

		lineBuffer += decoded;

		size_t pos;
		while ((pos = lineBuffer.find('\n')) != std::string::npos) {
			std::string line = lineBuffer.substr(0, pos);
			lineBuffer = lineBuffer.substr(pos + 1);
			if (!line.empty() && line.back() == '\r') line.pop_back();
			if (line.empty()) continue;

			try {
				ci::Json chunk = ci::Json::parse(line);

				// bundle streaming tokens, log events individually
				bool isStreamToken =
					chunk.contains("message") &&
					!chunk["message"].contains("tool_calls") &&
					!(chunk.contains("done") && chunk["done"].get<bool>()) &&
					!chunk.contains("error");

				if (isStreamToken) {
					// Append token content to the last RecvStream entry, or create one
					std::string tokenContent;
					if (chunk["message"].contains("content"))
						tokenContent += chunk["message"]["content"].get<std::string>();
					if (chunk["message"].contains("thinking"))
						tokenContent += chunk["message"]["thinking"].get<std::string>();

					if (!m_debugLog.empty() && m_debugLog.back().direction == DebugLogEntry::MsgDirection::MD_RECV_STREAM) {
						m_debugLog.back().text  += tokenContent;
					} else {
						m_debugLog.push_back({ DebugLogEntry::MsgDirection::MD_RECV_STREAM, tokenContent, "<< RECV stream" });
					}
				} else {
					std::string evtLabel = "<< RECV ";
					if (chunk.contains("error"))                                    evtLabel += "error";
					else if (chunk["message"].contains("tool_calls"))               evtLabel += "tool_calls";
					else if (chunk.contains("done") && chunk["done"].get<bool>())   evtLabel += "done";
					else                                                            evtLabel += "event";
					m_debugLog.push_back({ DebugLogEntry::MsgDirection::MD_RECV_EVENT, line, evtLabel });
				}

				if (chunk.contains("error")) {
					accumulated.error = "[error] " + chunk["error"].get<std::string>();
					if (onDone) onDone(accumulated);
					return;
				}

				// model wants to call one or more tools
				if (chunk.contains("message") && chunk["message"].contains("tool_calls")) {
					accumulated.done = true;
					handleToolCalls(chunk["message"]["tool_calls"], host, port, onStream, onDone, thinkState);
					return;
				}

				LLMResponse token;
				if (chunk.contains("message") && chunk["message"].contains("content")) {
					std::string t = sanitizeUtf8(chunk["message"]["content"].get<std::string>());

					if (chunk["message"].contains("thinking")) {
						std::string thinkTok = sanitizeUtf8(chunk["message"]["thinking"].get<std::string>());
						if (!thinkTok.empty()) {
							accumulated.thinking += thinkTok;
							token.thinking        = thinkTok;
						}
					}

					// Only parse inline <think> tags if the model does not use the native thinking field
					if (!chunk["message"].contains("thinking") || chunk["message"]["thinking"].get<std::string>().empty()) {
						std::string pending = thinkState.token + t;
						thinkState.token.clear();
						t.clear();

						while (!pending.empty()) {
							if (!thinkState.isThinking) {
								auto startPos = pending.find("<think>");
								if (startPos != std::string::npos) {
									t      += pending.substr(0, startPos);
									pending = pending.substr(startPos + 7);
									thinkState.isThinking = true;
								} else {
									// check if the tail is a partial "<think>" prefix
									static constexpr std::string_view tag = "<think>";
									std::size_t matchLen = 0;
									for (std::size_t k = 1; k <= std::min(pending.size(), tag.size() - 1); ++k) {
										if (pending.substr(pending.size() - k) == tag.substr(0, k))
											matchLen = k;
									}
									if (matchLen > 0) {
										t                  += pending.substr(0, pending.size() - matchLen);
										thinkState.token    = pending.substr(pending.size() - matchLen);
									} else {
										t += pending;
									}
									pending.clear();
								}
							} else {
								// inside <think> block
								auto endPos = pending.find("</think>");
								if (endPos != std::string::npos) {
									std::string thinkPart = pending.substr(0, endPos);
									accumulated.thinking     += thinkPart;
									token.thinking           += thinkPart;
									pending                   = pending.substr(endPos + 8);
									thinkState.isThinking        = false;
								} else {
									// check for partial </think> at end
									static constexpr std::string_view etag = "</think>";
								 std::size_t matchLen = 0;
								 for (std::size_t k = 1; k <= std::min(pending.size(), etag.size() - 1); ++k) {
										if (pending.substr(pending.size() - k) == etag.substr(0, k))
											matchLen = k;
									}
									if (matchLen > 0) {
										std::string thinkPart = pending.substr(0, pending.size() - matchLen);
										accumulated.thinking += thinkPart;
										token.thinking       += thinkPart;
										thinkState.token      = pending.substr(pending.size() - matchLen);
									} else {
										accumulated.thinking += pending;
										token.thinking       += pending;
									}
									pending.clear();
								}
							}
						}
					}

					accumulated.content += t;
					token.content        = t;
				}

				bool done = chunk.contains("done") && chunk["done"].get<bool>();
				token.done = done;
				if (onStream) onStream(token);

				if (done) {
					accumulated.done = true;
					if (onDone) onDone(accumulated);
					return;
				}
			}
			catch (...) { /* skip malformed JSON lines */ }
		}

		if (ec == asio::error::eof) break;
		if (ec) throw asio::system_error(ec);
	}

	accumulated.done = true;
	if (onDone) onDone(accumulated);
}

std::string act::llm::LLMConnector::httpGet(const std::string& host, int port, const std::string& path) {
	asio::io_context io;
	asio::ip::tcp::resolver resolver(io);
	asio::ip::tcp::socket   socket(io);

	asio::connect(socket, resolver.resolve(host, std::to_string(port)));

	std::string request =
		"GET " + path + " HTTP/1.1\r\n"
		"Host: " + host + ":" + std::to_string(port) + "\r\n"
		"Connection: close\r\n"
		"\r\n";

	asio::write(socket, asio::buffer(request));

	std::string raw;
	asio::error_code ec;
	std::array<char, 4096> buf;
	while (true) {
		size_t len = socket.read_some(asio::buffer(buf), ec);
		if (len > 0) raw.append(buf.data(), len);
		if (ec == asio::error::eof) break;
		if (ec) throw asio::system_error(ec);
	}

	auto headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return raw;

	std::string headers = raw.substr(0, headerEnd);
	std::string body    = raw.substr(headerEnd + 4);

	std::string headersLower = headers;
	std::transform(headersLower.begin(), headersLower.end(), headersLower.begin(), ::tolower);
	if (headersLower.find("transfer-encoding: chunked") == std::string::npos)
		return body;

	std::string decoded;
	size_t pos = 0;
	while (pos < body.size()) {
		auto lineEnd = body.find("\r\n", pos);
		if (lineEnd == std::string::npos) break;

		std::string sizeStr = body.substr(pos, lineEnd - pos);
		auto semi = sizeStr.find(';');
		if (semi != std::string::npos) sizeStr = sizeStr.substr(0, semi);

		size_t chunkSize = 0;
		try { chunkSize = std::stoul(sizeStr, nullptr, 16); }
		catch (...) { break; }

		if (chunkSize == 0) break; // last chunk

		pos = lineEnd + 2; // skip \r\n after size
		if (pos + chunkSize > body.size()) break;

		decoded.append(body, pos, chunkSize);
		pos += chunkSize + 2; // skip chunk data + trailing \r\n
	}
	return decoded;
}

ci::Json act::llm::LLMConnector::toJson() const {
	ci::Json json        = ci::Json::object();
	json["host"]         = m_host;
	json["port"]         = m_port;
	json["model"]        = m_model;
	json["systemPrompt"] = sanitizeUtf8(m_systemPrompt);

	ci::Json historyJson = ci::Json::array();
	for (auto& msg : m_chatHistory) {
		ci::Json entry   = ci::Json::object();
		entry["role"]    = msg.role;
		entry["content"] = sanitizeUtf8(msg.content);
		if (!msg.thinking.empty())
			entry["thinking"] = sanitizeUtf8(msg.thinking);
		if (!msg.toolCallId.empty())
			entry["tool_call_id"] = msg.toolCallId;
		historyJson.push_back(entry);
	}
	json["chatHistory"] = historyJson;
	return json;
}

void act::llm::LLMConnector::fromJson(const ci::Json& json) {
	util::setValueFromJson(json, "host", m_host);
	util::setValueFromJson(json, "port", m_port);
	util::setValueFromJson(json, "model", m_model);
	util::setValueFromJson(json, "systemPrompt", m_systemPrompt);

	m_chatHistory.clear();
	if (json.contains("chatHistory") && json["chatHistory"].is_array()) {
		for (auto& entry : json["chatHistory"]) {
			ChatMessage msg;
			util::setValueFromJson(entry, "role", msg.role);
			util::setValueFromJson(entry, "content", msg.content);
			util::setValueFromJson(entry, "thinking", msg.thinking);
			util::setValueFromJson(entry, "tool_call_id", msg.toolCallId);
			if (!msg.role.empty())
				m_chatHistory.push_back(msg);
		}
	}

	syncModelIndex();
}
