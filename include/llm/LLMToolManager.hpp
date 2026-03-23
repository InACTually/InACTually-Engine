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

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "cinder/Json.h"
#include "cinder/Log.h"

namespace act {
	namespace llm {

		struct ToolParameter {
			std::string name;
			std::string type;         // "string" | "number" | "boolean"
			std::string description;
			bool        required = true;
		};

		struct ToolDefinition {
			std::string                name;
			std::string                description;
			std::vector<ToolParameter> parameters;
		};

		struct ToolCall {
			std::string id;
			std::string name;
			ci::Json    arguments;
		};

		using ToolHandler = std::function<std::string(const ToolCall&)>;

		struct ToolEntry {
			ToolDefinition definition;
			ToolHandler    handler;
		};

		class LLMToolManager {
		public:
			LLMToolManager()          = default;
			virtual ~LLMToolManager() = default;

			static std::shared_ptr<LLMToolManager> create() {
				return std::make_shared<LLMToolManager>();
			}

			void add(const ToolDefinition& definition, ToolHandler handler);
			void add(const std::vector<ToolEntry>& entries);

			void clear();

			std::size_t size()  const { return m_tools.size(); }
			bool        isEmpty() const { return m_tools.empty(); }

			ci::Json getToolsDescription() const;

			// result message to be appended to the chat.
			struct DispatchResult {
				std::string id;
				std::string name;
				std::string result;
			};
			std::vector<DispatchResult> dispatch(const ci::Json& toolCallsJsonText) const;

		private:
			
			std::vector<ToolEntry> m_tools;
		};

		using LLMToolManagerRef = std::shared_ptr<LLMToolManager>;

	}
}
