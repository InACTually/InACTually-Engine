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

#include "roompch.hpp"
#include "lidar/LidarManager.hpp"
#include "lidar/LDLidarRoomNode.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

act::room::LidarManager::LidarManager()
	: RoomNodeManagerBase("lidarManager")
{
	m_selectedDevice = 0;
	m_fixtureNames = std::vector<std::string>(0);
	m_availableDeviceNames = std::vector<std::string>(0);

	loadFixtures();
	refreshLists();
}

act::room::LidarManager::~LidarManager()
{	
}

void act::room::LidarManager::setup()
{
}

void act::room::LidarManager::cleanUp()
{
	for (auto&& node : m_nodes) {
		node->cleanUp();
	}
}

act::room::RoomNodeBaseRef act::room::LidarManager::drawMenu()
{
	if (ImGui::Button("refresh Devicelist")) {
		refreshLists();
	}

	if (ImGui::Combo("use Device", &m_selectedDevice, m_fixtureNames)) {
		return addDevice(m_fixtureNames[m_selectedDevice]);
	}

	return nullptr;
}

act::room::RoomNodeBaseRef act::room::LidarManager::getDeviceByMarkerID(int id) {
	for (auto&& node : m_nodes) {
		if (node->getMarkerID() == id) {
			return node;
		}
	}
	return nullptr;
}

act::room::LDLidarRoomNodeRef act::room::LidarManager::getLDLidarByIndex(int index)
{
	if (index >= m_nodes.size())
		return nullptr;
	LDLidarRoomNodeRef ld = std::dynamic_pointer_cast<act::room::LDLidarRoomNode>(m_nodes[index]);
	return ld;
}

ci::Json act::room::LidarManager::toJson()
{
	auto json = ci::Json::object();

	ci::Json nodes = ci::Json::array();
	for (auto&& node : m_nodes) {
		auto device = std::dynamic_pointer_cast<act::room::LidarRoomNodeBase>(node);
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

void act::room::LidarManager::fromJson(ci::Json json)
{
	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {
			auto params = node["params"];
			std::string fixtureName = "";
			util::setValueFromJson(params, "fixtureName", fixtureName);

			if (!m_fixtureDescriptions.contains(fixtureName)) {
				CI_LOG_W("Can not find fixture: " << fixtureName);
				continue;
			}

			std::string name = fixtureName;
			util::setValueFromJson(node, "name", name);
			auto lidarDevice = addDevice(name);

			lidarDevice->fromJson(node);		
		}
	}
	refreshLists();
}

void act::room::LidarManager::loadFixtures()
{
	fs::path path = app::getAssetPath("lidar/fixtures.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "lidar/fixtures.json";
		writeJson(path, ""); // touch
		saveFixtures();
	}

	m_fixtureDescriptions.clear();
	m_fixtureNames.clear();
	ci::Json fixtureDescriptions = ci::loadJson(loadFile(path));

	int smallestID = INT_MAX;
	for (auto&& desc : fixtureDescriptions["devices"]) {
		std::string name = desc["name"];
		m_fixtureDescriptions[name] = desc;
		m_fixtureNames.push_back(name);
		CI_LOG_I("loaded Fixture: " << name);
	}
}

void act::room::LidarManager::saveDevicesToJson() {
	fs::path path = app::getAssetPath("recentRoomSetup.json");
	ci::Json wholeFile = ci::loadJson(loadFile(path));
	//save nodes
	try {
		wholeFile["lidarManager"]["devices"].clear();

		ci::Json devicesJson = toJson();
		ci::Json devices = ci::Json::array();
		for (auto child : devicesJson) {
			devices.push_back(child);
		}
		wholeFile["lidarManager"] = devices;
		ci::writeJson(path, wholeFile);
	}
	catch (cinder::Exception e) {
		CI_LOG_E(e.what());
	}
	
}

void act::room::LidarManager::saveFixtures()
{
	fs::path path = app::getAssetPath("lidar/fixtures.json");
	if (path.empty()) {
		path = app::getAssetPath("").string() + "lidar/fixtures.json";
		writeJson(path, ""); // touch
	}

	ci::Json fixtureDescriptions = ci::Json("{\"devices\":[]");
	auto& devices = fixtureDescriptions["devices"];

	for (auto&& desc : m_fixtureDescriptions) {
		devices.push_back(desc);
	}
	ci::writeJson(path, fixtureDescriptions);
}

void act::room::LidarManager::refreshLists()
{
	m_availableDeviceNames.clear();

	for(auto&&node : m_nodes) {
		m_availableDeviceNames.push_back(node->getName());
	}
}
 
act::room::RoomNodeBaseRef act::room::LidarManager::addDevice(std::string name)
{
	ci::Json description = m_fixtureDescriptions[name];

	std::string portName = "COM3";
	util::setValueFromJson(description, "portName", portName);

	int baudrate = 230400;
	util::setValueFromJson(description, "baudrate", baudrate);

	CI_LOG_I("LidarManager: opening " << portName << " at " << baudrate << " baud");

	RoomNodeBaseRef node;

	if (description["type"] == "LD19") {
		node = LDLidarRoomNode::create(portName, description, name);
	}

	if (node) {
		m_nodes.push_back(node);
		return node;
	}

	CI_LOG_E("LidarManager: unknown device type in fixture: " << name);
	return nullptr;
}
