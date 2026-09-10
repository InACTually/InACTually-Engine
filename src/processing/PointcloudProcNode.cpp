
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021â€“2025 Lars Engeln, Fabian TÃ¶pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "PointcloudProcNode.hpp"


act::room::Pointcloud globalPointcloud;


act::proc::PointcloudProcNode::PointcloudProcNode() : ProcNodeBase("Pointcloud") {
	m_drawSize = glm::ivec2(600, 400);

	m_colorImageInPort = createImageInput("colorImage", [&](cv::UMat colorImage) {
		m_colorImageCache = colorImage;
	});
	m_depthImageInPort = createImageInput("depthImage", [&](cv::UMat depthImage) {
		m_depthImageCache = depthImage;
		createPointcloud(m_depthImageCache, m_colorImageCache);
	});
	m_fovInPort = createVec2Input("FOV", [&](glm::vec2 fov) {
		m_fov = fov;
	});

	m_pointcloudOutPort = OutputPort<act::room::Pointcloud>::create(PT_POINTCLOUD, "pointcloud");
	m_outputPorts.push_back(m_pointcloudOutPort);
}

act::proc::PointcloudProcNode::~PointcloudProcNode() {

}

void act::proc::PointcloudProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	//m_pointcloudRoomNode = roomMgrs->kinectMgr->createPointcloudRoomNode();
}

void act::proc::PointcloudProcNode::update() {

}

void act::proc::PointcloudProcNode::draw() {
	beginNodeDraw();

	if (ImGui::Checkbox("Scan whole room", &m_scanWholeRoom)) {
	}
	ImGui::PushItemWidth(100);
	ImGui::DragInt("Threshold", &m_threshold, 1, 1, 100);
	ImGui::DragFloat("Voxel Size", &m_boxSize, 0.01, 0.1, 10);

	endNodeDraw();
}

void act::proc::PointcloudProcNode::createPointcloud(cv::UMat depthImage, cv::UMat colorImage)
{
	bool calcPointCloud = true;

	if (depthImage.empty()) return;

	cv::Mat depth = depthImage.getMat(cv::ACCESS_FAST);
	cv::Mat color = colorImage.getMat(cv::ACCESS_FAST);

	if (depth.type() == CV_8UC1) {
		int panic = 0;
	}

	float fx = 5.8f; //_intrinsics(0, 0);
	float fy = 5.8f; //_intrinsics(1, 1);
	float cy = depth.rows * 0.5f; //_intrinsics(0, 2);
	float cx = depth.cols * 0.5f; //_intrinsics(1, 2);

	float factor = 38.0f;

	act::room::Pointcloud pointcloudPointcloud;
	act::room::Pointcloud pointcloudFiltered;

	try {
#pragma omp parallel for
		for (int v = 0; v < depth.rows; v++)
		{
			for (int u = 0; u < depth.cols; u++)
			{

				auto depthColor = depth.at<uchar>(v, u);
				float Z = static_cast<float>(depthColor) / 255;
				//if (Z < 0) {Z = Z + 1;}

				if (Z > 0.12) {

					/*p.z = -(Z * factor) + 30;
					p.x = -((u - cx) * Z / fx);
					p.y = -((v - cy) * Z / fy) + 10;

					cv::Vec4b bgr = color.at<cv::Vec4b>(v, u);

					unsigned char alpha = bgr[3] / 255.0f;

					p.r = static_cast<float>(bgr[2]) * alpha;
					p.g = static_cast<float>(bgr[1]) * alpha;
					p.b = static_cast<float>(bgr[0]) * alpha;
					*/

					//pointcloud.push_back(p);

				}
			}
		}
	}
	catch (cv::Exception exc) {
		CI_LOG_E("PCL error:" << exc.what());
	}

}
		


ci::Json act::proc::PointcloudProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["threshold"]		= m_threshold;
	json["voxelSize"]		= m_boxSize;
	json["scanWholeRoom"]	= m_scanWholeRoom;
	return json;
}

void act::proc::PointcloudProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "threshold", m_threshold);
	util::setValueFromJson(json, "voxelSize", m_boxSize);
	util::setValueFromJson(json, "scanWholeRoom", m_scanWholeRoom);
}