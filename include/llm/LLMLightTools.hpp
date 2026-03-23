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

#include "llm/LLMToolManager.hpp"
#include "RoomManagers.hpp"

namespace act {
	namespace llm {


		class LLMLightTools {
		public:
			static std::vector<ToolEntry> getToolDefinitions(act::room::RoomManagers roomMgrs);
		};

	}
}
