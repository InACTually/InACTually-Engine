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

#include "llm/LLMLightTools.hpp"

#include <algorithm>
std::vector<act::llm::ToolEntry> act::llm::LLMLightTools::getToolDefinitions(act::room::RoomManagers roomMgrs) {

	std::vector<act::llm::ToolEntry> definitions;
	definitions.push_back({
		ToolDefinition{
			"list_lights",
			"Returns a JSON array of all available DMX fixtures (dimmers and moving heads) with their index, name and full current state.",
			{}
		},
		[roomMgrs](const ToolCall&) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			auto names = roomMgrs.dmxMgr->getFixtureNames();
			if (names.empty()) return "[]";
			ci::Json arr = ci::Json::array();
			for (int i = 0; i < (int)names.size(); i++) {
				ci::Json e = ci::Json::object();
				e["index"] = i;
				//e["name"]  = names[i];

				auto dimmer = roomMgrs.dmxMgr->getDimmerByIndex(i);
				if (dimmer) {
					e["type"]  = "dimmer";
					e["state"] = dimmer->toJson();
				} else {
					auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(i);
					if (mh) {
						e["type"]  = "light";
						e["color"] = mh->toJson()["params"]["color"];
					}
				}
				arr.push_back(e);
			}
			return arr.dump();
		} 
	});

	definitions.push_back({
		ToolDefinition{
			"get_light",
			"Returns the full current state of a single light by its zero-based index.",
			{
				{ "index", "number", "Zero-based index of the fixture.", true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int index = call.arguments.value("index", 0);

			ci::Json e = ci::Json::object();
			e["index"] = index;

			auto dimmer = roomMgrs.dmxMgr->getDimmerByIndex(index);
			if (dimmer) {
				e["type"] = "dimmer";
				e["name"] = dimmer->getName();
				e["state"] = dimmer->toJson();
				return e.dump();
			}

			auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(index);
			if (mh) {
				e["type"] = "movingHead";
				e["name"] = mh->getName();
				e["state"] = mh->toJson();
				return e.dump();
			}

			return "{\"error\":\"No fixture found at index " + std::to_string(index) + "\"}";
		}
	});
	/*
	definitions.push_back({
		ToolDefinition{
			"set_dimmer",
			"Sets the brightness (dimmer) of a dimmer fixture. Dimmer is 0.0 (off) to 1.0 (full).",
			{
				{ "index",      "number", "Zero-based index of the dimmer fixture.",             true },
				{ "dimmer", "number", "Brightness value between 0.0 (off) and 1.0 (full).", true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int   index = call.arguments.value("index",      0);
			float dimmer = std::clamp(static_cast<float>(call.arguments.value("dimmer", 1.0)), 0.0f, 1.0f);

			auto dim = roomMgrs.dmxMgr->getDimmerByIndex(index);
			if (!dim)
				return "{\"error\":\"Dimmer not found at index " + std::to_string(index) + "\"}";
			dim->setDimmer(dimmer);
			return "{\"ok\":true,\"id\":\"" + call.id + "\",\"dimmer\":" + std::to_string(dimmer) + "}";
		}
	});

	*/
	definitions.push_back({
		ToolDefinition{
			"set_light_dimmer",
			"Sets the brightness (dimmer) of a moving head fixture. Dimmer is 0.0 (off) to 1.0 (full).",
			{
				{ "index",      "number", "Zero-based index of the moving head fixture.",        true },
				{ "dimmer", "number", "Dimmer value between 0.0 (off) and 1.0 (full).", true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int   index      = call.arguments.value("index",      0);
			float dimmer = std::clamp(static_cast<float>(call.arguments.value("dimmer", 1.0)), 0.0f, 1.0f);

			auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(index);
			if (!mh)
				return "{\"error\":\"Moving head not found at index " + std::to_string(index) + "\"}";
			mh->setDimmer(dimmer);
			return "{\"ok\":true,\"id\":\"" + call.id + "\",\"dimmer\":" + std::to_string(dimmer) + "}";
		}
	});
	

	definitions.push_back({
		ToolDefinition{
			"set_light_color",
			"Sets the RGB color of a single light identified by the index. Each channel is 0.0 to 1.0 as number.",
			{
				{ "index", "number", "Zero-based index of ONE light.", true },
				{ "r",     "number", "Red channel 0.0-1.0.",                         true },
				{ "g",     "number", "Green channel 0.0-1.0.",                       true },
				{ "b",     "number", "Blue channel 0.0-1.0.",                        true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int   index = call.arguments.value("index", 0);
			float r     = std::clamp(static_cast<float>(call.arguments.value("r", 1.0)), 0.0f, 1.0f);
			float g     = std::clamp(static_cast<float>(call.arguments.value("g", 1.0)), 0.0f, 1.0f);
			float b     = std::clamp(static_cast<float>(call.arguments.value("b", 1.0)), 0.0f, 1.0f);

			auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(index);
			if (!mh)
				return "{\"error\":\"Moving head not found at index " + std::to_string(index) + "\"}";
			mh->setColor(ci::Color(r, g, b));
			return "{\"ok\":true,\"id\":\"" + call.id + "\"}";
		}
	});

	definitions.push_back({
		ToolDefinition{
			"set_all_light_color",
			"Sets the RGB color of ALL lights. Each channel is 0.0 to 1.0 as number.",
			{
				{ "r",     "number", "Red channel 0.0-1.0.",                         true },
				{ "g",     "number", "Green channel 0.0-1.0.",                       true },
				{ "b",     "number", "Blue channel 0.0-1.0.",                        true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int   index = call.arguments.value("index", 0);
			float r = std::clamp(static_cast<float>(call.arguments.value("r", 1.0)), 0.0f, 1.0f);
			float g = std::clamp(static_cast<float>(call.arguments.value("g", 1.0)), 0.0f, 1.0f);
			float b = std::clamp(static_cast<float>(call.arguments.value("b", 1.0)), 0.0f, 1.0f);

			auto nodes = roomMgrs.dmxMgr->getNodes();
			for (int i = 0; i < (int)nodes.size(); i++) {
				auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(i);
				if (!mh)
					continue;
				mh->setColor(ci::Color(r, g, b));
			}
			return "{\"ok\":true,\"id\":\"" + call.id + "\"}";
		}
		});


	definitions.push_back({
		ToolDefinition{
			"set_light_lookat",
			"Points a light at a world-space position (x, y, z) in metres. Use the position data from the room to point at objects.",
			{
				{ "index", "number", "Zero-based index of the moving head fixture.", true },
				{ "x",     "number", "Target X coordinate in metres.",               true },
				{ "y",     "number", "Target Y coordinate in metres.",               true },
				{ "z",     "number", "Target Z coordinate in metres.",               true }
			}
		},
		[roomMgrs](const ToolCall& call) -> std::string {
			if (!roomMgrs.dmxMgr) return "{\"error\":\"DMX manager not available\"}";
			int   index = call.arguments.value("index", 0);
			float x = static_cast<float>(call.arguments.value("x", 0.0));
			float y = static_cast<float>(call.arguments.value("y", 0.0));
			float z = static_cast<float>(call.arguments.value("z", 0.0));

			auto mh = roomMgrs.dmxMgr->getMovingHeadByIndex(index);
			if (!mh)
				return "{\"error\":\"Moving head not found at index " + std::to_string(index) + "\"}";
			mh->lookAt(ci::vec3(x, y, z));
			return "{\"ok\":true,\"id\":\"" + call.id + "\",\"lookAt\":{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y) + ",\"z\":" + std::to_string(z) + "}}";
		}
	});

	return definitions;
}
