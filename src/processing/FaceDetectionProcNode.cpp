
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
#include "FaceDetectionProcNode.hpp"

#include <numeric>

act::proc::FaceDetectionProcNode::FaceDetectionProcNode() : ProcNodeBase("FaceDetection") {
	m_resizeScale = 0.3f;

	m_show = false;

	m_isFixingFaceSize = true;
	m_fixedFaceSize = 200;

	m_faceAvailHeightThreshold = 300;
	m_faceAvailHistoryMaxSize = 10;
	m_faceAvailHistoryThreshold = 6;

	mFaceHistorySize = 20;

	m_faceImagePort = createImageOutput("biggest face image");
	m_faceAvailablePort = createBoolOutput("face is available");

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });


	std::string path = getAssetPath("3rd/haarcascade_cuda/haarcascade_frontalface_alt.xml").string();
	
	if (path.empty()) {
		CI_LOG_E("file not avaiable.");
		return;
	}
	try {
		mFaceCascade.load(path);
	}
	catch (cv::Exception exc) {
		CI_LOG_EXCEPTION("FaceDetection", exc);
	}

}

act::proc::FaceDetectionProcNode::~FaceDetectionProcNode() {
}

void act::proc::FaceDetectionProcNode::update() {
}

void act::proc::FaceDetectionProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("show", &m_show);

	if (m_show && m_texture) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));
		ci::vec2 texSize = Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(ivec2(0, 0), m_drawSize), true).getSize();

		ImGui::Image(m_texture, texSize, vec2(1, 1), vec2(0, 0));

		//displaySize = ivec2(m_texture->getWidth(), m_texture->getHeight());
		//ImGui::Image(m_texture, displaySize, vec2(1, 1), vec2(0, 0));
		gl::pushMatrices();
	}

	ImGui::SetNextItemWidth(600);
	preventDrag(ImGui::SliderFloat("resize", &m_resizeScale, 0.1f, 1.2f));

	ImGui::SetNextItemWidth(600);
	ImGui::Checkbox("resize faces", &m_isFixingFaceSize);

	ImGui::SetNextItemWidth(600);
	ImGui::InputInt("min height for availbility", &m_faceAvailHeightThreshold);

	if (m_isFixingFaceSize) {
		//ImGui::SameLine();
		ImGui::SetNextItemWidth(600);
		if (ImGui::InputInt("face size", &m_fixedFaceSize)) {
			m_fixedFaceSize = std::clamp(m_fixedFaceSize, 100, 4000);
		}
	}
	
	endNodeDraw();
}

void act::proc::FaceDetectionProcNode::onMat(cv::UMat event) {
	// clear out the previously detected faces
	mFaces.clear();

	
	cv::UMat mat;
	cv::resize(event, mat, cv::Size(event.cols * m_resizeScale, event.rows * m_resizeScale));
	float calcScale = 1.0f / m_resizeScale;
	// detect the faces and iterate them, appending them to m_faces
	std::vector<cv::Rect> faces;
	mFaceCascade.detectMultiScale(mat, faces);
	float faceArea = 0.0f;
	float faceHeight = 0.0f;
	cv::Rect biggestFace;
	for (std::vector<cv::Rect>::const_iterator faceIter = faces.begin(); faceIter != faces.end(); ++faceIter) {
		Rectf faceRect(fromOcv(*faceIter));
		faceRect *= calcScale;
		faceRect.x1 -= faceRect.getWidth() * 0.02f;
		faceRect.x2 += faceRect.getWidth() * 0.02f;
		faceRect.y1 -= faceRect.getHeight() * 0.06f;
		faceRect.y2 += faceRect.getHeight() * 0.06f;
		faceRect = util::fitRoi(faceRect, event);

		float area = toOcv(Area(faceRect)).area();
		if(faceArea < area) {
			faceArea = area;
			faceHeight = faceRect.getHeight();
			biggestFace = toOcv(Area(faceRect));
		}
		mFaces.push_back(faceRect);
		cv::rectangle(mat, faceIter->tl(), faceIter->br(), cv::Scalar(util::Design::primaryColor().b*255, util::Design::primaryColor().g * 255, util::Design::primaryColor().r * 255), 5);
	}

	m_faceAvailHistory.push_back(faceHeight > m_faceAvailHeightThreshold);
	if (m_faceAvailHistory.size() >= m_faceAvailHistoryMaxSize) {
		m_faceAvailHistory.pop_front();

		int sum = 0;
		sum = std::accumulate(m_faceAvailHistory.begin(), m_faceAvailHistory.end(), sum);
		if (sum >= m_faceAvailHistoryThreshold) {
			m_faceAvailablePort->send(true);
		}
		else {
			m_faceAvailablePort->send(false);
		}
	}


	if (faceArea > 0.0f) {
		if (m_isFixingFaceSize) {
			auto face = event(biggestFace);
			cv::UMat mat;
			cv::resize(face, mat, cv::Size(m_fixedFaceSize*0.94f, m_fixedFaceSize));
			m_faceImagePort->send(mat);
		}
		else {
			m_faceImagePort->send(event(biggestFace));
		}
	}
	faces.resize(0);
	faces.clear();
	mFacesHistory.push_back(mFaces);
	if (mFacesHistory.size() >= mFaceHistorySize) {
		mFacesHistory.pop_front();
	}
	if (m_show) {
		m_texture = gl::Texture2d::create(fromOcv(mat));
	}
}

ci::Json act::proc::FaceDetectionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["resizeScale"]					= m_resizeScale;
	json["faceAvailHeightThreshold"]	= m_faceAvailHeightThreshold;
	return json;
}

void act::proc::FaceDetectionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "resizeScale", m_resizeScale);
	util::setValueFromJson(json, "faceAvailHeightThreshold", m_faceAvailHeightThreshold);
}
