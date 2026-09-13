
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "IDBase.hpp"
#include "UniqueIDBase.hpp"

#include "Port.hpp"

#include <functional>

#ifndef incl_RN
#include "camera/CameraRoomNode.hpp"
#include "body/BodyRoomNode.hpp"
#endif // !1

#include "Osc.h"



#define USINGPORTREF(name, datatype)	using name##InputPort = InputPort<datatype>; \
										using name##InputPortRef = std::shared_ptr<name##InputPort>; \
										using name##OutputPort = OutputPort<datatype>; \
										using name##OutputPortRef = std::shared_ptr<name##OutputPort>; \
										static name##InputPortRef to##name##InputPort(act::proc::PortBaseRef port) { \
											return std::dynamic_pointer_cast<name##InputPort>(port); } \
										static name##OutputPortRef to##name##OutputPort(act::proc::PortBaseRef port) {	\
											return std::dynamic_pointer_cast<name##OutputPort>(port); }

 
namespace act {
	namespace proc {
		USINGPORTREF(Json,			ci::Json);
		USINGPORTREF(Osc,			ci::osc::Message);
		USINGPORTREF(Bool,			bool);
		USINGPORTREF(Number,		number);
		USINGPORTREF(NumberList,	numberList);
		USINGPORTREF(Vec2,			glm::vec2);
		USINGPORTREF(Vec2List,		vec2List);
		USINGPORTREF(Vec3,			glm::vec3);
		USINGPORTREF(Vec3List,		vec3List);
		USINGPORTREF(Quat,			glm::quat);
		USINGPORTREF(Color,			ci::Color);
		USINGPORTREF(ColorList,		colorList);
		USINGPORTREF(Text,			std::string);
		USINGPORTREF(Image,			proc::image);
		USINGPORTREF(Audio,			ci::audio::BufferRef);
		USINGPORTREF(AudioNode,		ci::audio::NodeRef);
		//USINGPORTREF(Pointcloud,	/**/);
		USINGPORTREF(Feature,		feature);
		USINGPORTREF(FeatureList,	featureList);
		USINGPORTREF(Body,			act::room::BodyRef);
		USINGPORTREF(BodyList,		std::vector<act::room::BodyRef>);

	}
}