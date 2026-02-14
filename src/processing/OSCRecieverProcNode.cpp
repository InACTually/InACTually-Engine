
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "OSCRecieverProcNode.hpp"

act::proc::OSCRecieverProcNode::OSCRecieverProcNode() : ProcNodeBase("OSCReciever", NT_OUTPUT) {
	m_drawSize = ivec2(200, 200);

	//auto osc = OutputPort<ci::osc::Message>::create(PT_OSC, "osc");
	//m_outputPorts.push_back(osc);

	m_port = 123456;
	
	m_text = "";
	m_isRunning = false;

	
}

act::proc::OSCRecieverProcNode::~OSCRecieverProcNode() {
}

void act::proc::OSCRecieverProcNode::setup(act::room::RoomManagers roomMgrs) {
	initialize();
}

void act::proc::OSCRecieverProcNode::initialize()
{
	m_text = "Initialize";
	m_reciever = act::net::OSCReciever::create(m_port);

	auto ep = OutputPort<ci::osc::Message>::create(PT_OSC, "bno2");

	m_outputPorts.push_back(ep);

	addEndpoint("/bno1/");
	addEndpoint("/bno2/");
	addEndpoint("/streichelorgan/");
	addEndpoint("/squeak/");

	m_reciever->listen();

	m_text = "listening";
	m_isRunning = true;
}

void act::proc::OSCRecieverProcNode::update() {
	
}

void act::proc::OSCRecieverProcNode::draw() {
	beginNodeDraw();

	if (!m_reciever) {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Port").x);
		ImGui::InputInt("Port", &m_port, 1, 100);
		
		if (ImGui::Button("open")) {
			initialize();
		}
	}
	else {
		if (ImGui::Button("close")) {
			m_reciever.reset();
			m_isRunning = false;
			m_text = "";
		}
	}

	if (!m_text.empty()) {
		std::stringstream strstr;
		strstr << m_text << ":" << m_port;
		ImGui::TextUnformatted(strstr.str().c_str());
	}

	ImGui::InputText("", &m_endpointName);
	if (ImGui::Button("add Endpoint")) {
		addEndpoint(m_endpointName);
	}


	ImGui::NextColumn();

	for (int i = 0; i < m_outputPorts.size(); i++) {
		auto port = m_outputPorts[i];

		ImGui::PushID(i);


		if (ImGui::Button(ICON_FA_MINUS)) {
			removeEndpoint(i);
		}

		ImGui::SameLine();
		std::string caption = port->getCaption();
		if (ImGui::InputText("", &caption)) {
			port->setCaption(caption);
		}

		port->draw(0, true);

		ImGui::PopID();
	}

	ImGui::Columns();

	endNodeDraw();
}

void act::proc::OSCRecieverProcNode::addEndpoint(std::string name)
{
	auto ep = OutputPort<ci::osc::Message>::create(PT_OSC, name);

	m_reciever->setListener(name, [ep](ci::osc::Message msg) { 
		ep->send(msg);  
	});

	m_outputPorts.push_back(ep);
}

void act::proc::OSCRecieverProcNode::setEndpointLabel(int index, std::string label)
{
	if (index >= m_outputPorts.size())
		return;

	m_outputPorts[index]->setCaption(label);
}

void act::proc::OSCRecieverProcNode::removeEndpoint(int index)
{
	if (index >= m_outputPorts.size())
		return;

	m_reciever->removeListener(m_outputPorts[index]->getCaption());

	m_outputPorts.erase(m_outputPorts.begin() + index);
}

ci::Json act::proc::OSCRecieverProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["port"] = m_port;

	if(m_reciever)
		json["isRunning"] = m_isRunning;
	else
		json["isRunning"] = false;

	ci::Json outputports = ci::Json::array();
	for (auto&& p : m_outputPorts) {
		//outputports.push_back(p->getName());
	}
	json["endpoints"] = outputports;

	return json;
}

void act::proc::OSCRecieverProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "port", m_port);
	util::setValueFromJson(json, "isRunning", m_isRunning); 
	
	if (json.contains("endpoints")) {
		auto endpoints = json["endpoints"];
		for (auto&& json : endpoints) {
			// addEndpoint(endpoints);
			
		}
	}

	if (m_isRunning) {
		initialize();
	}
}