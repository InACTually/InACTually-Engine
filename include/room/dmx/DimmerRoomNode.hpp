
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeBase.hpp"
#include "dmx/DMXRoomNodeBase.hpp"

#include "dmx/DMXPro.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace room {

		class DimmerRoomNode : public RoomNodeBase, public DMXRoomNodeBase
		{
		public:
			DimmerRoomNode(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID = "");
			virtual ~DimmerRoomNode();

			static std::shared_ptr<DimmerRoomNode> create(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, ci::vec3 position = ci::vec3(0.0f, 0.0f, 0.0f), ci::vec3 rotation = ci::vec3(0.0f, 0.0f, 0.0f), float radius = 0.5f, act::UID replyUID = "") { return std::make_shared<DimmerRoomNode>(dmxInterface, description, name, startAddress, position, rotation, radius, replyUID); };

			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;
			virtual void cleanUp()	override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

		 
			void setDimmer(float dim);
 
		private:
 
		 
			util::MinMaxValue<float>	m_dimmer;
			 

		}; using DimmerRoomNodeRef = std::shared_ptr<DimmerRoomNode>;
		
	}
}