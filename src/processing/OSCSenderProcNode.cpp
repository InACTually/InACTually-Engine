
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "OSCSenderProcNode.hpp"

act::proc::OSCSenderProcNode::OSCSenderProcNode() : ProcNodeBase("OSCSender", NT_OUTPUT) {
	m_drawSize = glm::ivec2(200, 200);

	auto osc = InputPort<ci::osc::Message>::create(PT_OSC, "osc", [&](ci::osc::Message osc) { this->onOSC(osc); });
	m_inputPorts.push_back(osc);

	m_port = 10001;
	m_addressString = "broadcast";
	
	m_text = "";
	m_isRunning = false;
}

act::proc::OSCSenderProcNode::~OSCSenderProcNode() {
}

void act::proc::OSCSenderProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	
}

void act::proc::OSCSenderProcNode::initialize()
{
	m_text = "Initialize";

	if (m_addressString.empty() || m_addressString == "broadcast") {
		m_addressString = "broadcast";
		m_address = asio::ip::address_v4::broadcast();
	}
	else {
		try {
			m_address = asio::ip::make_address(m_addressString);
		}
		catch (...) {
			m_text = "Invalid address";
		}
	}

	if(m_text != "Invalid address")
		m_server = act::net::OSCServer::create(m_port, m_address);
}

void act::proc::OSCSenderProcNode::update() {
	if (!m_server)
		return;

	if (m_server->isRunning())
		m_text = "Opened";
	else
		m_text = "Can't open";
}

void act::proc::OSCSenderProcNode::draw() {
	beginNodeDraw();

	if (!m_server) {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("adress").x);
		ImGui::InputText("adress", &m_addressString);

		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Port").x);
		ImGui::InputInt("Port", &m_port, 1, 100);
		
		if (ImGui::Button("open")) {
			initialize();
		}
	}
	else {
		if (ImGui::Button("close")) {
			m_server.reset();
			m_text = "";
		}
	}

	if (!m_text.empty()) {
		std::stringstream strstr;
		strstr << m_text << ": " << m_addressString << ":" << m_port;
		ImGui::TextUnformatted(strstr.str().c_str());
	}

	endNodeDraw();
}

void act::proc::OSCSenderProcNode::onOSC(ci::osc::Message msg) {
	if(m_server)
		m_server->sendMsg(msg);
}

ci::Json act::proc::OSCSenderProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["port"] = m_port;

	if(m_server)
		json["isRunning"] = m_server->isRunning();
	else
		json["isRunning"] = false;

	return json;
}

void act::proc::OSCSenderProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "port", m_port);
	util::setValueFromJson(json, "isRunning", m_isRunning); 

	if (m_isRunning) {
		initialize();
	}
}