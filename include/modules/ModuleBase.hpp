
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

#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "cinder/Json.h"

#include "RoomManagers.hpp"


namespace act {

	namespace net {
		class NetworkManager;
	}

	namespace mod {

		class ModuleBase
		{
		public:
			ModuleBase() {};

			virtual void setup(act::room::RoomManagersRef roomMgrs, std::shared_ptr<act::net::NetworkManager> networkMgr) = 0;
			virtual void cleanUp() = 0;
			virtual void update() = 0;
			virtual void draw() = 0;

			virtual void drawGUI() = 0;

			virtual void load(std::filesystem::path path) {};
			virtual void save(std::filesystem::path path) {};


			void setName(std::string name) { m_name = name; };
			std::string getName() const { return m_name; };

			virtual ci::Json getFullDescription() = 0;

			bool getIsActive() { return m_isActive; };
			void setIsActive(bool isActive) { m_isActive = isActive; };

		protected:
			std::string m_name;
			bool m_isActive = true;

		}; using ModuleBaseRef = std::shared_ptr<ModuleBase>;

	}
}