
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "camera/CameraRoomNode.hpp"
#include "PortType.hpp"

#include "computing/DepthDetector.hpp"


act::room::CameraRoomNode::CameraRoomNode(CameraDeviceRef camera, std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase("camera", position, rotation, radius, replyUID)
{
	m_camera = camera;
	setCaption(name);
	/*if (m_capture) {
		m_name = m_capture->getDevice()->getName();
		m_captureSize = m_capture->getSize();
	}
	else {
		m_name = deviceName;
		m_captureSize = glm::ivec2(1920, 1080);
	}*/
	m_displaySize = glm::ivec2(m_camera->getCaptureSize().x * 0.25, m_camera->getCaptureSize().y * 0.25);

	m_cameraImagePort = proc::ImageOutputPort::create(act::proc::PT_IMAGE, "cameraImage");

	m_cameraPersp = ci::CameraPersp(m_camera->getCaptureSize().x, m_camera->getCaptureSize().y, 70, 0.1f, 5.0f);
	m_cameraPersp.setEyePoint(glm::vec3(0.0f));
	setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	m_cameraPersp.lookAt(glm::vec3(0.0f, 1.0f, 0.0f));

	if (!camera->isCalibrated()) {
		ci::Json iniJson = ci::Json("{\"distCoeffs\":\"[0, 0, 0, 0, 0]\",\"intrinsic\":\"[16, 0, 9, 0, 16, 4, 0, 0, 1]\"}");
		fromParams(iniJson);
	}

	m_pointcloudRoomNode = PointcloudRoomNode::create(glm::vec3(0, 0, 0), 1, getName() + "_Pointcloud");
}



act::room::CameraRoomNode::CameraRoomNode(ci::Capture::DeviceRef deviceRef, std::string deviceName, std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase("camera", position, rotation, radius, replyUID)
{
	m_camera = CameraDevice::create(deviceRef);
	setCaption(name);

	m_cameraImagePort = proc::ImageOutputPort::create(act::proc::PT_IMAGE, "cameraImage");

	m_cameraPersp = ci::CameraPersp(m_camera->getCaptureSize().x, m_camera->getCaptureSize().y, 70, 0.1f, 5.0f);
	m_cameraPersp.setEyePoint(glm::vec3(0.0f));
	setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	m_cameraPersp.lookAt(glm::vec3(0.0f, 0.0f, 1.0f));

	if (!m_camera->isCalibrated()) {
		ci::Json iniJson = ci::Json("{\"distCoeffs\":\"[0, 0, 0, 0, 0]\",\"intrinsic\":\"[16, 0, 9, 0, 16, 4, 0, 0, 1]\"}");
		fromParams(iniJson);
	}

	m_pointcloudRoomNode = PointcloudRoomNode::create(glm::vec3(0, 0, 0), 1, getName() + "_Pointcloud");
}

act::room::CameraRoomNode::~CameraRoomNode()
{
	if (m_camera->hasCapture() && m_camera->isCapturing()) {
		m_camera->~CameraDevice();
		//m_capture.reset(); // ignore stop()=>pGrabber-error, seems to be only in debug ^^
	}
}

void act::room::CameraRoomNode::setup()
{
}

void act::room::CameraRoomNode::update()
{
	if(m_cameraImagePort->getListenerCount() > 0)
		if (m_camera->update()) // if has new frame
			m_cameraImagePort->send(m_camera->getCurrentImage());

	if (m_isDetectingDepth && !m_depthDetector) {
		m_depthDetector = comp::DepthDetector::create(shared_from_this());
	}

	if (m_isDetectingDepth && m_depthDetector && m_depthDetector->hasNewCandidates()) {
		m_depthTexture = m_depthDetector->getFeedbackTexture();
		m_pointcloud = toPointCloud(m_depthDetector->getCandidates()[0], m_depthDetector->getCandidates()[1]);
		m_pointcloudRoomNode->setPointcloud(m_pointcloud);
		m_pointcloudRoomNode->update();
	}
}

void act::room::CameraRoomNode::draw()
{
	ci::gl::ScopedColor color;
	enableStatusColor(); 

	ci::gl::pushMatrices();
	ci::gl::translate(m_position);
	ci::gl::rotate(m_rotation);
	ci::gl::drawCube(glm::vec3(0.0f), glm::vec3(0.1f, 0.075f, 0.075f));

	m_pointcloudRoomNode->draw();
	util::drawCoords();

	if (getIsUnfolded())
		ci::gl::color(util::Design::highlightColor(0.85f));
	else
		ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, 0.6f));
	ci::gl::scale(glm::vec3(0.1, 0.1, 0.1));
	ci::gl::drawFrustum(m_cameraPersp);
	
	ci::gl::popMatrices();
}

void act::room::CameraRoomNode::drawSpecificSettings()
{
	bool flipped = m_camera->isFlipped();
	ImGui::Checkbox("flipped", &flipped);
	m_camera->setIsFlipped(flipped);

	ImGui::Checkbox("detect Markers", &m_isDetectingMarkers);
	ImGui::Checkbox("detect Objects", &m_isDetectingObjects);
	ImGui::Checkbox("detect Depth", &m_isDetectingDepth);

	if (m_depthTexture) {
		ImGui::Image(m_depthTexture, glm::vec2(m_depthTexture->getSize()) * 0.6f);
	}
}

ci::Json act::room::CameraRoomNode::toParams()
{
	ci::Json json = ci::Json::object();
	if (!m_camera->getIntrinsic().empty()) {

		cv::Mat oneRowIntr = m_camera->getIntrinsic().reshape(0, 1);
		std::ostringstream osIntr;
		osIntr << oneRowIntr;
		std::string intrAsStr = osIntr.str();
		json["intrinsic"] = intrAsStr;

		cv::Mat oneRowCoeff = m_camera->getDistCoeffs().reshape(0, 1);
		std::ostringstream osCoeff;
		osCoeff << oneRowCoeff;
		std::string coeffAsStr = osCoeff.str();
		json["distCoeffs"] = coeffAsStr;

		json["flipped"] = m_camera->isFlipped() ? "true" : "false";
		json["width"] = m_camera->getCaptureSize().x;
		json["height"] = m_camera->getCaptureSize().y;

	}
	return json;
}

void act::room::CameraRoomNode::fromParams(ci::Json json)
{
	if (json.contains("flipped")) {
		auto flipped = json["flipped"];

		if (flipped == "true") {
			m_camera->setIsFlipped(true);
		}
	}

	if (json.contains("intrinsic") && json.contains("distCoeffs")) {
		std::string j_intrinsics = json["intrinsic"];
		std::string j_distCoeffs = json["distCoeffs"];

		cv::Mat intrinsics = cv::Mat(3, 3, CV_32FC1);
		cv::Mat distCoeffs = cv::Mat(1, 5, CV_32FC1);

		j_intrinsics.erase(0, 1);
		j_intrinsics.erase(j_intrinsics.length() - 1, j_intrinsics.length());
		j_distCoeffs.erase(0, 1);
		j_distCoeffs.erase(j_distCoeffs.length() - 1, j_distCoeffs.length());

		double intr_data[9];
		std::string delimiter = ", ";
		std::string token;
		size_t pos = 0;
		int i = 0;
		while ((pos = j_intrinsics.find(delimiter)) != std::string::npos) {
			token = j_intrinsics.substr(0, pos);
			intr_data[i] = std::stof(token);
			j_intrinsics.erase(0, pos + delimiter.length());
			i++;
		}
		intr_data[i] = std::stof(j_intrinsics);

		double distCoeffs_data[5];
		pos = 0;
		i = 0;
		while ((pos = j_distCoeffs.find(delimiter)) != std::string::npos) {
			token = j_distCoeffs.substr(0, pos);
			distCoeffs_data[i] = std::stof(token);
			j_distCoeffs.erase(0, pos + delimiter.length());
			i++;
		}
		distCoeffs_data[i] = std::stof(j_distCoeffs);

		i = 0;
		for (int r = 0; r < intrinsics.rows; r++)
			for (int c = 0; c < intrinsics.cols; c++)
				intrinsics.at<float>(r, c) = intr_data[i++];
			

		i = 0;
		for (int r = 0; r < distCoeffs.rows; r++)
			for (int c = 0; c < distCoeffs.cols; c++)
				distCoeffs.at<float>(r, c) = distCoeffs_data[i++];

		m_camera->setCalibration(intrinsics, distCoeffs);
	}
}

act::room::Pointcloud act::room::CameraRoomNode::toPointCloud(cv::UMat depth, cv::UMat color)
{
	Pointcloud pointcloud;// = std::make_shared<>();

	if (!m_camera->isCalibrated())
		return pointcloud;

	int width = depth.cols;
	int height = depth.rows;

	cv::Mat d = depth.getMat(cv::ACCESS_READ);
	cv::Mat c = color.getMat(cv::ACCESS_READ);

	if (width == 0 || height == 0) {
		return pointcloud;
	}

	pointcloud->resize(static_cast<size_t>(height) * width);
	//%%% pointcloud->width = width;
	//%%% pointcloud->height = height;

	long i{ 0 };
	float min = 10000;
	float max = 0;

	float fx = m_camera->getIntrinsic().at<float>(cv::Point(0, 0))*0.1f; // Horizontal focal length
	float fy = m_camera->getIntrinsic().at<float>(cv::Point(1, 1))*0.1f; // Vertcal focal length
	float cx = ((float)depth.cols - 1.f) / 2.f;  // Center x
	float cy = ((float)depth.rows - 1.f) / 2.f; // Center y

	// Get inverse focal length for calculations below
	float fx_inv = 1.0f / fx;
	float fy_inv = 1.0f / fy;

	int x = 0;
	int y = 0;

	for (auto& point : *pointcloud)
	{
		point.z = d.at<float>(i);  
		point.x = (static_cast<float> (x) - cx)* fx_inv*point.z;
		point.y = (static_cast<float> (y) - cy)* fy_inv*point.z;

		if (point.z < min)
			min = point.z;
		if (point.z > max)
			max = point.z;

		point.b = c.at<cv::Vec3b>(i)[0];
		point.g = c.at<cv::Vec3b>(i)[1];
		point.r = c.at<cv::Vec3b>(i)[2];

		i = i + 1;

		x++;
		if (x >= d.cols) {
			y++;
			x = 0;
		}
	}

	//TODO 

	return pointcloud;
}

