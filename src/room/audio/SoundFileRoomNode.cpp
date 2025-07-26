
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "audio/SoundFileRoomNode.hpp"

#include "..\..\include\room\audio\SoundFileRoomNode.hpp"

#include "cinder/audio/Context.h"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/Utilities.h"
#include "cinder/audio/Param.h"

act::room::SoundFileRoomNode::SoundFileRoomNode(ci::vec3 position, std::filesystem::path path, float radius, std::string name)
	: SoundRoomNode(position, radius, name)
{
	setTriMesh(ci::TriMesh::create(ci::geom::Sphere()));

	//m_gen = ci::audio::Context::master()->makeNode(new ci::audio::GenSineNode(440, ci::audio::Node::Format().autoEnable()));
	//m_gen >> m_gain;
	m_bufferPlayerNode = ci::audio::Context::master()->makeNode(new ci::audio::BufferPlayerNode());

	if(path.string().find(":") == std::string::npos)
		path = ci::app::getAssetPath(path);
	
	loadFile(path);

	// m_bufferPlayerNode >> m_gain;

	m_isLooping = false;
	m_bufferPlayerNode->setLoopEnabled(m_isLooping);

	m_bufferPlayerNode->enable();

	m_targetVolume = m_volume;
	
	//m_bufferPlayerNode->start();
}

act::room::SoundFileRoomNode::~SoundFileRoomNode()
{
	if (m_stretcherNode)
		m_stretcherNode->pause();
}

void act::room::SoundFileRoomNode::setup()
{
}

void act::room::SoundFileRoomNode::update()
{
	if (m_isLoadingFile) {
		auto path = ci::app::getOpenFilePath();
		loadFile(path);
	}
	

	if(m_bufferPlayerNode->isEof() || getPlayPosition() >= 0.99f) {
		m_isFading = false;
		if (!m_isLooping) {
			stop();
		}
		else {
			setVolume(m_targetVolume, m_fadeInPosition * getSeconds());
		}
		finishedPlayingFn();
	} 

	if (!m_isPlaying)
		return;

	if (!m_isFading && getPlayPosition() > m_fadeOutPosition) {
		m_isFading = true;
		m_targetVolume = getVolume();
		setVolume(0.0f, (1.0f - m_fadeOutPosition) * getSeconds());
	}
}

void act::room::SoundFileRoomNode::draw()
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

void act::room::SoundFileRoomNode::disconnectExternals()
{
	m_gain->disconnectAllOutputs();
	m_bufferPlayerNode >> m_gain;
}

void act::room::SoundFileRoomNode::play()
{
	if (m_isPlaying) {
		stop();
		setVolume(m_targetVolume, 0.0f);
	}
	else {
		m_targetVolume = m_volume;
	}
		
	setVolume(0.0f, 0.0f);
	rampVolume(m_targetVolume, m_fadeInPosition * getSeconds());
	//m_bufferPlayerNode->start();
	if (m_stretcherNode)
		m_stretcherNode->play();
	m_isPlaying = true;
}

void act::room::SoundFileRoomNode::stop()
{
	m_isFading = false;
	// m_bufferPlayerNode->stop();
	if(m_stretcherNode)
		m_stretcherNode->pause();
	setVolume(m_targetVolume);
	m_isPlaying = false;
}

void act::room::SoundFileRoomNode::loop(bool isLooping)
{ 
	m_isLooping = isLooping;
	m_bufferPlayerNode->setLoopEnabled(m_isLooping);
}

void act::room::SoundFileRoomNode::loadFile(fs::path path)
{
	try {
		auto source = ci::audio::load(ci::loadFile(path), ci::audio::Context::master()->getSampleRate());
		
		ci::audio::BufferRef buffer = source->loadBuffer();
		m_bufferPlayerNode->setBuffer(buffer);

		m_gain->disconnectAll();

		auto ctx = audio::Context::master();
		m_stretcherNode = ctx->makeNode(new aio::TimeStretchingNode(source, 120));
		m_stretcherNode >> m_gain >> ctx->getOutput();
	}
	catch (...) {
	}
	m_isLoadingFile = false;
	finishedLoadingFn();
}

void act::room::SoundFileRoomNode::drawSpecificSettings()
{
	if (ImGui::SmallButton("play")) {
		play();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("stop")) {
		stop();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("load")) {
		m_isLoadingFile = true;
	}
	
	if (ImGui::Checkbox("loop", &m_isLooping)) {
		loop(m_isLooping);
	}

}

void act::room::SoundFileRoomNode::setSpeed(float speed) 
{
	m_speed = speed;

	if (!m_isStretching) {
		m_isStretching = true;
		//init();
	}
	if(m_stretcherNode)
		m_stretcherNode->setPlaybackSpeed(m_speed);
}