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
	m_isAdding = false;
	m_isPlaying = false;
	m_isLooping = true;

	m_currentVideoIndex = 0;

	m_isFading = false;
	m_fadeAt = 1.0f;

	auto trigger = createBoolInput("fire", [&](bool event) { onTrigger(event); });
	createNumberInput("seek", [&](number playPos) { seek(playPos); });
	createNumberInput("fade to video", [&](number videoIndex) { fadeToVideoIndex(videoIndex); });

	m_videoImageOutPort = createImageOutput("videoImage");
}

act::proc::VideoPlayerProcNode::~VideoPlayerProcNode() {

}

void act::proc::VideoPlayerProcNode::setup(act::room::RoomManagers roomMgrs) {

}

void act::proc::VideoPlayerProcNode::update() {
	if (m_isOpenDialog) {
		m_isOpenDialog = false;
		fs::path path = ci::app::getOpenFilePath().string();

		if (path.empty())
			return;

		if (m_isAdding) {
			m_isAdding = false;
			m_paths.push_back(path);
		}
		else {
			m_paths.resize(0);
			m_paths.push_back(path);
			m_currentVideoIndex = 0;
			loadVideo(getCurrentPath());
		}
	}

	if (m_isPlaying && m_video && m_video->getTexture()) {
		cv::UMat frame;
		auto frameTexture = m_video->getTexture();
		auto src = frameTexture->createSource();
		frame = toOcv(src).getUMat(cv::ACCESS_FAST);

		if (m_fadeAt.isComplete())
			m_isFading = false;
		if (m_isFading && m_videoFadeFrom && m_videoFadeFrom->getTexture()) {
			cv::UMat fromFrame;
			auto fromFrameTexture = m_videoFadeFrom->getTexture();
			auto fromSrc = fromFrameTexture->createSource();
			fromFrame = toOcv(fromSrc).getUMat(cv::ACCESS_FAST);

			cv::addWeighted(frame, m_fadeAt, fromFrame, 1.0f - m_fadeAt, 0.0f, frame);
		}

		if (m_video->isDone()) {
			if (!m_isLooping) {
				m_isPlaying = false;
			}
			else {
				m_video->seekToStart();
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
	if (m_video) {
		ImGui::SameLine();
		if (ImGui::Button("add")) {
			m_isAdding = true;
			m_isOpenDialog = true;
		}

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


	for(auto&& path : m_paths)
		ImGui::Text(path.string().c_str());

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

	if (!m_isPlaying && m_video) {
		m_isPlaying = true;
	}
	else if (m_isPlaying) {
		if(!m_isResuming)
			m_video->seekToStart();
	}
}

void act::proc::VideoPlayerProcNode::seek(number playPosition)
{
	playPosition = std::clamp(playPosition, 0.0f, 1.0f);
	m_video->seekToTime(m_video->getDuration() * playPosition);
}

ci::Json act::proc::VideoPlayerProcNode::toParams() {
	ci::Json json = ci::Json::object();

	json["paths"] = m_paths;
	json["resuming"] = m_isResuming;
	json["looping"] = m_isLooping;

	return json;
}

void act::proc::VideoPlayerProcNode::fromParams(ci::Json json) {
	if (json.contains("paths")) {
		m_paths.clear();
		for (auto& path : json["paths"])
			m_paths.push_back(path.get<std::string>());
	}
	util::setValueFromJson(json, "resuming", m_isResuming);
	util::setValueFromJson(json, "looping", m_isLooping);

	loadVideo(getCurrentPath());
}

void act::proc::VideoPlayerProcNode::loadVideo(fs::path path)
{
	if (path.empty())
		return;

	fs::path p(path);
	if (p.is_relative())
		p = ci::app::getAssetPath(p);
	try {
		// load up the movie, set it to loop, and begin playing
		m_video = qtime::MovieGl::create(p.string());
		//mMovie->setLoop();
		m_video->play();
	}
	catch (ci::Exception& exc) {
		std::stringstream strstr;
		strstr << "Failed to load the movie from path: " << path << ", what: " << exc.what();
		CI_LOG_E(strstr.str());
		m_video.reset();
	}
}

bool act::proc::VideoPlayerProcNode::fadeToVideoIndex(int index)
{
	if (index >= m_paths.size())
		return false;

	m_videoFadeFrom = m_video;
	loadVideo(m_paths[index]);

	if (!m_video) {
		m_video = m_videoFadeFrom;
		return false;
	}

	m_video->seekToTime(m_videoFadeFrom->getCurrentTime());

	m_fadeAt = 0.0f;
	ci::app::timeline().apply(&m_fadeAt, 1.0f, 3, ci::easeInOutSine);
	m_isFading = true;

	return true;
}

fs::path act::proc::VideoPlayerProcNode::getCurrentPath()
{
	if (m_currentVideoIndex >= m_paths.size())
		return fs::path();
	return m_paths[m_currentVideoIndex];
}
