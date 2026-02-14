
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
#include "object/ObjectManager.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::room::ObjectManager::ObjectManager() : RoomNodeManagerBase("objectManager") {
}

act::room::ObjectManager::ObjectManager(CameraManagerRef cameraMgr) : RoomNodeManagerBase("objectManager")
{
	m_cameraMgr = cameraMgr;
	checkCameras();
}


act::room::ObjectManager::~ObjectManager()
{
}


void act::room::ObjectManager::setup()
{

}


void act::room::ObjectManager::update()
{
	checkCameras();

	processObjects();

	/*
	if (m_newObjectQueue.size() > 0) {
		int id = m_newObjectQueue.front();
		m_newObjectQueue.pop();
		//addObject(id);
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

act::room::RoomNodeBaseRef act::room::ObjectManager::drawMenu() {

	

	if (ImGui::Button("Create Test-Object")) {
		auto node = ObjectRoomNode::create(-1, "test object");
		node->setPosition(ci::vec3(0.0f, 0.0f, 0.0f));
		m_nodes.push_back(node);
	}

	for (auto&& tex : m_feedbackTextures) { 
		ImGui::Image(tex, ci::vec2(tex->getSize()) * 0.6f);
	}

	for (auto&& node : m_nodes) {
		auto n = std::dynamic_pointer_cast<ObjectRoomNode>(node);
		if (n) {
			std::stringstream strstr;
			strstr << n->getName();
			ImGui::Text(strstr.str().c_str());
		}
	}
	
	return nullptr;
}

act::room::ObjectRoomNodeRef act::room::ObjectManager::addObject(act::comp::ObjectCandidate candidate)
{
	std::stringstream strstr;
	strstr << candidate.className << " - " << candidate.id;
	ObjectRoomNodeRef object;
	object = ObjectRoomNode::create(candidate.id, strstr.str());
	m_nodes.push_back(object);
	
	return object;
}


ci::Json act::room::ObjectManager::toJson() {
	auto json = ci::Json::object();


	return json;
}

void act::room::ObjectManager::fromJson(ci::Json json) {

}

void act::room::ObjectManager::checkCameras()
{
	for (auto&& node : m_cameraMgr->getNodes()) {
		auto camera = std::dynamic_pointer_cast<CameraRoomNode>(node);
		if (!camera)
			continue;
		
		if (camera->isDetectingObjects() && !m_objectDetectors[camera->getUID()]) {
			m_objectDetectors[camera->getUID()] = comp::ObjectDetector::create(camera);
		}
		else if (!camera->isDetectingObjects() && m_objectDetectors[camera->getUID()]) {
			m_objectDetectors.erase(camera->getUID());
		}			
	}
}

void act::room::ObjectManager::processObjects() {
	std::vector<gl::TextureRef> feedbackTextures;

	for (auto&& objectDetector : m_objectDetectors) {	
		auto&& detector = objectDetector.second;

		if (!detector || !detector->hasNewCandidates())
			continue;

		auto tex = detector->getFeedbackTexture();
		if (tex && tex->getWidth() > 0 && tex->getHeight() > 0)
			feedbackTextures.push_back(tex);

		auto&& cantidates = detector->getCandidates();

		for (auto&& candidate : cantidates) {
			bool newObject = true;
			room::ObjectRoomNodeRef object;
			for (auto node : m_nodes) {
				room::ObjectRoomNodeRef objectNode = std::dynamic_pointer_cast<room::ObjectRoomNode> (node);
				if (objectNode->getID() == candidate.id) {
					object = objectNode;
					newObject = false;
					break;
				}
			}

			if (newObject) {
				object = addObject(candidate);
			}

			if (!object) {
				continue;
			}

			if (object->isFixed() && !object->isDynamic()) {
				
			}
			else if (object->isDynamic()) {
				//ci::vec3 objectPosition = detector->transformPosition(candidate.tvec, detector->getCamera()->getPosition(), detector->getCamera()->getOrientation());
				//glm::mat4 objectOrientierung = detector->transformRotation(candidate.tvec, candidate.rvec, detector->getCamera()->getPosition(), detector->getCamera()->getOrientation());

				//object->setPosition(objectPosition);
				//object->setOrientation(objectOrientierung);
			}
		}
	}

	if(feedbackTextures.size() > 0)
		m_feedbackTextures = feedbackTextures;
}

