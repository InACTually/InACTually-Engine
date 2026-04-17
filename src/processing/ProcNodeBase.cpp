
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "ProcNodeBase.hpp"

unsigned int act::proc::IDBase::nextID = 0;


void act::proc::ProcNodeBase::beginNodeDraw(ProcNodeDrawState state)
{
	unsigned int color;

	if (state == ProcNodeDrawState::NDS_ACTIVE)	{
		switch (m_nodeType) {
		case NT_INPUT:
			color = m_inputNodeColor;
			break;
		case NT_OUTPUT:
			color = m_outputNodeColor;
			break;
		case NT_CONTAINER:
			color = m_containerNodeColor;
			break;
		default:
			color = m_darkprocessingNodeColor;
		}
	}
	else {
		color = m_darkErrorNodeColor;
	}

	ImNodes::PushColorStyle(ImNodesCol_TitleBar, color);
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, color);
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, color);

	ImNodes::BeginNode(m_id);

	ImNodes::BeginNodeTitleBar();
	if(isEnabled())
		ImGui::TextUnformatted(getTitle().c_str());
	else
		ImGui::TextUnformatted((getTitle() + " [disabled]").c_str());
	ImNodes::EndNodeTitleBar();

	ImGui::PushItemWidth(m_drawSize.x);
}

void act::proc::ProcNodeBase::endNodeDraw(bool doDrawInputPorts, bool doDrawOutputPorts) // !LinkerNode & Container"Group"Node is not using this
{
	ImGui::PopItemWidth();

	drawPorts(m_drawSize.x, doDrawInputPorts, doDrawOutputPorts);

	vec2 newPos = ImNodes::GetNodeGridSpacePos(m_id);
	if (newPos != m_position && m_isInitialized) {
		m_position = newPos;
		setPosition(m_position);
	}
	
	if (!m_isInitialized) {
		m_isInitialized = true;
	}

	ImNodes::EndNode();

	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}

ci::Json act::proc::ProcNodeBase::getJsonTypeDefinition()
{
	ci::Json typeDefinition = ci::Json::object();
	ci::Json inputs			= ci::Json::array();
	ci::Json outputs		= ci::Json::array();
	ci::Json rpcs			= ci::Json::array();
	ci::Json params			= toParams();

	for (auto&& port : m_inputPorts) {
		ci::Json input	= ci::Json::object();
		input["name"]	= port->getName();
		input["type"]	= portTypeToString(port->getType());
		inputs.push_back(input);
	}

	for (auto&& port : m_outputPorts) {
		ci::Json output	= ci::Json::object();
		output["name"]	= port->getName();
		output["type"]	= portTypeToString(port->getType());
		outputs.push_back(output);
	}

	for (auto&& rpc : getRPCMap()) {
		ci::Json rpcJson	= ci::Json::object();
		rpcJson["name"]		= rpc.first;
		rpcs.push_back(rpcJson);
	}

	typeDefinition[m_name]["inputs"]	= inputs;
	typeDefinition[m_name]["outputs"]	= outputs;
	typeDefinition[m_name]["rpcs"]		= rpcs;
	typeDefinition[m_name]["params"]	= params;

	return typeDefinition;
}

bool act::proc::ProcNodeBase::removePort(PortBaseRef port, std::vector<PortBaseRef>& ports) {
	for (auto it = ports.begin(); it != ports.end(); it++) {
		if (port->getUID() == (*it)->getUID()) {
			(*it)->disconnectAll();
			ports.erase(it);
			return true;
		}
	}
	return false;
}