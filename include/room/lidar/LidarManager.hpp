
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

#include "RoomNodeManagerBase.hpp"
#include "lidar/LidarRoomNodeBase.hpp"
#include "lidar/LDLidarRoomNode.hpp"

namespace act {
	namespace room {

		class LidarManager : public RoomNodeManagerBase {
		public:
			LidarManager();
			~LidarManager();

			static	std::shared_ptr<LidarManager> create() { return std::make_shared<LidarManager>(); };

			void	setup() override;
			void	cleanUp() override;

			void	update() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			act::room::RoomNodeBaseRef getDeviceByMarkerID(int id);
			act::room::LDLidarRoomNodeRef getLDLidarByIndex(int index);

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);
			void saveDevicesToJson();
			act::room::RoomNodeBaseRef addDevice(std::string name);

			float getMovement() { return m_movement; }
			int getNumOfBlobs() { return m_blobAmount; }
 
		private:

			void loadFixtures();
			void saveFixtures();
			std::map<std::string, ci::Json>		m_fixtureDescriptions;
			std::vector<std::string>			m_fixtureNames;

			void refreshLists() override;
			std::vector<std::string>			m_availableDeviceNames;
			int									m_selectedDevice;

			std::vector<ci::vec2>				m_latestData;
			std::vector<ci::vec2>				m_background;
			float								m_movement;
			int									m_blobAmount;

			void calculate();
			
		};
		using LidarManagerRef = std::shared_ptr<LidarManager>;
	}
}