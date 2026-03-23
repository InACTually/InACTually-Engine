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

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "RoomManagers.hpp"
#include "NetworkManager.hpp"
#include "ModuleBase.hpp"
#include "llm/LLMConnector.hpp"

#include <atomic>
#include <mutex>

using namespace ci;
using namespace ci::app;

namespace act {
	namespace mod {

		class LLMModule : public ModuleBase
		{
		public:
			LLMModule();
			~LLMModule();

			void setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) override;
			void cleanUp()	override;
			void update()	override;
			void draw()		override;
			void drawGUI()	override;

			ci::Json getFullDescription() override;

			void load(std::filesystem::path path) override;
			void save(std::filesystem::path path) override;

		protected:
			act::room::RoomManagers		m_roomMgrs;
			act::net::NetworkManagerRef	m_networkMgr;

			act::llm::LLMConnectorRef	m_llmConnector;

			std::string					m_inputBuffer;
			std::string					m_statusText;
			bool						m_scrollToBottom			= false;
			bool						m_showThinking				= false;
			bool						m_showToolUse				= false;
			bool						m_showDebugLog				= false;
			bool						m_debugLogScrollToBottom	= true;

			std::mutex					m_streamMutex;
			std::string					m_streamBuffer;
			std::string					m_streamThinking;
			std::string					m_streamError;
			std::atomic<bool>			m_isProcessingDone = false;

			void sendRequest(const std::string& userMessage);
			void loadFromFile(std::filesystem::path path);
			void saveToFile(std::filesystem::path path);
		};

		using LLMModuleRef = std::shared_ptr<LLMModule>;

	}
}
