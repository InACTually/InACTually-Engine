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

#pragma once

#include "ProcNodeBase.hpp"
#include <thread>
#include <atomic>
#include <mutex>

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class LLMProcNode : public ProcNodeBase
		{
		public:
			LLMProcNode();
			~LLMProcNode();

			PROCNODECREATE(LLMProcNode);

			void setup(act::room::RoomManagers roomMgrs)	override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			void sendRequest(const std::string& userMessage);
			void httpPostStreaming(const std::string& host, int port, const std::string& path, const std::string& body);
			std::string httpGet(const std::string& host, int port, const std::string& path);
			void fetchAvailableModels();

			std::string m_model;
			std::string m_systemPrompt;
			std::string m_ollamaHost;
			int			m_ollamaPort;

			std::vector<std::string>	m_availableModels;
			int							m_selectedModelIndex;

			struct ChatMessage {
				std::string role;
				std::string content;
				std::string thinking;
			};
			std::vector<ChatMessage>	m_chatHistory;
			std::string					m_inputBuffer;
			std::string					m_lastResponse;
			std::string					m_statusText;

			std::thread					m_thread;
			std::atomic<bool>			m_isProcessing;
			std::atomic<bool>			m_isProcessingDone;
			std::mutex					m_streamMutex;
			std::string					m_streamBuffer;
			std::string					m_streamThinking;
			std::string					m_streamError;
			bool						m_isThinking;
			bool						m_showThinking;
			bool						m_scrollToBottom;

			OutputPortRef<std::string>	m_responsePort;
			OutputPortRef<bool>			m_busyPort;

		}; using LLMProcNodeRef = std::shared_ptr<LLMProcNode>;

	}
}
