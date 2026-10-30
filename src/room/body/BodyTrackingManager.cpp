
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "body/BodyTrackingManager.hpp"

#include <algorithm> 

act::room::BodyTrackingManager::BodyTrackingManager(CameraManagerRef cameraMgr) : RoomNodeManagerBase("bodyTrackingManager")
{
	m_cameraMgr = cameraMgr;
	m_bodiesOutPort = act::proc::OutputPort<std::vector<act::room::BodyRef>>::create(act::proc::PT_BODYLIST, "bodies");
}

act::room::BodyTrackingManager::~BodyTrackingManager()
{
}

void act::room::BodyTrackingManager::setup()
{
}

void act::room::BodyTrackingManager::update()
{
	// TODO: m_bodies = m_cameraMgr->getBodies();
	if(!m_wereZeroBodiesBefore)
		m_bodiesOutPort->send(m_bodies);

	m_wereZeroBodiesBefore = m_bodies.size() == 0;

	if(m_nodes.size() > m_bodies.size())
		m_nodes.resize(m_bodies.size());

	for (int i = 0; i < m_bodies.size(); i++) {
		if (m_nodes.size() > i)
			dynamic_pointer_cast<BodyRoomNode>(m_nodes[i])->setBody(m_bodies[i]);
		else
			addBody(m_bodies[i]);
	}

	for (auto&& node : m_nodes) {
		node->update();
	}
}

void act::room::BodyTrackingManager::draw()
{
	ci::gl::ScopedLineWidth scpLW(15);

	for (auto&& node : m_nodes) {
		node->draw();
	}

	ci::gl::lineWidth(1);
}

act::room::RoomNodeBaseRef act::room::BodyTrackingManager::drawMenu()
{
	return nullptr;
}

ci::Json act::room::BodyTrackingManager::toJson()
{
	auto json = ci::Json::object();
	json["name"] = getName();

	return json;
}

void act::room::BodyTrackingManager::fromJson(ci::Json json)
{
}

act::room::BodyRoomNodeRef act::room::BodyTrackingManager::getBodyRoomNodeByBodyUID(UID uid)
{
	act::room::BodyRoomNodeRef bodyNode;

	for (auto&& node : m_nodes) {
		auto body = dynamic_pointer_cast<BodyRoomNode>(node);
		if (body->getBodyUID() == uid) {
			bodyNode = body;
			break;
		}
	}

	return bodyNode;
}

act::room::BodyRoomNodeRef act::room::BodyTrackingManager::addBody(BodyRef body)
{
	BodyRoomNodeRef bodyNode = BodyRoomNode::create("body");
	bodyNode->setBody(body);
	m_nodes.push_back(bodyNode);

	return bodyNode;
}
