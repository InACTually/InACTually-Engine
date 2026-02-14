
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
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

#include "PortMsg.hpp"

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
				//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (act::Settings::get().fontSize*0.5f));
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

			std::vector<PortBaseRef> m_inputPorts;
			std::vector<PortBaseRef> m_outputPorts;


			InputPortRef<ci::Json>					createJsonInput			(std::string label, std::function<void(ci::Json)> cb, bool display = true)				{ auto port = InputPort<ci::Json>::create(PT_JSON, label, cb);						if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<bool>						createBoolInput			(std::string label, std::function<void(bool)> cb, bool display = true)					{ auto port = InputPort<bool>::create(PT_BOOL, label, cb);							if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<number>					createNumberInput		(std::string label, std::function<void(number)> cb, bool display = true)				{ auto port = InputPort<number>::create(PT_NUMBER, label, cb);						if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<std::vector<number>>		createNumberListInput	(std::string label, std::function<void(std::vector<number>)> cb, bool display = true)	{ auto port = InputPort<std::vector<number>>::create(PT_NUMBERLIST, label, cb);		if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<vec2>						createVec2Input			(std::string label, std::function<void(vec2)> cb, bool display = true)					{ auto port = InputPort<vec2>::create(PT_VEC2, label, cb);							if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<std::vector<vec2>>			createVec2ListInput		(std::string label, std::function<void(std::vector<vec2>)> cb, bool display = true)		{ auto port = InputPort<std::vector<vec2>>::create(PT_VEC2LIST, label, cb);			if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<vec3>						createVec3Input			(std::string label, std::function<void(vec3)> cb, bool display = true)					{ auto port = InputPort<vec3>::create(PT_VEC3, label, cb);							if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<std::vector<vec3>>			createVec3ListInput		(std::string label, std::function<void(std::vector<vec3>)> cb, bool display = true)		{ auto port = InputPort<std::vector<vec3>>::create(PT_VEC3LIST, label, cb);			if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<quat>						createQuatInput			(std::string label, std::function<void(quat)> cb, bool display = true)					{ auto port = InputPort<quat>::create(PT_QUAT, label, cb);							if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<ci::Color>					createColorInput		(std::string label, std::function<void(ci::Color)> cb, bool display = true)				{ auto port = InputPort<ci::Color>::create(PT_COLOR, label, cb);					if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<std::vector<ci::Color>>	createColorListInput	(std::string label, std::function<void(std::vector<ci::Color>)> cb, bool display = true){ auto port = InputPort<std::vector<ci::Color>>::create(PT_COLORLIST, label, cb);	if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<std::string>				createTextInput			(std::string label, std::function<void(std::string)> cb, bool display = true)			{ auto port = InputPort<std::string>::create(PT_TEXT, label, cb);					if (display) m_inputPorts.push_back(port); return port; }
			ImageInputPortRef						createImageInput		(std::string label, std::function<void(image)> cb, bool display = true)					{ auto port = ImageInputPort::create(PT_IMAGE, label, cb);						if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<ci::audio::BufferRef>		createAudioInput		(std::string label, std::function<void(ci::audio::BufferRef)> cb, bool display = true)	{ auto port = InputPort<ci::audio::BufferRef>::create(PT_AUDIO, label, cb);			if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<ci::audio::NodeRef>		createAudioNodeInput	(std::string label, std::function<void(ci::audio::NodeRef)> cb, bool display = true)	{ auto port = InputPort<ci::audio::NodeRef>::create(PT_AUDIONODE, label, cb);		if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<feature>					createFeatureInput		(std::string label, std::function<void(feature)> cb, bool display = true)				{ auto port = InputPort<feature>::create(PT_FEATURE, label, cb);					if (display) m_inputPorts.push_back(port); return port; }
			InputPortRef<featureList>				createFeatureListInput	(std::string label, std::function<void(featureList)> cb, bool display = true)			{ auto port = InputPort<featureList>::create(PT_FEATURELIST, label, cb);			if (display) m_inputPorts.push_back(port); return port; }


			OutputPortRef<ci::Json>					createJsonOutput		(std::string label, bool display = true) { auto port = OutputPort<ci::Json>::create(PT_JSON, label);					if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<bool>						createBoolOutput		(std::string label, bool display = true) { auto port = OutputPort<bool>::create(PT_BOOL, label);						if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<number>					createNumberOutput		(std::string label, bool display = true) { auto port = OutputPort<number>::create(PT_NUMBER, label);					if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<std::vector<number>>		createNumberListOutput	(std::string label, bool display = true) { auto port = OutputPort<std::vector<number>>::create(PT_NUMBERLIST, label);	if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<vec2>						createVec2Output		(std::string label, bool display = true) { auto port = OutputPort<vec2>::create(PT_VEC2, label);						if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<std::vector<vec2>>		createVec2ListOutput	(std::string label, bool display = true) { auto port = OutputPort<std::vector<vec2>>::create(PT_VEC2LIST, label);		if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<vec3>						createVec3Output		(std::string label, bool display = true) { auto port = OutputPort<vec3>::create(PT_VEC3, label);						if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<std::vector<vec3>>		createVec3ListOutput	(std::string label, bool display = true) { auto port = OutputPort<std::vector<vec3>>::create(PT_VEC3LIST, label);		if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<quat>						createQuatOutput		(std::string label, bool display = true) { auto port = OutputPort<quat>::create(PT_QUAT, label);						if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<ci::Color>				createColorOutput		(std::string label, bool display = true) { auto port = OutputPort<ci::Color>::create(PT_COLOR, label);					if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<std::vector<ci::Color>>	createColorListOutput	(std::string label, bool display = true) { auto port = OutputPort<std::vector<ci::Color>>::create(PT_COLORLIST, label);	if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<std::string>				createTextOutput		(std::string label, bool display = true) { auto port = OutputPort<std::string>::create(PT_TEXT, label);					if (display) m_outputPorts.push_back(port); return port; }
			ImageOutputPortRef						createImageOutput		(std::string label, bool display = true) { auto port = ImageOutputPort::create(PT_IMAGE, label);						if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<ci::audio::BufferRef>		createAudioOutput		(std::string label, bool display = true) { auto port = OutputPort<ci::audio::BufferRef>::create(PT_AUDIO, label);		if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<ci::audio::NodeRef>		createAudioNodeOutput	(std::string label, bool display = true) { auto port = OutputPort<ci::audio::NodeRef>::create(PT_AUDIONODE, label);		if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<feature>					createFeatureOutput		(std::string label, bool display = true) { auto port = OutputPort<feature>::create(PT_FEATURE, label);					if (display) m_outputPorts.push_back(port); return port; }
			OutputPortRef<featureList>				createFeatureListOutput	(std::string label, bool display = true) { auto port = OutputPort<featureList>::create(PT_FEATURELIST, label);			if (display) m_outputPorts.push_back(port); return port; }
			
		private:
			std::string	m_name;

		}; using ProcNodeBaseRef = std::shared_ptr<ProcNodeBase>;

	}
}