/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021,2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "VideoPlayerProcNode.hpp"

act::proc::VideoPlayerProcNode::VideoPlayerProcNode() : ProcNodeBase("VideoPlayer") {
	m_videoSize = ivec2(1920, 1080);
	m_drawSize = ivec2(m_videoSize.x * 0.25, m_videoSize.y * 0.25);

	m_isOpenDialog = false;
	m_isPlaying = false;
	m_isLooping = true;

	auto trigger = createBoolInput("fire", [&](bool event) { onTrigger(event); });
	createNumberInput("seek", [&](number playPos) { seek(playPos); });

	m_videoImageOutPort = createImageOutput("videoImage");
}

act::proc::VideoPlayerProcNode::~VideoPlayerProcNode() {

}

void act::proc::VideoPlayerProcNode::setup(act::room::RoomManagers roomMgrs) {

}

void act::proc::VideoPlayerProcNode::update() {
	if (m_isOpenDialog) {
		m_isOpenDialog = false;
		m_path = ci::app::getOpenFilePath().string();
		loadVideo(m_path);
	}

	if (m_isPlaying && m_inputVideo && m_inputVideo->getTexture()) {
		cv::UMat frame;
		auto frameTexture = m_inputVideo->getTexture();
		auto src = frameTexture->createSource();
		frame = toOcv(src).getUMat(cv::ACCESS_FAST);

		if (m_inputVideo->isDone()) {
			if (!m_isLooping) {
				m_isPlaying = false;
			}
			else {
				m_inputVideo->seekToStart();
			}
		}
		if (!frame.empty()) {
			m_videoImageOutPort->send(frame);
		}
	}
}

void act::proc::VideoPlayerProcNode::draw() {
	beginNodeDraw();

	if (ImGui::Button("load")) {
		m_isOpenDialog = true;
	}
	if (m_inputVideo) {
		ImGui::SameLine();
		if (!m_isPlaying && ImGui::Button("play")) {
			onTrigger(true);
		}
		else if (m_isPlaying && ImGui::Button("stop")) {
			onTrigger(false);
		}
	}

	ImGui::Checkbox("resume video", &m_isResuming);
	ImGui::SameLine();
	ImGui::Checkbox("loop video", &m_isLooping);

	ImGui::Text(m_path.c_str());

	if (m_videoTexture) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));
		ImGui::Image(m_videoTexture, m_drawSize, vec2(1, 1), vec2(0, 0));
		gl::pushMatrices();
	}

	endNodeDraw();
}

void act::proc::VideoPlayerProcNode::onTrigger(bool event)
{
	if (!event)
		return;

	if (!m_isPlaying) {
		m_isPlaying = true;
	}
	else if (m_isPlaying) {
		if(!m_isResuming)
			m_inputVideo->seekToStart();
	}
}

void act::proc::VideoPlayerProcNode::seek(number playPosition)
{
	playPosition = std::clamp(playPosition, 0.0f, 1.0f);
	m_inputVideo->seekToTime(m_inputVideo->getDuration() * playPosition);
}

ci::Json act::proc::VideoPlayerProcNode::toParams() {
	ci::Json json = ci::Json::object();

	json["path"] = m_path;
	json["resuming"] = m_isResuming;
	json["looping"] = m_isLooping;

	return json;
}

void act::proc::VideoPlayerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "path", m_path);
	util::setValueFromJson(json, "resuming", m_isResuming);
	util::setValueFromJson(json, "looping", m_isLooping);

	loadVideo(m_path);
}

void act::proc::VideoPlayerProcNode::loadVideo(std::string path)
{
	if (path == "")
		return;

	m_path = path;
	fs::path p(m_path);
	if (p.is_relative())
		p = ci::app::getAssetPath(p);
	try {
		// load up the movie, set it to loop, and begin playing
		m_inputVideo = qtime::MovieGl::create(p.string());
		//mMovie->setLoop();
		m_inputVideo->play();
	}
	catch (ci::Exception& exc) {
		std::stringstream strstr;
		strstr << "Failed to load the movie from path: " << m_path << ", what: " << exc.what();
		CI_LOG_E(strstr.str());
		m_inputVideo.reset();
	}
}
