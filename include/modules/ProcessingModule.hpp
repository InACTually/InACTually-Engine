/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

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

#include "ProcNodeRegistry.hpp"
#include "ContainerProcNode.hpp"


namespace act {
	namespace mod {

		class ProcessingModule : public ModuleBase
		{
		public:
 
			ProcessingModule();
			~ProcessingModule();

			void setup(act::room::RoomManagersRef roomMgrs, act::net::NetworkManagerRef networkMgr)	override;
			void cleanUp()	override;
			void update()	override;
			void draw()		override;

			void drawGUI()	override;

			ci::Json getFullDescription() override;

			void load(std::filesystem::path path) override;
			void save(std::filesystem::path path) override;

			bool hasNodeWithUID(act::UID uid);
			proc::ProcNodeBaseRef getNodeByUID(act::UID uid);
			proc::ContainerProcNodeRef getContainerByContainingNode(act::UID uid);
			proc::ProcNodeBaseRef createNodeByName(std::string nodeName);
			void deleteNodeByUID(act::UID uid);

			proc::ContainerProcNodeRef createContainerByName(std::string name);
			void deleteContainerByUID(act::UID uid);
			proc::ContainerProcNodeRef getContainerByUID(act::UID uid);
			proc::ContainerProcNodeRef getContainerByName(std::string name);

			bool callRPC(act::UID uid, std::string functionName);

			void connect(proc::PortBaseRef from, proc::PortBaseRef to);

		protected:
			std::vector<proc::ContainerProcNodeRef>				m_containers;

			act::room::RoomManagersRef								m_roomMgrs;
			act::net::NetworkManagerRef							m_networkMgr;

			std::shared_ptr<act::proc::ProcNodeRegistry>		m_nodeRegistry;

			proc::ContainerProcNodeRef							m_rootContainerNode;

			proc::ContainerProcNode*							m_focusedContainerNode;
			std::function<void(proc::ContainerProcNode*)>		m_onFocusCallback;
			
			std::string											m_newGroupName{ "Group" };

			void drawNodePool();
			void drawCreateButton(std::string nodeName);
			void loadFromFile(ci::fs::path path);
			void saveToFile(ci::fs::path path);
			void connect(int from, int to);
			std::pair<act::UID, std::string> getNodeUIDAndPortName(std::string str);
		};

		using ProcessingModuleRef = std::shared_ptr<ProcessingModule>;

	}
}