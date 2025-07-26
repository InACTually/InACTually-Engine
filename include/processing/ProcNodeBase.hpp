
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomManagers.hpp"
#include "procpch.hpp"



using namespace ci;
using namespace ci::app;

#define PROCNODECREATE(node) CREATE(node, ProcNodeBase);

namespace act {
	namespace proc {

		enum ProcNodeDrawState {
			NDS_ERROR = -1,
			NDS_UNKNOWN = 0,
			NDS_ACTIVE,
			NDS_INACTIVE
		};

		enum ProcNodeType {
			NT_INPUT,
			NT_OUTPUT,
			NT_PROCESSOR,
			NT_CONTAINER
		};

		class ProcNodeBase : public UniqueIDBase, public IDBase, public net::RPCHandler
		{
		public:
			ProcNodeBase(std::string name, ProcNodeType type = ProcNodeType::NT_PROCESSOR) : m_name(name), m_title(name), m_nodeType(type), m_position(vec2(0,0)) {
				m_visualOutputPort = OutputPort<cv::UMat>::create(PT_IMAGE, "visual");
				m_valueOutputPort = OutputPort<ci::Json>::create(PT_JSON, "values");

				setPosition(vec2(0, 0));
			};
			virtual ~ProcNodeBase() {
				m_outputPorts.clear();
				m_inputPorts.clear();
			};

			virtual void setup(act::room::RoomManagers) {};
			virtual void update() = 0;
			virtual void draw() = 0;

			virtual void beginNodeDraw(ProcNodeDrawState state = ProcNodeDrawState::NDS_ACTIVE);
			virtual void endNodeDraw(bool doDrawInputPorts = true, bool doDrawOutputPorts = true);

			inline std::string getName() const { return m_name; };
			inline std::string getTitle() const { return m_title; };
			void setTitle(std::string title) { m_title = title; };

			vec2 getPosition() {
				return ImVec2(m_position.x, m_position.y);
			};

			void setPosition(vec2 pos) {
				m_position = pos;
				ImNodes::SetNodeGridSpacePos(m_id, ImVec2(pos.x, pos.y));
			}

			bool isEnabled() {
				return m_isEnabled;
			}

			virtual void enable() {
				for (auto port : m_inputPorts)
					port->enable();

				m_isEnabled = true;
			}

			virtual void disable() {
				for (auto port : m_inputPorts)
					port->disable();

				m_isEnabled = false;
			}

			virtual void drawPorts(int width) {
				drawPorts(width, true, true);
			}
			virtual void drawPorts(int width, bool doDrawInputPorts = true, bool doDrawOutputPorts = true) {
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (act::Settings::get().fontSize*0.5f));
				if(doDrawInputPorts)
					for (auto&& port : m_inputPorts) {
						port->draw(width);
					}
				if (doDrawOutputPorts)
					for (auto&& port : m_outputPorts) {
						port->draw(width);
					}
			}

			PortBaseRef getOutputPortByUid(act::UID uid) {
				for (auto&& port : m_outputPorts) {
					if (port->getUID() == uid)
						return port;
				}
				return nullptr;
			}

			PortBaseRef getOutputPortByRuntimeID(int id) {
				for (auto&& port : m_outputPorts) {
					if (port->getRuntimeID() == id)
						return port;
				}
				return nullptr;
			}


			virtual PortBaseRef getOutputPortByName(std::string name)  {
				for (auto&& port : m_outputPorts) {
					if (port->getName() == name)
						return port;
				}
				return nullptr;
			}
			PortBaseRef getOutputPortByType(proc::PortType type) { // return first of type
				for (auto&& port : m_outputPorts) {
					if (port->getType() == type)
						return port;
				}
				return nullptr;
			}

			PortBaseRef getInputPortByUid(act::UID uid) {
				for (auto&& port : m_inputPorts) {
					if (port->getUID() == uid)
						return port;
				}
				return nullptr;
			}
			PortBaseRef getInputPortByRuntimeID(int id) {
				for (auto&& port : m_inputPorts) {
					if (port->getRuntimeID() == id)
						return port;
				}
				return nullptr;
			}
			virtual PortBaseRef getInputPortByName(std::string name) {
				for (auto&& port : m_inputPorts) {
					if (port->getName() == name)
						return port;
				}
				return nullptr;
			}
			PortBaseRef getInputPortByType(proc::PortType type) { // return first of type
				for (auto&& port : m_inputPorts) {
					if (port->getType() == type)
						return port;
				}
				return nullptr;
			}
			PortBaseRef getPortByRuntimeID(int id)
			{
				PortBaseRef port = getInputPortByRuntimeID(id);
				if (port == nullptr)
				{
					port = getOutputPortByRuntimeID(id);
					return port;
				}
				return port;
			}
			
			std::vector<int> getRuntimeIDs() {
				std::vector<int> rid;
				for (auto out : m_outputPorts)
				{
					rid.push_back(out->getRuntimeID());
				}
				for (auto in : m_inputPorts)
				{
					rid.push_back(in->getRuntimeID());
				}
				return rid;
			}

			std::vector<PortBaseRef> getAllInputPorts() { return m_inputPorts; };
			std::vector<PortBaseRef> getAllOutputPorts() { return m_inputPorts; };

			virtual void setIsHovered(bool isHovered) { m_isHovered = isHovered; }
			virtual void setIsSelected(bool isSelected) { m_isSelected = isSelected; }

			virtual ci::Json toParams() { return ci::Json::object(); };
			virtual void fromParams(ci::Json json) {};
			ci::Json getJsonTypeDefinition();

			void preventDrag(bool prevent) {
				ImNodes::SetNodeDraggable(m_id, !prevent);
			}

		protected:
			std::string	m_title;
			
			ProcNodeType	m_nodeType;
			vec2		m_position		= vec2(150, 20);
			bool		m_isInitialized	= false;
			bool		m_isEnabled		= true;
			bool		m_isHovered		= false;
			bool		m_isSelected	= false;
			ci::ivec2	m_drawSize		= ci::ivec2(250, 250);

			unsigned int m_errorNodeColor			= IM_COL32(util::Design::errorColor().r * 255, util::Design::errorColor().g * 255, util::Design::errorColor().b * 255, 255);
			unsigned int m_darkErrorNodeColor		= IM_COL32(util::Design::darkErrorColor().r * 255, util::Design::darkErrorColor().g * 255, util::Design::darkErrorColor().b * 255, 255);
			unsigned int m_darkprocessingNodeColor	= IM_COL32(util::Design::darkPrimaryColor().r * 255, util::Design::darkPrimaryColor().g * 255, util::Design::darkPrimaryColor().b * 255, 255);
			unsigned int m_processingNodeColor		= IM_COL32(util::Design::primaryColor().r * 255, util::Design::primaryColor().g * 255, util::Design::primaryColor().b * 255, 255);
			unsigned int m_inputNodeColor			= IM_COL32(util::Design::primaryColor().g * 255, util::Design::primaryColor().r * 255, util::Design::primaryColor().b * 255, 255);
			unsigned int m_outputNodeColor			= IM_COL32(util::Design::primaryColor().r * 255, util::Design::primaryColor().b * 255, util::Design::primaryColor().g * 255, 255);
			unsigned int m_containerNodeColor		= IM_COL32(util::Design::primaryColor().g * 255, util::Design::primaryColor().b * 255, util::Design::primaryColor().r * 255, 255);

			OutputPortRef<cv::UMat>	 m_visualOutputPort;
			OutputPortRef<ci::Json>  m_valueOutputPort;

			std::vector<PortBaseRef> m_inputPorts;
			std::vector<PortBaseRef> m_outputPorts;

		private:
			std::string	m_name;

		}; using ProcNodeBaseRef = std::shared_ptr<ProcNodeBase>;

	}
}