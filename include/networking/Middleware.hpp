
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ModuleBase.hpp"
#include "ModuleRegistry.hpp"

#include "RoomManagers.hpp"
#include "RoomModule.hpp"
#include "ProcessingModule.hpp"

#include "Connection.hpp"
#include "Message.hpp"


namespace act {
	namespace net {

		class Middleware : public act::mod::ModuleRegistry
		{
		public:
			Middleware(act::room::RoomManagersRef roomMgrs);
			~Middleware();

			static std::shared_ptr<Middleware> create(act::room::RoomManagersRef roomMgrs) { return std::make_shared<Middleware>(roomMgrs); }

			void		update();
			void		draw();


			ci::Json	toJson();
			void		fromJson(ci::Json json);

			void		recieveJson(ci::Json json, ConnectionProviderRef sender);
			void		introduceSender(ConnectionProviderRef sender);


			ci::Json	requestProcNodeTypes(act::UID msgUID);
			void		createProcNodeTypeData();
			ci::Json	getRoomDescription(act::UID msgUID = "");
			ci::Json	getProcDescription(act::UID msgUID = "");
			ci::Json	getFullDescription(act::UID msgUID = "");

			ci::Json	callRPC(act::UID msgUID, act::UID uid, std::string functionName);

			ci::Json	createProcNode(act::UID msgUID, std::string nodeName);
			ci::Json	deleteProcNode(act::UID uid);
			void		connectProcNodes(act::UID fromUID, std::string outputName, act::UID toUID, std::string inputName);
			ci::Json	disconnectProcNodes(act::UID fromUID, std::string outputName, act::UID toUID, std::string inputName);
			ci::Json	getParameterOfProcNode(act::UID msgUID, act::UID uid);
			void		setParameterOfProcNode(act::UID uid, ci::Json params);
			void		subscribeToProcNode(act::UID uid, std::string valueName, ConnectionProviderRef sender);
			void		unsubscribeFromProcNode(act::UID uid, std::string valueName);
			
			ci::Json	createRoomNode(act::UID msgUID, ci::Json data);
			ci::Json	updateRoomNode(act::UID msgUID, ci::Json data);
			ci::Json	deleteRoomNode(act::UID msgUID, act::UID uid);

			ci::Json	uploadAudio(act::UID msgUID, act::UID uid, std::string fileName, std::string fileData);
			

		private:

			std::string						m_text;

			act::room::RoomManagersRef		m_roomMgrs;
			act::mod::ProcessingModuleRef	m_procMod;
			act::mod::RoomModuleRef			m_roomMod;

			std::map<act::UID, std::map<std::string, proc::PortBaseRef>> m_subscriptions; // uid -> valueName -> port
			std::vector<proc::ProcNodeBaseRef>	m_jsonNodes; // keeping them on a warm place


			bool							checkEmpty(std::string var, std::string where, std::string what, ConnectionProviderRef sender = nullptr);

			std::vector<proc::ProcNodeBaseRef>	m_nodeTypeNodes; // holds all nodes to be registered, so if the request will be come in again, the nodes don't have to be generated again
			ci::Json						m_procNodeTypeData;

		}; using MiddlewareRef = std::shared_ptr<Middleware>;

	}
}