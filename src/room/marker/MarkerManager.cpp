
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "marker/MarkerManager.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::room::MarkerManager::MarkerManager() : RoomNodeManagerBase("markerManager") {
}

act::room::MarkerManager::MarkerManager(CameraManagerRef cameraMgr) : RoomNodeManagerBase("markerManager")
{
	m_cameraMgr = cameraMgr;
	checkCameras();
}


act::room::MarkerManager::~MarkerManager()
{
}


void act::room::MarkerManager::setup()
{

}


void act::room::MarkerManager::update()
{
	checkCameras();

	processMarkers();

	/*
	if (m_newMarkerQueue.size() > 0) {
		int id = m_newMarkerQueue.front();
		m_newMarkerQueue.pop();
		//addMarker(id);
	}

	if (m_imgTexQueue.size() > 0)
		m_textures.clear();
	while (m_imgTexQueue.size() > 0) {
		auto img = fromOcv(m_imgTexQueue.front());
		if (img)
			m_textures.push_back(gl::Texture::create(img, ci::gl::Texture::Format().loadTopDown()));
		m_imgTexQueue.pop();
	}
	*/
}

act::room::RoomNodeBaseRef act::room::MarkerManager::drawMenu() {

	if (ImGui::InputInt("Origin Marker ID", &m_originMarkerID, 1, 1)) {
		m_originMarkerID = std::clamp(m_originMarkerID, 0, 4095);
	};

	if (ImGui::InputFloat("Marker Size", &m_markerSize)) {
		for(auto&& detector : m_markerDetectors)
			detector.second->setMarkerSize(m_markerSize);
	}
	/*if (ImGui::InputFloat("Other Markers Size", &m_otherMarkerSize)) {

	}*/

	if (ImGui::Button("Create Marker")) {
		auto node = MarkerRoomNode::create(1);
		node->setPosition(ci::vec3(0.0f, 0.0f, 0.0f));
		m_nodes.push_back(node);
	}

	for (auto&& tex : m_feedbackTextures) {
		ImGui::Image(tex, ci::vec2(tex->getSize()) * 0.3f);
	}
	
	return nullptr;
}

act::room::MarkerRoomNodeRef act::room::MarkerManager::addMarker(int id)
{
	MarkerRoomNodeRef marker;
	if (id != -1 && !getMarkerByID(id)) {
		m_markerIDs.push_back(id);
		marker = MarkerRoomNode::create(id);
		m_nodes.push_back(marker);
	}
	return marker;
}


ci::Json act::room::MarkerManager::toJson() {
	auto json = ci::Json::object();


	return json;
}

void act::room::MarkerManager::fromJson(ci::Json json) {

}

act::proc::OutputPortRef<vec3> act::room::MarkerManager::getMarkerPort(int markerID)
{
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[markerID](RoomNodeBaseRef node) {
			return !node->getName().compare(std::to_string(markerID));
		});
	if (nodeIter != std::end(m_nodes)) {
		MarkerRoomNodeRef marker = std::dynamic_pointer_cast<MarkerRoomNode> (*nodeIter);
		//if (marker)
		//	return marker->getOutputPort();
	}
	return nullptr;
}

std::vector<std::string> act::room::MarkerManager::getMarkerIDsAsStrings()
{
		std::vector<std::string> markerStrings;
		for (auto markerID : m_markerIDs) {
			markerStrings.push_back(std::to_string(markerID));
		}
		return markerStrings;
}

act::room::MarkerRoomNodeRef  act::room::MarkerManager::getMarkerByID(int id) {
	MarkerRoomNodeRef marker;
	for (auto&& node : m_nodes) {
		marker = std::dynamic_pointer_cast<MarkerRoomNode> (node);
		if (marker->getID() == id) {
			return marker;
		}
	}
	//rkerRoomNodeRef marker = MarkerRoomNode::create(id);
	//nodes.push_back(marker);
	return nullptr;
}

void act::room::MarkerManager::checkCameras()
{
	for (auto&& node : m_cameraMgr->getNodes()) {
		auto camera = std::dynamic_pointer_cast<CameraRoomNode>(node);
		if (!camera)
			continue;
		
		if (camera->isDetectingMarkers() && !m_markerDetectors[camera->getUID()]) {
			m_markerDetectors[camera->getUID()] = comp::MarkerDetector::create(camera);
		}
		else if (!camera->isDetectingMarkers() && m_markerDetectors[camera->getUID()]) {
			m_markerDetectors.erase(camera->getUID());
		}			
	}
}

void act::room::MarkerManager::processMarkers() {
	std::vector<gl::TextureRef> feedbackTextures;

	for (auto&& markerDetector : m_markerDetectors) {	
		auto&& detector = markerDetector.second;

		if (!detector || !detector->hasNewCandidates())
			continue;

		auto tex = detector->getFeedbackTexture();
		if (tex && tex->getWidth() > 0 && tex->getHeight() > 0)
			feedbackTextures.push_back(tex);

		auto&& cantidates = detector->getCandidates();

		for (auto&& candidate : cantidates) {
			bool newMarker = true;
			room::MarkerRoomNodeRef marker;
			for (auto node : m_nodes) {
				room::MarkerRoomNodeRef markerNode = std::dynamic_pointer_cast<room::MarkerRoomNode> (node);
				if (markerNode->getID() == candidate.id) {
					marker = markerNode;
					newMarker = false;
					break;
				}
			}

			if (newMarker) {
				marker = addMarker(candidate.id);
			}

			if (!marker) {
				continue;
			}

			if (marker->isFixed() && !marker->isDynamic()) {
				detector->setCameraInverseToMarker(candidate.rvec, candidate.tvec, marker->getPosition(), marker->getOrientation());
			}
			else if (marker->isDynamic()) {
				ci::vec3 markerPosition = detector->transformPosition(candidate.tvec, detector->getCamera()->getPosition(), detector->getCamera()->getOrientation());
				glm::mat4 markerOrientierung = detector->transformRotation(candidate.tvec, candidate.rvec, detector->getCamera()->getPosition(), detector->getCamera()->getOrientation());

				marker->setPosition(markerPosition);
				marker->setOrientation(markerOrientierung);
			}
		}
	}

	if (feedbackTextures.size() > 0)
		m_feedbackTextures = feedbackTextures;
}

