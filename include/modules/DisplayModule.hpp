
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"

#include "RoomManagers.hpp"
#include "NetworkManager.hpp"
#include "ModuleBase.hpp"

#include "ContainerProcNode.hpp"


namespace act {
	namespace mod {

		class DisplayModule : public ModuleBase
		{
		public:
 
			DisplayModule();
			~DisplayModule();

			void setup(act::room::RoomManagersRef roomMgrs, act::net::NetworkManagerRef networkMgr)	override;
			void cleanUp()	override;
			void update()	override;
			void draw()		override;

			void drawGUI()	override;

			ci::Json getFullDescription() override;


		protected:
		
			act::room::RoomManagersRef	m_roomMgrs;
			act::net::NetworkManagerRef m_networkMgr;

			void loadFromFile(ci::fs::path path);
			void saveToFile(ci::fs::path path);

		};

		using MonitorModuleRef = std::shared_ptr<DisplayModule>;

	}
}