
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "dmx/DMXManager.hpp"

#include "dmx/MovingHeadRoomNode.hpp"
#include "dmx/DimmerRoomNode.hpp"


act::room::DMXManager::DMXManager()
	: RoomNodeManagerBase("dmxManager")
{
	m_selectedInterface = 0;
	m_selectedFixture = 0;
	m_currentAddress = 1;

	m_fixtureNames = std::vector<std::string>(0);
	m_availableDeviceNames = std::vector<std::string>(0);

	refreshInterfaceNames();
	loadFixtures();
	refreshLists();

	changeInterface("COM3");
}

act::room::DMXManager::~DMXManager()
{	
}

void act::room::DMXManager::setup()
{
}

void act::room::DMXManager::cleanUp()
{
	for (auto&& node : m_nodes) {
		node->cleanUp();
	}
}

act::room::RoomNodeBaseRef act::room::DMXManager::drawMenu()
{
	if (ImGui::Button("refresh Interfacelist")) {
		refreshInterfaceNames();
	}
	if (m_interfaceNames.empty()) {
		ImGui::Text("No DMX-Interface has been found.");
	}
	else {
		if (ImGui::Combo("use	 Interface", &m_selectedInterface, m_interfaceNames)) {
			changeInterface(m_interfaceNames[m_selectedInterface]);
		}
	}

	//ImGui::SetNextItemWidth(m_displaySize.x - ImGui::CalcTextSize("Device").x);
	ImGui::Combo("Device", &m_selectedFixture, m_fixtureNames);
	if (ImGui::InputInt("address", &m_currentAddress)) {
		m_currentAddress = std::clamp(m_currentAddress, 1, 512);
	}
	if (ImGui::Button("add Device")) {
		// add input for name
		return addDevice(m_fixtureDescriptions[m_selectedFixture]["name"], m_selectedFixture, m_currentAddress);
	}
	
	return nullptr;
}

act::room::MovingHeadRoomNodeRef act::room::DMXManager::getMovingHeadByIndex(int index)
{
	if (index >= m_nodes.size())
		return nullptr;
	MovingHeadRoomNodeRef mh = std::dynamic_pointer_cast<act::room::MovingHeadRoomNode>(m_nodes[index]);
	return mh;
}

/*
returns: RoomNodeBaseRef assigned to the given marker-ID
*/
act::room::RoomNodeBaseRef act::room::DMXManager::getDeviceByMarkerID(int id) {
	for (auto&& node : m_nodes) {
		if (node->getMarkerID() == id) {
			return node;
		}
	}
	return nullptr;
}

act::room::DimmerRoomNodeRef act::room::DMXManager::getDimmerByIndex(int index)
{
	if (index >= m_nodes.size())
		return nullptr;
	DimmerRoomNodeRef mh = std::dynamic_pointer_cast<act::room::DimmerRoomNode>(m_nodes[index]);
	return mh;
}

ci::Json act::room::DMXManager::toJson()
{
	auto json = ci::Json::object();

	if(m_dmxInterface)
		json["interfaceName"] = m_dmxInterface->getDeviceName();

	ci::Json nodes = ci::Json::array();
	for (auto&& node : m_nodes) {
		auto device = std::dynamic_pointer_cast<act::room::DMXRoomNodeBase>(node);
		auto nodeJson = node->toJson();
		nodes.push_back(nodeJson);
	}
	json["nodes"] = nodes;


	ci::Json availableDevices = ci::Json::array();
	for (auto&& name : m_fixtureNames) {
		auto nodeJson = ci::Json::object();
		nodeJson["deviceName"] = name;
		availableDevices.push_back(nodeJson);
	};

	json["availableDevices"] = availableDevices;

	return json;
}

void act::room::DMXManager::fromJson(ci::Json json)
{
	if (json.contains("interfaceName")) {
		std::string interfaceName = json["interfaceName"];
		m_selectedInterface = -1;
		if (std::find_if(m_interfaceNames.begin(), m_interfaceNames.end(), [&](std::string name) {m_selectedInterface++; return name == interfaceName; }) != m_interfaceNames.end()) {
			changeInterface(interfaceName);
		}
		else {
			// could not find prior attached interface
			m_selectedInterface = -1;
		}
	}
	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {
			auto params = node["params"];
			std::string fixtureName = "";
			util::setValueFromJson(params, "fixtureName", fixtureName);
			int startAddress = 0;
			util::setValueFromJson(params, "startAddress", startAddress);

			int fixtureIndex = getFixtureIndexByName(fixtureName);
			if (fixtureIndex < 0) {
				CI_LOG_W("Can not find fixture: " << fixtureName);
				continue;
			}

			std::string name = fixtureName;
			util::setValueFromJson(node, "name", name);
			auto dmxDevice = addDevice(name, fixtureIndex, startAddress);

			dmxDevice->fromJson(node);		
		}
	}
	refreshLists();
}

void act::room::DMXManager::refreshInterfaceNames()
{
	DMXPro::listDevices();
	m_interfaceNames = DMXPro::getDevicesList();

	if (!m_dmxInterface && !m_interfaceNames.empty())
		changeInterface(m_interfaceNames[0]);
}

void act::room::DMXManager::changeInterface(std::string interfaceName)
{
	if (m_dmxInterface)
		m_dmxInterface.reset();

	m_dmxInterface = DMXPro::create(interfaceName);
}

void act::room::DMXManager::loadFixtures()
{
	fs::path path = app::getAssetPath("dmx/fixtures.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "dmx/fixtures.json";
		writeJson(path, ""); // touch
		saveFixtures();
	}

	m_fixtureDescriptions.clear();
	m_fixtureNames.clear();
	ci::Json fixtureDescriptions = ci::loadJson(loadFile(path));

	int smallestID = INT_MAX;
	for (auto&& desc : fixtureDescriptions["devices"]) {
		m_fixtureDescriptions.push_back(desc);
		std::string name = desc["name"];
		m_fixtureNames.push_back(name);
		CI_LOG_I("loaded Fixture: " << name);
	}
}

void act::room::DMXManager::saveDevicesToJson() {
	fs::path path = app::getAssetPath("recentRoomSetup.json");
	ci::Json wholeFile = ci::loadJson(loadFile(path));
	//save nodes
	try {
		wholeFile["dmxManager"]["devices"].clear();

		ci::Json devicesJson = toJson();
		ci::Json devices = ci::Json::array();
		for (auto child : devicesJson) {
			devices.push_back(child);
		}
		wholeFile["dmxManager"] = devices;
		ci::writeJson(path, wholeFile);
	}
	catch (cinder::Exception e) {
		CI_LOG_E(e.what());
	}
	
}

void act::room::DMXManager::saveFixtures()
{
	fs::path path = app::getAssetPath("dmx/fixtures.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "dmx/fixtures.json";
		writeJson(path, ""); // touch
	}

	ci::Json fixtureDescriptions = ci::Json("{\"devices\":[]");
	auto& devices = fixtureDescriptions["devices"];

	for (auto&& desc : m_fixtureDescriptions) {
		devices.push_back(desc);
	}
	ci::writeJson(path, fixtureDescriptions);
}

int act::room::DMXManager::getFixtureIndexByName(std::string fixtureName)
{
	for (int i = 0; i < m_fixtureNames.size(); i++) {
		if (m_fixtureNames[i] == fixtureName)
			return i;
	}
	return -1;
}

void act::room::DMXManager::refreshLists()
{
	m_availableDeviceNames.clear();
	for (auto&& device : m_nodes) {
		m_availableDeviceNames.push_back(device->getName());
	}
}

 
act::room::RoomNodeBaseRef act::room::DMXManager::addDevice(std::string name, int fixtureIndex, int startAddress)
{
	if (fixtureIndex < 0 || fixtureIndex >= m_fixtureDescriptions.size())
		return nullptr;
	auto description = m_fixtureDescriptions[fixtureIndex];
	RoomNodeBaseRef node;
	if (description["type"] == "mv") {
		node = MovingHeadRoomNode::create(m_dmxInterface, description, name, startAddress);
	}
	if (description["type"] == "dimmer") {
		node = DimmerRoomNode::create(m_dmxInterface, description, name, startAddress);
	}
	if (node) {
		m_nodes.push_back(node);
		refreshLists();
		return node;
	}
	return nullptr;
}

int act::room::DMXManager::hasAvailableDevice(std::string deviceName) {
	int i = 0;

	for (auto&& d : m_fixtureNames) {
		
		if (d.compare(deviceName) == 0) {
			return i;
		}
		i++;
	}

	return -1;
}

