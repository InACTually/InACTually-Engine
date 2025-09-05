
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Fabian Töpfer - baniaf@uber.space
*/

#include "procpch.hpp"
#include "MediaPipeProcNode.hpp"
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

act::proc::MediaPipeProcNode::MediaPipeProcNode() : ProcNodeBase("MediaPipe") {
	m_drawSize = ivec2(200, 200);
	
	auto image = InputPort<cv::UMat>::create(PT_IMAGE, "image", [&](cv::UMat mat) { this->onMat(mat); });
	m_inputPorts.push_back(image);

	m_outputPort = OutputPort<cv::UMat>::create(PT_IMAGE, "output image");
	 
	m_outputPorts.push_back(m_outputPort);

	m_input = cv::UMat (100, 100, CV_8UC1, cv::Scalar(128));

}

act::proc::MediaPipeProcNode::~MediaPipeProcNode() {
}


void act::proc::MediaPipeProcNode::onMat(cv::UMat event) {
	m_input = event;

}

void act::proc::MediaPipeProcNode::update() {


	//needed to init it here since constructor is somehow called even before a MediaPipeProcNode is created in GUI ???
	if (m_guard == nullptr) {
		m_guard = std::make_unique<py::scoped_interpreter>();
	}
	

	if (m_adaptor == nullptr) {

	 
		m_adaptor = act::comp::PythonAdaptor::create(
			"mediapipe/",
			"mediapipe",
			"MediaPipe_Module",
			"process_frame");
	}


	if (m_input.empty()) {
		return;
	}

	// Convert cv::Mat -> numpy
	py::array_t<uint8_t> np_frame(
		{ m_input.rows, m_input.cols, m_input.channels() },
		{ static_cast<size_t>(m_input.step),
		  static_cast<size_t>(m_input.elemSize()),
		  static_cast<size_t>(1) },
		m_input.getMat(cv::ACCESS_READ).data
	);

	// Call python function with np_frame
	py::object result = m_adaptor->call(py::make_tuple(np_frame));

	if (!result.is_none()) {
		//dict of all outcomes 
		auto result_dict = result.cast<py::dict>();


		if (result_dict.contains("image")) {
			py::array_t<uint8_t> np_out = result_dict["image"].cast<py::array_t<uint8_t>>();
			py::buffer_info buf = np_out.request();

			m_output = cv::Mat(
				buf.shape[0], buf.shape[1], CV_8UC3, buf.ptr
			).getUMat(cv::ACCESS_READ);

			m_outputPort->send(m_output);
		}
	}
}
void act::proc::MediaPipeProcNode::draw() {
	beginNodeDraw();
	
	endNodeDraw();
}

ci::Json act::proc::MediaPipeProcNode::toParams() {
	ci::Json json = ci::Json::object();
 
	return json;
}

void act::proc::MediaPipeProcNode::fromParams(ci::Json json) {
  
}
 
