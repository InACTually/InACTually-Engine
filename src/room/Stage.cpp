/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "Stage.hpp"

act::room::Stage::Stage()
	: RoomNodeBase("stage")
{
	auto colorShader = ci::gl::getStockShader(ci::gl::ShaderDef().color());

	m_size = glm::vec3(12.0f, 3.5f, 11.0f);

	m_wirePlane	= ci::gl::Batch::create(ci::geom::WirePlane().size(glm::vec2(15)).subdivisions(glm::ivec2(15)), colorShader);
	m_wireRoom	= ci::gl::Batch::create(ci::geom::WireCube(), colorShader);
}

act::room::Stage::~Stage()
{
}

void act::room::Stage::setup(act::room::RoomManagersRef roomMgrs)
{
	m_roomMgrs = roomMgrs;
}

void act::room::Stage::update()
{
	for (auto&& node : m_nodes) {
		node->update();
	}
}

void act::room::Stage::draw()
{

	ci::gl::color(0.3f, 0.3f, 0.3f);
	//ci::gl::drawStrokedCube(m_size * glm::vec3(0.5f), m_size);
	//ci::gl::drawBillboard(glm::vec3(0, 0, 0), glm::vec2(0.15, 0.15), 0.0, glm::vec3(0.0, 0.0, 1.0), glm::vec3(1.0, 0.0, 0.0));
	
	//ci::gl::drawCoordinateFrame();
	util::drawCoords();
	
	ci::gl::ScopedMatrices push;

	// Enable depth buffer.
	ci::gl::ScopedDepth depth(true);

	{
		ci::gl::pushMatrices();
		ci::gl::translate(-6.0f, 0.0f, -5.5f);
		ci::gl::ScopedColor color(ci::Color::gray(0.25f)); 
		ci::gl::translate(m_size * glm::vec3(0.5f, 0.0f, 0.5f));
		m_wirePlane->draw();

		ci::gl::ScopedModelMatrix model;

		//ci::gl::multModelMatrix(ci::translate(m_bounds.getCenter()) * ci::scale(m_bounds.getSize()));
		//m_wireRoom->draw();
		ci::gl::popMatrices();
	}

	ci::gl::pushMatrices();
	for (auto&& node : m_nodes) {
		node->draw();
	}
	for(auto&& mgr : m_roomMgrs->list)
		mgr->draw();

	ci::gl::popMatrices();

}

void act::room::Stage::addNode(RoomNodeBaseRef node)
{
	m_nodes.push_back(node);
}

act::room::RoomNodeBaseRef act::room::Stage::getNodeByUID(act::UID uid)
{
	for (auto&& node : m_nodes) {
		if (node->getUID() == uid)
			return node;
	}
	for (auto&& mgr : m_roomMgrs->list) {
		auto node = mgr->getNodeByUID(uid);
		if (node)
			return node;
	}
	return nullptr;
}

bool act::room::Stage::hit(glm::vec3 pos)
{
	for (auto&& node : m_nodes) {
		if (node->hit(pos)) 
			return true;
	}
	for (auto&& mgr : m_roomMgrs->list) {
		if (mgr->hit(pos))
			return true;
	}
	return false;
}

bool act::room::Stage::hitRay(ci::Ray ray)
{
	for (auto&& node : m_nodes) {
		if (node->hitRay(ray))
			return true;
	}
	for (auto&& mgr : m_roomMgrs->list) {
		if (mgr->hitRay(ray))
			return true;
	}
	return false;
}

act::room::RoomNodeBaseRef act::room::Stage::getNodeAtPos(glm::vec3 pos)
{
	for (auto&& node : m_nodes) {
		if (node->hit(pos))
			return node;
	}
	for (auto&& mgr : m_roomMgrs->list) {
		auto node = mgr->getNodeAtPos(pos);
		if (node)
			return node;
	}
	return nullptr;
}

act::room::RoomNodeBaseRef act::room::Stage::getNodeOnRay(ci::Ray ray)
{
	float distance = FLT_MAX;
	RoomNodeBaseRef candidate;

	for (auto&& node : m_nodes) {
		if (node->hitRay(ray)) {
			float d = ci::distance(node->getPosition(), ray.getOrigin());
			if (distance > d) {
				distance = d;
				candidate = node;
			}
		}
	}
	for (auto&& mgr : m_roomMgrs->list) {
		auto node = mgr->getNodeOnRay(ray);
		if (node) {
			float d = ci::distance(node->getPosition(), ray.getOrigin());
			if (distance > d) {
				distance = d;
				candidate = node;
			}
		}
	}

	return candidate;
}


bool act::room::Stage::removeNode(act::UID uid)
{
	if (m_selectedNode->getUID() == uid)
		m_selectedNode.reset();

	size_t nsize = m_nodes.size();
	bool removed = false;

	m_nodes.erase(		std::remove_if(m_nodes.begin(),		m_nodes.end(),		[&](RoomNodeBaseRef node) {	return node->getUID() == uid; }), m_nodes.end());
	removed = removed || nsize != m_nodes.size();

	if(!removed)
	for (auto&& mgr : m_roomMgrs->list) {
		removed = removed || mgr->removeNode(uid);
		if (removed)
			return removed;
	}

	return removed;
}

void act::room::Stage::clear()
{
	for (auto&& mgr : m_roomMgrs->list)
		mgr->clear();
	m_nodes.clear();
}

std::vector<act::room::RoomNodeBaseRef> act::room::Stage::getAllNodes()
{
	std::vector<RoomNodeBaseRef> nodes;
	for (auto&& node : m_nodes) {
		nodes.push_back(node);
	}
	for (auto&& mgr : m_roomMgrs->list) {
		for (auto node : mgr->getNodes()) {
			nodes.push_back(node);
		}
		
	}
	return nodes;
}

void act::room::Stage::setSelectedNode(RoomNodeBaseRef node)
{
	if (node) {
		if (m_selectedNode)
			m_selectedNode->setIsSelected(false);
		m_selectedNode = node;
		m_selectedNode->setIsSelected(true);
	}
}

ci::Json act::room::Stage::toJson()
{
	auto stageJson = ci::Json::object();
	auto sizeJson = util::valueToJson(m_size);
	
	stageJson["size"] = sizeJson;

	return stageJson;
}

void act::room::Stage::fromJson(ci::Json json, act::UID replyUID)
{
	util::setValueFromJson(json, "size", m_size);
}
