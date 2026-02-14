
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
#include "camera/CameraManager.hpp"

#include <algorithm> 

act::room::CameraManager::CameraManager()
	: RoomNodeManagerBase("cameraManager")
{
	m_selectedDevice = 0;
	m_usedDevicesNames = std::vector<std::string>(0);
	refreshLists();
	
}

act::room::CameraManager::~CameraManager()
{
}

void act::room::CameraManager::setup()
{

}

act::room::RoomNodeBaseRef act::room::CameraManager::drawMenu()
{
	//ImGui::SetNextItemWidth(m_displaySize.x - ImGui::CalcTextSize("Device").x);
	ImGui::Combo("Device", &m_selectedDevice, m_availableDeviceNames);

	
	if (ImGui::Button("add Device")) {
		m_calibrator = CameraCalibrator::create();
		m_calibrator->setCamera(m_currentCamera); 
		m_doCalibrate = true;
	}
	
	if (ImGui::Button("refresh Devicelist")) {
		refreshLists();
	}

	if (m_doCalibrate) {
		ImGui::OpenPopup("Add Device");
	}

	if (ImGui::BeginPopupModal("Add Device")) {


		m_calibrator->draw();

		
		ImGui::NewLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
			m_doCalibrate = false;
			addSelectedDevice(m_availableDeviceNames[m_selectedDevice], m_availableDeviceNames[m_selectedDevice]);
		}
		ImGui::SameLine();
		if (m_calibrator->isFinishedCalibrating() && ImGui::Button("Apply")) {
			ImGui::CloseCurrentPopup();
			m_doCalibrate = false;
			addSelectedDevice(m_availableDeviceNames[m_selectedDevice], m_availableDeviceNames[m_selectedDevice]);
		}
		

		ImGui::EndPopup();
	}

	return nullptr;
}

void act::room::CameraManager::update()
{
	for (auto&& node : m_nodes) {
		node->update();
	}
	if (m_selectedDevice != m_prevSelectedDevice) {
		if(m_availableDeviceNames.size() > 0) {
			setCameraByDeviceName(m_availableDeviceNames[m_selectedDevice]);

			//m_capture = Capture::create(captureSize.x, captureSize.y, m_device);
			//m_capture->start();
			m_prevSelectedDevice = m_selectedDevice;
		}
	}
	if (!m_currentCamera) return;
	else m_currentCamera->update();

	//if (m_currentCamera->m_capture)
	//	m_currentCamera->m_captureSurface = m_currentCamera->m_capture->getSurface();

	if (m_doCalibrate) {
		m_calibrator->update();
	}
	else {

	}
	

}

ci::Json act::room::CameraManager::toJson()
{
	auto json = ci::Json::object();

	ci::Json nodes = ci::Json::array();
	for (auto&& node : m_nodes) {
		//auto device = std::dynamic_pointer_cast<act::room::CameraRoomNode>(node);
		auto nodeJson = node->toJson();

		nodes.push_back(nodeJson);
	}
	json["nodes"] = nodes;

	ci::Json availableDevices = ci::Json::array();
	for (auto&& name : m_availableDeviceNames) {
		auto deviceJson = ci::Json::object();
		deviceJson["deviceName"] = name; 
	
		availableDevices.push_back(deviceJson);
	};

	json["availableDevices"] = availableDevices;

	return json;	
}

void act::room::CameraManager::fromJson(ci::Json json)
{
	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {
			std::string deviceName = "";
			util::setValueFromJson(node, "deviceName", deviceName);

			std::string name = deviceName;
			util::setValueFromJson(node, "name", name);
			auto camRoomNode = addDevice(deviceName, name);

			camRoomNode->fromJson(node);

			refreshLists();
		}
	}
}

act::room::CameraRoomNodeRef act::room::CameraManager::getCamera(act::UID cameraUID) {
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[cameraUID](RoomNodeBaseRef node) {
			return !node->getUID().compare(cameraUID);
		});
	if (nodeIter != std::end(m_nodes)) {
		return std::dynamic_pointer_cast<CameraRoomNode> (*nodeIter);
	}
	return nullptr;
}

act::room::CameraRoomNodeRef act::room::CameraManager::getCameraByIndex(int index) {
	if (index < m_usedDevicesUID.size()) {
		return getCamera(m_usedDevicesUID[index]);
	}
	return nullptr;
}

act::proc::ImageOutputPortRef act::room::CameraManager::getCameraPort(act::UID cameraUID)
{
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[cameraUID](RoomNodeBaseRef node) {
			return !node->getUID().compare(cameraUID);
		});
	if (nodeIter != std::end(m_nodes)) {
		CameraRoomNodeRef camera = std::dynamic_pointer_cast<CameraRoomNode> (*nodeIter);
		if (camera)
			return camera->getCameraImagePort();
	}
	return nullptr;
}

void act::room::CameraManager::refreshLists()
{
	m_usedDevicesNames.clear();
	m_usedDevicesUID.clear();
	for (auto&& camera : m_nodes) {
		m_usedDevicesNames.push_back(camera->getName());
		m_usedDevicesUID.push_back(camera->getUID());
	}

	// TODO: needs to be reconsidered
	m_availableDeviceNames.clear();
	m_devices = cinder::Capture::getDevices();
	for (auto&& d : m_devices) {
		if(std::find_if(m_nodes.begin(), m_nodes.end(), [d](RoomNodeBaseRef node) { return node->getName() == d->getName();}) == m_nodes.end())
			if (d->getName() != kinectNameFilter) {
				if (!(std::find(m_usedDevicesNames.begin(), m_usedDevicesNames.end(), d->getName()) != m_usedDevicesNames.end())) {
					m_availableDeviceNames.push_back(d->getName());
				}
				
			}
	}

	
}

void act::room::CameraManager::setCameraByDeviceName(std::string deviceName)
{
	ci::Capture::DeviceRef device = *std::find_if(
		std::begin(m_devices),
		std::end(m_devices),
		[&](ci::Capture::DeviceRef device) {
			return device->getName() == deviceName;
		}
	);
	if (device)	
		m_currentCamera = act::room::CameraDevice::create(device);
}

act::room::RoomNodeBaseRef act::room::CameraManager::addDevice(std::string deviceName, std::string name)
{
	
	auto device = *std::find_if(std::begin(m_devices), std::end(m_devices) - 1, [&](ci::Capture::DeviceRef device) { return device->getName() == deviceName; });
	if (device) {
		if (m_currentCamera && m_currentCamera->getName() == deviceName) {
			auto cam = CameraRoomNode::create(m_currentCamera, name);
			m_nodes.push_back(cam);
			refreshLists();
			return cam;
		}
		else {
			auto cam = CameraRoomNode::create(device, device->getName(), name);
			m_nodes.push_back(cam);
			refreshLists();
			return cam;
		}
	}
	else { // if device not available anymore, i.e. loading Nodes
		CI_LOG_W("Cannot find CameraDevice " << deviceName << "!");
		auto cam = CameraRoomNode::create(nullptr, deviceName, name);
		m_nodes.push_back(cam);
		return cam;
		//refreshDeviceList();
	}

	return nullptr;
}

act::room::RoomNodeBaseRef act::room::CameraManager::addSelectedDevice(std::string deviceName, std::string name) {
	if (m_currentCamera->hasCapture()) {

		auto cam = CameraRoomNode::create(m_currentCamera, name);
		m_nodes.push_back(cam);
		refreshLists();
		return cam;
	}
	else { // if device not available anymore, i.e. loading Nodes
		CI_LOG_W("Cannot find CameraDevice " << deviceName << "!");
		auto cam = CameraRoomNode::create(nullptr, name);
		m_nodes.push_back(cam);
		return cam;
		//refreshDeviceList();
	}
}

bool act::room::CameraManager::hasAvailableDevice(std::string deviceName) {
	
	for (auto&& d : m_availableDeviceNames) {
		if (d.compare(deviceName) == 0) {
			return true;
		}
	}

	return false;
}
