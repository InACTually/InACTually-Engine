
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021-2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
	ein-christoph
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

	m_fixtureDescriptionImporter = act::system::FixtureDescriptionImporter::create();

	setupSimpleDimmer();

	refreshInterfaceNames();

	changeInterface("COM3");
}

act::room::DMXManager::~DMXManager()
{
}

void act::room::DMXManager::setup()
{
	m_fixtureDescriptionImporter->registerImportFixtureListener(weak_from_this());
}

void act::room::DMXManager::update()
{
	m_fixtureDescriptionImporter->update();
}

void act::room::DMXManager::cleanUp()
{
	for (auto&& node : m_nodes) {
		node->cleanUp();
	}
}

act::room::RoomNodeBaseRef act::room::DMXManager::drawMenu()
{
	ImGui::Text("DMX Interfaces:");

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

	ImGui::Separator();

	if (m_fixtureNodesMissingDesc.size()) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::Text("The room contains broken fixture nodes!");
		ImGui::PopStyleColor();
		ImGui::Text("The following room nodes are missing their fixture description an will not be processed!");
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 0, 255));
		std::string label = "";
		for (auto&& node : m_fixtureNodesMissingDesc) {
			if (node.contains("name") && node["name"].is_string())
				label = node["name"].get<std::string>();
			else
				label = "ukn";

			if (node.contains("params") && node["params"].contains("fixtureName") && node["params"]["fixtureName"].is_string())
				label = label + ":" + node["params"]["fixtureName"].get<std::string>();
			else
				label = label+":ukn";

			ImGui::Text(label.c_str());
		}
		ImGui::PopStyleColor();
		ImGui::Text("Import a fixture with matching name to resolve the error.");
		ImGui::Checkbox("Keep broken fixture nodes in room file", &m_isSaveBrokenNodes);
		ImGui::Separator();
	}

	ImGui::Text("Add Device(s):");

	if (m_fixtureNames.size() > 0) {

		ImGui::Combo("Device", &m_selectedFixture, m_fixtureNames);
		if (ImGui::InputInt("address", &m_currentAddress)) {
			m_currentAddress = std::clamp(m_currentAddress, 1, 512);
		}
		if (ImGui::Button("add Device")) {
			// add input for name
			return addDevice(m_fixtureDescriptions[m_selectedFixture]["name"], m_selectedFixture, m_currentAddress);
		}
	}
	else {
		ImGui::Text("No fixture descriptions present in room file.");
	}

	//Fixture Importer
	m_fixtureDescriptionImporter->draw();

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

	// save dmx interface used
	if(m_dmxInterface)
		json["interfaceName"] = m_dmxInterface->getDeviceName();

	// save fixture descriptions used for this room
	ci::Json fixtureDescriptions = ci::Json::object();
	fixtureDescriptions["devices"] = ci::Json::array();
	auto& devices = fixtureDescriptions["devices"];

	for (auto&& desc : m_fixtureDescriptions) {
		devices.push_back(desc);
	}

	json["fixtureDescriptions"] = devices;

	// save fixture room nodes
	ci::Json nodes = ci::Json::array();

	for (auto&& node : m_nodes) {
		auto device = std::dynamic_pointer_cast<act::room::DMXRoomNodeBase>(node);
		auto nodeJson = node->toJson();
		nodes.push_back(nodeJson);
	}

	if (m_isSaveBrokenNodes) {
		for (auto&& node : m_fixtureNodesMissingDesc) {
			nodes.push_back(node);
		}
	}

	json["nodes"] = nodes;

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

	if (json.contains("fixtureDescriptions") && json["fixtureDescriptions"].is_array()) {

		m_fixtureDescriptions.clear();
		m_fixtureNames.clear();

		for (auto&& desc : json["fixtureDescriptions"]) {
			m_fixtureDescriptions.push_back(desc);
			std::string name = desc["name"];
			m_fixtureNames.push_back(name);
			CI_LOG_I("loaded Fixture: " << name);
		}

		if (m_fixtureNames.empty())
			setupSimpleDimmer();

	}

	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {
			if(!addRoomNodeFromJson(node))
				m_fixtureNodesMissingDesc.push_back(node);
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

void act::room::DMXManager::setupSimpleDimmer() {
	std::string simpleDimmerName = "Dimmer";

	if (!std::count(m_fixtureNames.begin(), m_fixtureNames.end(), simpleDimmerName)){
		ci::Json simpleDimmerDesc = ci::Json::object();
		simpleDimmerDesc["name"] = simpleDimmerName;
		simpleDimmerDesc["channel"] = 1;
		simpleDimmerDesc["type"] = "dimmer";
		simpleDimmerDesc["mapping"]["dimmer"] = 1;

		m_fixtureDescriptions.push_back(simpleDimmerDesc);
		m_fixtureNames.push_back(simpleDimmerName);
	}
}

void act::room::DMXManager::changeInterface(std::string interfaceName)
{
	if (m_dmxInterface)
		m_dmxInterface.reset();

	m_dmxInterface = DMXPro::create(interfaceName);
}

void act::room::DMXManager::onImportFixture(ci::Json fixtureDescription)
{
	if (!fixtureDescription.contains("name")
		|| !fixtureDescription["name"].is_string()
		|| !fixtureDescription.contains("type")
		|| !fixtureDescription.contains("mapping")) {
		CI_LOG_E("importFixture called with malformed fixtureDescription. Fixture description has to have a name, a type and a mapping!");
		return;
	}

	std::string fixturename = fixtureDescription["name"];

	if (std::find(m_fixtureNames.begin(), m_fixtureNames.end(), fixturename) != m_fixtureNames.end()) {
		CI_LOG_E("Fixture with the name '" + fixturename + "' already exists!");
		return;
	}

	m_fixtureDescriptions.push_back(fixtureDescription);
	m_fixtureNames.push_back(fixturename);
	CI_LOG_I("imported Fixture loaded:" << fixturename);

	if (m_fixtureNodesMissingDesc.size()) {
		CI_LOG_I("Checking fixture room nodes with missing description ...");

		std::erase_if(m_fixtureNodesMissingDesc, [this](ci::Json& node) {
				try {
					return addRoomNodeFromJson(node);
				}
				catch (const std::exception& e) {
					CI_LOG_E("Failed to addRoomNodeFromJson on fixture missing description : " << e.what());
				}
				return false;
			});
	}
}

void act::room::DMXManager::saveDevicesToJson() {
	ci::fs::path path = ci::app::getAssetPath("recentRoomSetup.json");
	ci::Json wholeFile = ci::loadJson(ci::loadFile(path));
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

int act::room::DMXManager::getFixtureIndexByName(std::string fixtureName)
{
	for (int i = 0; i < m_fixtureNames.size(); i++) {
		if (m_fixtureNames[i] == fixtureName)
			return i;
	}
	return -1;
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

bool act::room::DMXManager::addRoomNodeFromJson(ci::Json json) {
	auto params = json["params"];
	std::string fixtureName = "";
	util::setValueFromJson(params, "fixtureName", fixtureName);
	int startAddress = 0;
	util::setValueFromJson(params, "startAddress", startAddress);

	int fixtureIndex = getFixtureIndexByName(fixtureName);
	if (fixtureIndex < 0) {
		CI_LOG_W("Can not find fixture: " << fixtureName);
		return false;
	}

	std::string name = fixtureName;
	util::setValueFromJson(json, "name", name);
	auto dmxDevice = addDevice(name, fixtureIndex, startAddress);

	dmxDevice->fromJson(json);
	return true;
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

