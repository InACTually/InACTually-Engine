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

#include "RoomNodeBase.hpp"
#include "lidar/LidarRoomNodeBase.hpp"
#include "ldlidar/include/ldlidar_driver.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace room {

		class LDLidarRoomNode : public LidarRoomNodeBase
		{
		public:
			LDLidarRoomNode(const std::string& portName, ci::Json description, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID = "");
			virtual ~LDLidarRoomNode();

			static std::shared_ptr<LDLidarRoomNode> create(const std::string& portName, ci::Json description, std::string name, ci::vec3 position = ci::vec3(0.0f), ci::vec3 rotation = ci::vec3(0.0f), float radius = 0.5f, act::UID replyUID = "") {
				return std::make_shared<LDLidarRoomNode>(portName, description, name, position, rotation, radius, replyUID);
			}

			virtual void setup()	override;
			virtual void update()	override;
			virtual void cleanUp()	override;

			virtual std::vector<ci::vec2>& getData() override;

		private:
			ldlidar::LDLidarDriver	m_driver;
			std::vector<ci::vec2>	m_lidarData;

		}; using LDLidarRoomNodeRef = std::shared_ptr<LDLidarRoomNode>;

	}
}