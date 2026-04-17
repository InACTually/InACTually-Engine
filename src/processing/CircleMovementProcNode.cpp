
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "CircleMovementProcNode.hpp"

act::proc::CircleMovementProcNode::CircleMovementProcNode() : ProcNodeBase("CircleMovement") { 
	m_drawSize = ivec2(200, 200);

	m_position = vec3(0.0f, 0.0f, 0.0f);
	m_angle = 0.0f;
	m_speed = 0.1f;
	m_radius = 1.0f;

	m_positionPort = createVec3Output("position");

	createVec3Input("center", [&](vec3 center) { m_position = center; });
	createNumberInput("radius", [&](number radius) { m_radius = radius; });
	createNumberInput("speed", [&](number speed) { m_speed = speed; });

}

act::proc::CircleMovementProcNode::~CircleMovementProcNode() {
}

void act::proc::CircleMovementProcNode::update() {
	m_angle += m_speed * 0.1f;
	vec3 vec = vec3(cosf(m_angle), 0.0f, sinf(m_angle)) * m_radius;

	m_positionPort->send(m_position + vec);
}

void act::proc::CircleMovementProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x);

	if (ImGui::DragFloat3("center", &m_position, 0.01f)) {
		preventDrag(true);
		update();
	}
	else {
		preventDrag(false);
	}

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("radius", &m_radius, 0.01f)) {
		preventDrag(true);
		update();
	}
	else {
		preventDrag(false);
	}

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("speed", &m_speed, 0.01f)) {
		preventDrag(true);
		update();
	}
	else {
		preventDrag(false);
	}

	endNodeDraw();
}

ci::Json act::proc::CircleMovementProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["x"]		= m_position.x;
	json["y"]		= m_position.y;
	json["z"]		= m_position.z;
	json["radius"]	= m_radius;
	json["speed"]	= m_speed;
	return json;
}

void act::proc::CircleMovementProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "x", m_position.x);
	util::setValueFromJson(json, "y", m_position.y);
	util::setValueFromJson(json, "z", m_position.z);
	util::setValueFromJson(json, "radius", m_radius);
	util::setValueFromJson(json, "speed", m_speed);
}
