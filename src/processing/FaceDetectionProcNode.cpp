
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "FaceDetectionProcNode.hpp"
#include "CinderOpenCV.h"

#include <numeric>

act::proc::FaceDetectionProcNode::FaceDetectionProcNode() : ProcNodeBase("FaceDetection") {
	m_resizeScale = 0.3f;

	m_show = false;

	m_textureHelper = proc::TextureHelper::create();

	m_isFixingFaceSize = true;
	m_fixedFaceSize = 200;

	m_faceAvailHeightThreshold = 300;
	m_faceAvailHistoryMaxSize = 10;
	m_faceAvailHistoryThreshold = 6;

	m_facesHistorySize = 20;

	m_faceImagePort = createImageOutput("biggest face image");
	m_faceAvailablePort = createBoolOutput("face is available");

	auto image = createImageInput("image", [&](cv::UMat mat) { 
		try { this->onMat(mat); } 
		catch (cv::Exception exc) { CI_LOG_E(exc.what()); }
	});

	auto input_size = cv::Size(320, 320);
	float conf_threshold = 0.6f;
	float nms_threshold = 0.3f;;
	int top_k = 5000;

	std::string path = ci::app::getAssetPath("3rd/models/face_detection_yunet/face_detection_yunet_2026may.onnx").string();
	if (path.empty()) {
		CI_LOG_E("File not avaiable.");
		return;
	}

	try {
		m_model = cv::FaceDetectorYN::create(path, "", input_size, conf_threshold, nms_threshold, top_k, cv::dnn::DNN_BACKEND_OPENCV, cv::dnn::DNN_TARGET_OPENCL);
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

	if (m_show && !m_textureHelper->getTexture().empty()) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));
		glm::vec2 texSize = ci::Rectf(glm::vec2(0, 0), ci::fromOcv(m_textureHelper->getTexture().size())).getCenteredFit(ci::Rectf(glm::ivec2(0, 0), m_drawSize), true).getSize();

		ImGui::Image(m_textureHelper->getTexture().texId(), texSize);

		ci::gl::pushMatrices();
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
	if (event.empty() || event.cols < 100 || event.rows < 100)
		return;

	// clear out the previously detected faces
	m_faces.clear();

	
	cv::UMat mat;
	cv::resize(event, mat, cv::Size(event.cols * m_resizeScale, event.rows * m_resizeScale));
	float calcScale = 1.0f / m_resizeScale;
	// detect the faces and iterate them, appending them to m_faces
	std::vector<cv::Rect> faces;
	
	m_model->setInputSize(mat.size());
	cv::Mat result;
	m_model->detect(mat, result);

	float faceArea = 0.0f;
	float faceHeight = 0.0f;
	cv::Rect biggestFace;
	for (int i = 0; i < result.rows; ++i) {
		int x1 = static_cast<int>(result.at<float>(i, 0));
		int y1 = static_cast<int>(result.at<float>(i, 1));
		int w = static_cast<int>(result.at<float>(i, 2));
		int h = static_cast<int>(result.at<float>(i, 3));
		float conf = result.at<float>(i, 14);

		ci::Rectf faceRect(x1, y1, x1 + w, y1 + h);
		faceRect *= calcScale;
		faceRect.x1 -= faceRect.getWidth() * 0.02f;
		faceRect.x2 += faceRect.getWidth() * 0.02f;
		faceRect.y1 -= faceRect.getHeight() * 0.06f;
		faceRect.y2 += faceRect.getHeight() * 0.06f;
		faceRect = util::fitRoi(faceRect, event);

		float area = ci::toOcv(ci::Area(faceRect)).area();
		if(faceArea < area) {
			faceArea = area;
			faceHeight = faceRect.getHeight();
			biggestFace = ci::toOcv(ci::Area(faceRect));
		}
		m_faces.push_back(faceRect);
		cv::rectangle(mat, cv::Point(x1, y1), cv::Point(x1+w, y1+h), cv::Scalar(util::Design::primaryColor().b * 255, util::Design::primaryColor().g * 255, util::Design::primaryColor().r * 255), 5);
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
			cv::resize(face, face, cv::Size(m_fixedFaceSize*0.94f, m_fixedFaceSize));
			m_faceImagePort->send(face);
		}
		else {
			m_faceImagePort->send(event(biggestFace));
		}
	}
	faces.resize(0);
	faces.clear();
	m_facesHistory.push_back(m_faces);
	if (m_facesHistory.size() >= m_facesHistorySize) {
		m_facesHistory.pop_front();
	}
	if (m_show) {
		m_textureHelper->toTextureAsync(mat);
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
