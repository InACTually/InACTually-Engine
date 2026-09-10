
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

#include "procpch.hpp"
#include "VideoRecorderProcNode.hpp"

act::proc::VideoRecorderProcNode::VideoRecorderProcNode() : ProcNodeBase("VideoRecorder") {
	m_videoSize = glm::ivec2(0, 0);
	m_drawSize = glm::ivec2(m_videoSize.x * 0.25, m_videoSize.y * 0.25);

	m_isRecording = false;
	m_isSaveDialog = false;

	m_videoImageInPort = createImageInput("image", [&](cv::UMat image) { this->onImage(image); });
	m_inputPorts.push_back(m_videoImageInPort);

}

act::proc::VideoRecorderProcNode::~VideoRecorderProcNode() {

}

void act::proc::VideoRecorderProcNode::setup(act::room::RoomManagersRef roomMgrs) {

}

void act::proc::VideoRecorderProcNode::update() {
	if (m_isSaveDialog) {
		m_isSaveDialog = false;
		std::vector<std::string> exts;
		exts.push_back("mp4");
		m_path = ci::app::getSaveFilePath(ci::app::getAssetPath("./../recordings/"), exts).string();
		m_path = m_path.substr(0, m_path.find(".")) + ".mp4";
		saveVideo(m_path);
	}
}

void act::proc::VideoRecorderProcNode::draw() {
	beginNodeDraw();

	if (m_videoSize.x == 0)
		ImGui::Text("Set input for capturing first.");
	else if (ImGui::Button("save mp4 to")) {
		m_isSaveDialog = true;
	}

	if (m_outputVideo.isOpened()) {
		ImGui::SameLine();
		if (!m_isRecording && ImGui::Button("record")) {
			record();
		}
		else if (m_isRecording && ImGui::Button("stop")) {
			stop();
		}
	}

	if (m_videoTexture) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));
		ImGui::Image(m_videoTexture, m_drawSize, glm::vec2(1, 1), glm::vec2(0, 0));
		ci::gl::pushMatrices();
	}

	endNodeDraw();
}



ci::Json act::proc::VideoRecorderProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::VideoRecorderProcNode::fromParams(ci::Json json) {

}

void act::proc::VideoRecorderProcNode::onImage(cv::UMat image)
{
	if (m_videoSize.x == 0)
		m_videoSize = glm::ivec2(image.cols, image.rows);

	if(m_isRecording)
		m_outputVideo << image;
}

void act::proc::VideoRecorderProcNode::saveVideo(std::string path, int fps, bool isColor)
{
	m_path = path;
	cv::Size S = cv::Size(m_videoSize.x, m_videoSize.y);
	try {
		int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
		m_outputVideo = cv::VideoWriter();
		m_outputVideo.open(path, fourcc, fps, S, isColor);
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
}

void act::proc::VideoRecorderProcNode::record()
{
	m_isRecording = true;
}

void act::proc::VideoRecorderProcNode::stop()
{
	m_isRecording = false;
	m_outputVideo.release();
}
