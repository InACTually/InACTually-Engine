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
#include "lidar/LidarRoomNodeBase.hpp"


act::room::LidarRoomNodeBase::LidarRoomNodeBase(const std::string& portName, ci::Json description, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase(name, position, rotation, radius, replyUID)
{
	m_portName		= portName;
	m_fixtureName	= description["name"];
	m_maxDistance	= description["distance"];

	setTriMesh(ci::TriMesh::create(ci::geom::Cylinder()));
}

act::room::LidarRoomNodeBase::~LidarRoomNodeBase()
{
	
}


void act::room::LidarRoomNodeBase::setup()
{

}

void act::room::LidarRoomNodeBase::update()
{

}

void act::room::LidarRoomNodeBase::draw()
{
	gl::ScopedColor color;

	gl::pushMatrices();
	gl::translate(m_position);

	gl::color(Color::white());
	enableStatusColor();

	// device body
	gl::drawCube(ci::vec3(-0.025f, -0.3f, -0.025f), ci::vec3(0.025f, 0.0f, 0.025f));
	gl::scale(0.05f, 0.05f, 0.05f);
	m_mesh->draw();

	gl::popMatrices();

	auto& points = getData();
	if (points.empty())
		return;

	// build a VBO from the current scan in one go
	std::vector<ci::vec3> verts;
	verts.reserve(points.size());
	for (const auto& p : points) {
		// lidar data is in the XZ plane relative to the sensor position
		verts.emplace_back(m_position.x + p.x,
		                   m_position.y,
		                   m_position.z + p.y);
	}

	gl::pushMatrices();

	auto vboMesh = gl::VboMesh::create(
	    static_cast<uint32_t>(verts.size()),
	    GL_POINTS,
	    { gl::VboMesh::Layout().usage(GL_STREAM_DRAW).attrib(geom::POSITION, 3) });

	vboMesh->bufferAttrib(geom::POSITION, verts);
	gl::pointSize(3);
	gl::draw(vboMesh);
	gl::pointSize(1);
	gl::popMatrices();
}

void act::room::LidarRoomNodeBase::cleanUp()
{
}

void act::room::LidarRoomNodeBase::drawSpecificSettings()
{
}

ci::Json act::room::LidarRoomNodeBase::toParams()
{
	ci::Json json = ci::Json::object();
	json["fixtureName"] = getFixtureName();

	return json;
}

void act::room::LidarRoomNodeBase::fromParams(ci::Json json)
{
	util::setValueFromJson(json, "fixtureName", m_fixtureName);
}
