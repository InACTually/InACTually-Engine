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

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

#include "cinder/Json.h"
#include "cinder/Log.h"

#include "llm/LLMToolManager.hpp"

namespace act {
	namespace room {
		class RoomManagers;
	}

	namespace llm {

		struct ChatMessage {
			std::string role;
			std::string content;
			std::string thinking;
			std::string toolCallId;
			std::string toolName;
			std::string toolCallsJsonText;      // display only: the raw JSON shown in the GUI
			std::string toolCallsJson;			// serialised tool_calls array for Ollama
			bool        displayOnly = false;

			// Merged streaming segments for display ("thinking"|"content").
			struct Segment { std::string type; std::string text; };
			std::vector<Segment> segments;
		};

		struct LLMResponse {
			std::string content;
			std::string thinking;
			std::string error;
			bool done  = false;
			bool reset = false; // if history should be reset (e.g. after falty tool calls)
		};

		using StreamCallback = std::function<void(const LLMResponse&)>;
		using DoneCallback   = std::function<void(const LLMResponse&)>;

		class LLMConnector {
		public:
			LLMConnector();
			~LLMConnector();

			static std::shared_ptr<LLMConnector> create() { return std::make_shared<LLMConnector>(); }

			void setup(act::room::RoomManagers roomMgrs);

			void setHost(const std::string& host)           { m_host = host; }
			void setPort(int port)                          { m_port = port; }
			void setModel(const std::string& model)         { m_model = model; }
			void setSystemPrompt(const std::string& prompt) { m_systemPrompt = prompt; }

			const std::string&              getHost()            const { return m_host; }
			int                             getPort()            const { return m_port; }
			const std::string&              getModel()           const { return m_model; }
			const std::string&              getSystemPrompt()    const { return m_systemPrompt; }
			const std::vector<std::string>& getAvailableModels() const { return m_availableModels; }

			bool isProcessing() const { return m_isProcessing.load(); }

			void fetchAvailableModels();
			int  getSelectedModelIndex() const { return m_selectedModelIndex; }
			void syncModelIndex();

			const std::vector<ChatMessage>& getChatHistory() const { return m_chatHistory; }
			void pushUserMessage(const std::string& content)     { m_chatHistory.push_back({ "user",      content, "" }); }
			void pushAssistantPlaceholder()                       { m_chatHistory.push_back({ "assistant", "",      "" }); }
			void updateLastAssistant(const std::string& content, const std::string& thinking);
			void clearChatHistory()                               { m_chatHistory.clear(); }

			void addTool(const ToolDefinition& definition, ToolHandler handler) { m_toolManager.add(definition, handler); }
			void addTools(const std::vector<ToolEntry>& entries);
			void clearTools()                                                    { m_toolManager.clear(); }

			// Sends a request using the internal chat history (all messages except the last placeholder).
			void sendRequest(
				const std::string& userMessage,
				StreamCallback     onStream,
				DoneCallback       onDone
			);

			void waitForCompletion();

			virtual ci::Json toJson()          const;
			virtual void     fromJson(const ci::Json& json);

			std::string httpGet(const std::string& host, int port, const std::string& path);
			void httpPostStreaming(
				const std::string& host, int port,
				const std::string& path,
				const std::string& body,
				StreamCallback onStream,
				DoneCallback   onDone
			);

			struct DebugLogEntry {
				enum MsgDirection {
					MD_SEND,         // outgoing request
					MD_RECV_STREAM,  // accumulated streaming content/thinking tokens
					MD_RECV_EVENT    // single structural event: tool_calls, done, error
				};
				MsgDirection    direction;
				std::string		text;   
				std::string		label;
			};

			const std::vector<DebugLogEntry>&	getDebugLog()  const { return m_debugLog; }
			void								clearDebugLog()      { m_debugLog.clear(); }

		private:
			// state for inline <think>...</think> tag parsing across
			struct ThinkState {
				bool        isThinking  = false;	// currently inside a <think> block
				std::string token;					// partial tag bytes from last token
			};

			void handleToolCalls(
				const ci::Json&    toolCallsJsonText,
				const std::string& host, int port,
				StreamCallback     onStream,
				DoneCallback       onDone,
				ThinkState&        thinkState
			);

			void httpPostStreamingImpl(
				const std::string& host, int port,
				const std::string& path,
				const std::string& body,
				StreamCallback onStream,
				DoneCallback   onDone,
				ThinkState&    thinkState
			);

			std::string					m_host         = "127.0.0.1";
			int							m_port         = 11434;
			std::string					m_model        = "";
			std::string					m_systemPrompt = "You are a helpful assistant.";

			std::vector<std::string>	m_availableModels;
			int							m_selectedModelIndex { -1 };

			std::vector<ChatMessage>	m_chatHistory;

			std::thread					m_thread;
			std::atomic<bool>			m_isProcessing { false };

			LLMToolManager				m_toolManager;

			std::vector<DebugLogEntry>	m_debugLog;
		};

		using LLMConnectorRef = std::shared_ptr<LLMConnector>;

	}
}
