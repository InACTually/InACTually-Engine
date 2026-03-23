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

#include "llm/LLMToolManager.hpp"

void act::llm::LLMToolManager::add(const ToolDefinition& definition, ToolHandler handler) {
	m_tools.push_back({ definition, std::move(handler) });
}

void act::llm::LLMToolManager::add(const std::vector<ToolEntry>& entries) {
	for (auto& entry : entries)
		m_tools.push_back(entry);
}

void act::llm::LLMToolManager::clear() {
	m_tools.clear();
}

ci::Json act::llm::LLMToolManager::getToolsDescription() const {
	ci::Json tools = ci::Json::array();
	for (auto& entry : m_tools) {
		ci::Json props    = ci::Json::object();
		ci::Json required = ci::Json::array();
		for (auto& param : entry.definition.parameters) {
			ci::Json p        = ci::Json::object();
			p["type"]         = param.type;
			p["description"]  = param.description;
			props[param.name] = p;
			if (param.required)
				required.push_back(param.name);
		}
		ci::Json fn       = ci::Json::object();
		fn["name"]        = entry.definition.name;
		fn["description"] = entry.definition.description;
		fn["parameters"]  = {
			{ "type",       "object" },
			{ "properties", props    },
			{ "required",   required }
		};
		ci::Json tool     = ci::Json::object();
		tool["type"]      = "function";
		tool["function"]  = fn;
		tools.push_back(tool);
	}
	return tools;
}

std::vector<act::llm::LLMToolManager::DispatchResult>
act::llm::LLMToolManager::dispatch(const ci::Json& toolCallsJsonText) const {
	std::vector<DispatchResult> results;
	for (auto& toolCall : toolCallsJsonText) {
		DispatchResult dispatchResult;
		dispatchResult.id   = toolCall.value("id", "");
		dispatchResult.name = toolCall["function"].value("name", "");

		ci::Json args = ci::Json::object();
		try {
			auto& funcJson = toolCall["function"];
			if (funcJson.contains("arguments")) {
				auto& argsJson = funcJson["arguments"];
				if (argsJson.is_string())
					args = ci::Json::parse(argsJson.get<std::string>());
				else if (argsJson.is_object())
					args = argsJson;
			}
		}
		catch (...) {}

		ToolCall call{ dispatchResult.id, dispatchResult.name, args };

		dispatchResult.result = "[tool not found: " + dispatchResult.name + "]";
		for (auto& entry : m_tools) {
			if (entry.definition.name == dispatchResult.name) {
				try { dispatchResult.result = entry.handler(call); }
				catch (const std::exception& e) {
					dispatchResult.result = std::string("[tool error] ") + e.what();
				}
				break;
			}
		}
		results.push_back(std::move(dispatchResult));
	}
	return results;
}
