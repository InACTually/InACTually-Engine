/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "DepthDetector.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::comp::DepthDetector::DepthDetector()
	: DetectorBase("DepthDetector")
{
	initNetwork();
}

act::comp::DepthDetector::DepthDetector(room::CameraRoomNodeRef camera)
	: DetectorBase("DepthDetector", camera)
{
	initNetwork();
}


act::comp::DepthDetector::~DepthDetector()
{
}


ci::Json act::comp::DepthDetector::toJson() {
	auto json = ci::Json::object();
	json["inputWidth"] = m_blobSize.width;
	json["inputHeight"] = m_blobSize.height;
	json["useTinyModel"] = m_isUsingTiny;

	return json;
}

void act::comp::DepthDetector::fromJson(ci::Json json) {
	int inputWidth = m_blobSize.width;
	int inputHeight = m_blobSize.height;
	bool useTinyModel = m_isUsingTiny;

	util::setValueFromJson(json, "inputWidth", inputWidth);
	util::setValueFromJson(json, "inputHeight", inputHeight);
	util::setValueFromJson(json, "useTinyModel", useTinyModel);

	setInputSize(cv::Size(inputWidth, inputHeight));

	if (m_isUsingTiny != useTinyModel) {
		m_isUsingTiny = useTinyModel;
		initNetwork();
	}
}

void act::comp::DepthDetector::setInputSize(cv::Size inputSize)
{
	m_blobSize.width = std::max(16, inputSize.width);
	m_blobSize.height = std::max(16, inputSize.height);
}

void act::comp::DepthDetector::initNetwork()
{
	auto modelFile = m_isUsingTiny
		? ci::app::getAssetPath("3rd/depth-anything/depth_anything_v2_vits_dynamic.onnx")
		: ci::app::getAssetPath("3rd/depth-anything/depth_anything_v2_vitb_dynamic.onnx");

	try {
		m_net = cv::dnn::readNetFromONNX(modelFile.string());
		m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);

		if (cv::ocl::useOpenCL()) {
			m_net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
			CI_LOG_I("DepthDetector: using OpenCL target");
		}
		else {
			m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
			CI_LOG_W("DepthDetector: OpenCL not available, fallback to CPU");
		}

		m_isInitialized = true;
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
}

void act::comp::DepthDetector::detect() {
	m_currentImage.copyTo(m_colorImage);
	if (m_colorImage.empty() || m_net.empty())
		return;

	cv::Mat blob;
	cv::Mat depth_mat;

	try {
		cv::UMat image;
		cv::resize(m_colorImage, image, m_blobSize);
		cv::flip(image, image, 1);
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
		image.convertTo(image, CV_32FC3, 1.0f / 255.0f);
		cv::subtract(image, cv::Scalar(mean[0], mean[1], mean[2]), image);
		cv::divide(image, cv::Scalar(std[0], std[1], std[2]), image);

		blob = cv::dnn::blobFromImage(image, 1.0, m_blobSize, cv::Scalar(), false, false, CV_32F);

		m_net.setInput(blob);
		cv::Mat output = m_net.forward();

		if (output.dims == 4 && output.size[0] == 1 && output.size[1] == 1) {
			depth_mat = cv::Mat(output.size[2], output.size[3], CV_32FC1, output.ptr<float>()).clone();
		}
		else if (output.dims == 3 && output.size[0] == 1) {
			depth_mat = cv::Mat(output.size[1], output.size[2], CV_32FC1, output.ptr<float>()).clone();
		}
		else if (output.dims == 2) {
			depth_mat = output.clone();
		}
		else {
			CI_LOG_E("Unexpected depth output shape");
			return;
		}
	}
	catch (cv::Exception exc) {
		CI_LOG_E("Failed to predict depth: " << exc.what());
		return;
	}

	try {
		m_currentCandidates.resize(0);
		m_currentCandidates.push_back(depth_mat.getUMat(cv::ACCESS_READ));
		cv::resize(m_colorImage, m_colorImage, depth_mat.size());
		m_currentCandidates.push_back(m_colorImage);

		cv::UMat colormap;
		cv::UMat normalizedDepth;
		cv::normalize(depth_mat, normalizedDepth, 0, 255, cv::NORM_MINMAX, CV_8U);
		cv::applyColorMap(normalizedDepth, colormap, cv::COLORMAP_INFERNO);

		cv::resize(colormap, colormap, m_currentImage.size());
		colormap.copyTo(m_feedbackImage);

		m_areNewCandidatesAvailable = true;
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
}