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
#include "llm/LLMConnector.hpp"

#include <mutex>
#include <atomic>

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
			act::llm::LLMConnectorRef	m_llmConnector;

			std::string					m_inputBuffer;
			std::string					m_lastResponse;
			std::string					m_statusText;

			// Stream accumulation (written from worker thread)
			std::mutex					m_streamMutex;
			std::string					m_streamBuffer;
			std::string					m_streamThinking;
			std::string					m_streamError;
			std::atomic<bool>			m_isProcessingDone	= false;
			bool						m_showThinking		= false;
			bool						m_showToolUse		= false;
			bool						m_scrollToBottom	= false;

			OutputPortRef<std::string>	m_responsePort;
			OutputPortRef<bool>			m_busyPort;

			void sendRequest(const std::string& userMessage);
		}; using LLMProcNodeRef = std::shared_ptr<LLMProcNode>;

	}
}
