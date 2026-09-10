/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"

#include "position/PositionManager.hpp"
#include "camera/CameraManager.hpp"
#include "dmx/DMXManager.hpp"
#include "kinect/KinectManager.hpp"
#include "body/BodyTrackingManager.hpp"
#include "marker/MarkerManager.hpp"
#include "object/ObjectManager.hpp"
#include "audio/AudioManager.hpp"
#include "display/DisplayManager.hpp"
#include "computer/ComputerManager.hpp"
#include "actionspace/ActionspaceManager.hpp"
#include "projector/ProjectorManager.hpp"
#include "lidar/LidarManager.hpp"


namespace act {
	namespace room {

		class RoomManagers {
		public:
			act::room::PositionManagerRef		positionMgr;
			act::room::CameraManagerRef			cameraMgr;
			act::room::DMXManagerRef			dmxMgr;
			act::room::KinectManagerRef			kinectMgr;
			act::room::BodyTrackingManagerRef	bodyTrackingMgr;
			act::room::MarkerManagerRef			markerMgr;
			act::room::ObjectManagerRef			objectMgr;
			act::room::AudioManagerRef			audioMgr;
			act::room::DisplayManagerRef		displayMgr;
			act::room::ComputerManagerRef		computerMgr;
			act::room::ActionspaceManagerRef	actionspaceMgr;
			act::room::ProjectorManagerRef		projectorMgr;
			act::room::LidarManagerRef			lidarMgr;

			std::vector<act::room::RoomNodeManagerBaseRef> list;

			static std::shared_ptr<RoomManagers> get() {
				if (m_roomMgrs == nullptr) {
					m_roomMgrs = std::shared_ptr<RoomManagers>(new RoomManagers());
					m_roomMgrs->init();
				}
				return m_roomMgrs;
			}

			act::room::RoomNodeBaseRef	getRoomNodeByUID(act::UID uid) {
				for (auto&& mgr : list) {
					act::room::RoomNodeBaseRef roomNode = mgr->getNodeByUID(uid);
					if (roomNode)
						return roomNode;
				}
				return nullptr;
			}

			private:
				static std::shared_ptr<RoomManagers> m_roomMgrs;

				RoomManagers() {}

				void init() {
					m_roomMgrs->positionMgr = act::room::PositionManager::create();
					m_roomMgrs->cameraMgr = act::room::CameraManager::create();
					m_roomMgrs->dmxMgr = act::room::DMXManager::create();
					m_roomMgrs->kinectMgr = act::room::KinectManager::create();
					m_roomMgrs->markerMgr = act::room::MarkerManager::create(m_roomMgrs->cameraMgr);
					m_roomMgrs->objectMgr = act::room::ObjectManager::create(m_roomMgrs->cameraMgr);
					m_roomMgrs->audioMgr = act::room::AudioManager::create();
					m_roomMgrs->displayMgr = act::room::DisplayManager::create();
					m_roomMgrs->computerMgr = act::room::ComputerManager::create();
					m_roomMgrs->actionspaceMgr = act::room::ActionspaceManager::create();
					m_roomMgrs->projectorMgr = act::room::ProjectorManager::create();
					m_roomMgrs->lidarMgr = act::room::LidarManager::create();
							  
					m_roomMgrs->bodyTrackingMgr = act::room::BodyTrackingManager::create(m_roomMgrs->kinectMgr);
							  
					m_roomMgrs->list.push_back(m_roomMgrs->actionspaceMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->positionMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->cameraMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->projectorMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->dmxMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->kinectMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->bodyTrackingMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->markerMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->objectMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->audioMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->displayMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->computerMgr);
					m_roomMgrs->list.push_back(m_roomMgrs->lidarMgr);

					for (auto&& mgr : m_roomMgrs->list) {
						mgr->setup();
					}
				};
		};
		using RoomManagersRef = std::shared_ptr<RoomManagers>;
	}
}