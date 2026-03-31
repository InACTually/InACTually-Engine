
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "audio/SoundRoomNode.hpp"

#include "..\..\include\room\audio\SoundRoomNode.hpp"

#include "cinder/audio/Context.h"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/Utilities.h"
#include "cinder/audio/Param.h"

act::room::SoundRoomNode::SoundRoomNode(ci::vec3 position, float radius, std::string name)
	: AudioRoomNodeBase("sound", position, ci::vec3(0.0f), radius)
{
	setTriMesh(ci::TriMesh::create(ci::geom::Sphere()));
	setCaption(name);
}

act::room::SoundRoomNode::~SoundRoomNode()
{
}

void act::room::SoundRoomNode::setup()
{
}

void act::room::SoundRoomNode::update()
{
}

void act::room::SoundRoomNode::draw()
{
	ci::gl::ScopedColor color(ci::Color(0.6f, 0.6f, 0.6f));

	enableStatusColor();	

	// Draw the mesh.
	{
		//ci::gl::ScopedColor color(ci::Color::white());

		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		m_mesh->draw();
	}
	
}

void act::room::SoundRoomNode::disconnectExternals()
{
	m_gain->disconnectAllOutputs();
}

void act::room::SoundRoomNode::drawSpecificSettings() 
{
	
}