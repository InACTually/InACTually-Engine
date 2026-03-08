
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

			act::room::RoomNodeBaseRef	getRoomNodeByUID(act::UID uid) {
				for (auto&& mgr : list) {
					act::room::RoomNodeBaseRef roomNode = mgr->getNodeByUID(uid);
					if (roomNode)
						return roomNode;
				}
				return nullptr;
			}
		};
		using RoomManagersRef = std::shared_ptr<RoomManagers>;
	}
}