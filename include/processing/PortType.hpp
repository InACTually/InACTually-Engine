
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/


#pragma once

#include <vector>

namespace act {
	namespace proc {

		using number = float;
		using numberList = std::vector<number>;

		using feature = std::pair<std::string, float>;
		using featureList = std::vector<feature>;

#ifdef CV_VERSION
		using image = cv::UMat;
#else
		class image {};
#endif

		//## Note that the types are mapped by portTypeToString in the Stage!
		enum PortType {
			PT_UNKNOWN = -1,
			PT_GENERIC = 0,
			PT_JSON,
			PT_OSC,

			PT_BOOL,
			PT_NUMBER,
			PT_NUMBERLIST,	// std::vector<float>
			PT_VEC2,
			PT_VEC2LIST,	// std::vector<vec2>
			PT_VEC3,
			PT_VEC3LIST,	// std::vector<vec3>
			PT_QUAT,
			PT_COLOR,
			PT_COLORLIST,	// std::vector<ci::Color>
			PT_TEXT,

			PT_IMAGE,
			PT_AUDIO,		// ci::audio::BufferRef
			PT_AUDIONODE,	// ci::audio::NodeRef
			PT_POINTCLOUD,
			
			PT_FEATURE,		// std::pair<string,float>
			PT_FEATURELIST, // std::vector<feature>
			PT_BODY,		// proc::BodyRef
			PT_BODYLIST,	// std::vector<proc::BodyRef>
			
			PT_METAMODEL
		};

		static std::string portTypeToString(PortType type) {
			switch (type) {
			case PT_GENERIC:		return "generic";
			case PT_JSON:			return "json";
			case PT_OSC:			return "osc";

			case PT_BOOL:			return "bool";
			case PT_NUMBER:			return "number";
			case PT_NUMBERLIST:		return "numberList";
			case PT_VEC2:			return "vec2";
			case PT_VEC2LIST:		return "vec2List";
			case PT_VEC3:			return "vec3";
			case PT_VEC3LIST:		return "vec3List";
			case PT_QUAT:			return "quat";
			case PT_COLOR:			return "color";
			case PT_COLORLIST:		return "colorList";
			case PT_TEXT:			return "text";

			case PT_IMAGE:			return "image";
			case PT_AUDIO:			return "audio";
			case PT_AUDIONODE:		return "audioNode";
			case PT_POINTCLOUD:		return "pointcloud";

			case PT_FEATURE:		return "feature";
			case PT_FEATURELIST:	return "featureList";
			case PT_BODY:			return "body";
			case PT_BODYLIST:		return "bodyList";

			case PT_METAMODEL:		return "metamodel";

			default:				return "unknown";
			}
		}

	}
}