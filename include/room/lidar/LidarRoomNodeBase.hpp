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

#include "roompch.hpp"
#include "RoomNodeBase.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace room {

		class LidarRoomNodeBase : public RoomNodeBase
		{
		public:
			LidarRoomNodeBase(const std::string& portName, ci::Json description, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID);
			virtual ~LidarRoomNodeBase();

			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;
			virtual void cleanUp()	override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

			std::string getFixtureName() { return m_fixtureName; }

		protected:
			std::string				m_portName;
			std::string				m_fixtureName;
			float					m_maxDistance;

			virtual std::vector<ci::vec2>& getData() = 0;

		}; using LidarRoomNodeBaseRef = std::shared_ptr<LidarRoomNodeBase>;

	}
}