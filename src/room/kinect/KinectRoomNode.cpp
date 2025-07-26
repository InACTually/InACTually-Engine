
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "kinect/KinectRoomNode.hpp"

act::room::KinectRoomNode::KinectRoomNode(KinectDeviceRef kinect, std::string deviceName, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase(deviceName, position, rotation, radius, replyUID)
{
	m_isProvidingPointCloud = true;
	m_kinect = kinect;
	setPosition(position);
	setRotation(rotation);

	m_deviceName = deviceName;
	setCaption("Kinect - " + name);

	if (m_kinect)
	{
		m_kinect->setNodeUID(getUID());
		
		if (m_kinect->isDummy()) {
			m_offsetPosition = position;
			m_offsetRotation = rotation;
			m_offsetRotation.y += -glm::pi<float>() * 0.5f;
		}
	}
	else
	{
	}

	m_kinectImageOutPort	= act::proc::OutputPort<cv::UMat>::create(act::proc::PT_IMAGE, "kinectImage");
	m_kinectDepthOutPort	= act::proc::OutputPort<cv::UMat>::create(act::proc::PT_IMAGE, "kinectDepth");
	m_kinectIROutPort		= act::proc::OutputPort<cv::UMat>::create(act::proc::PT_IMAGE, "kinectIR");
	m_kinectBIMOutPort		= act::proc::OutputPort<cv::UMat>::create(act::proc::PT_IMAGE, "kinectBIM");

	m_kinectBodiesOutPort	= act::proc::OutputPort<room::BodyRefList>::create(act::proc::PT_BODYLIST, "bodies");

	m_pointcloudRoomNode	= PointcloudRoomNode::create(vec3(0,0,0), 1, getName()+"_Pointcloud");
}

act::room::KinectRoomNode::~KinectRoomNode()
{
}

void act::room::KinectRoomNode::setup()
{
}

void act::room::KinectRoomNode::update()
{
	m_cameraPersp = ci::CameraPersp(1920, 1080, 90, 0.5f, 3.86f);
	m_cameraPersp.setEyePoint(vec3(0, 0, 0));
	m_cameraPersp.lookAt(vec3(0, 0, 1));

	if (m_kinect != nullptr) {

		m_captureBodies.clear();

		m_kinect->update();

		if (m_kinect) {
			if (m_kinect->m_isColorFrameAvailable) {
				m_captureImage = m_kinect->getColorFrame();
				m_kinect->m_isColorFrameAvailable = false;
			}
			if (m_kinect->m_isDepthFrameAvailable) {
				m_captureDepth = m_kinect->getDepthFrame();
				m_kinect->m_isDepthFrameAvailable = false;
			}
			if (m_kinect->m_isInfraRedFrameAvailable) {
				m_captureIR = m_kinect->getInfraRedFrame();
				m_kinect->m_isInfraRedFrameAvailable = false;
			}
			if (m_kinect->m_isBodyIndexFrameAvailable) {
				m_captureBIM = m_kinect->getBIMFrame();
				m_kinect->m_isBodyIndexFrameAvailable = false;
			}
			//if (m_kinect->m_areBodiesAvailable) {
				m_captureBodies = m_kinect->getBodyMap();
				m_kinect->m_areBodiesAvailable = false;
			//}
			if (m_kinect->m_isPointCloudAvailable) {
				//m_pointcloud = m_kinect->getPointCloud();
				m_kinect->m_isPointCloudAvailable = false;
				m_pointcloudRoomNode->setPointcloud(m_pointcloud);
				m_pointcloudRoomNode->update();
			}

			if (m_captureImage.rows > 0 && !cameraInit) {
				initCameraPersp(m_captureImage.cols, m_captureImage.rows);
				cameraInit = true;
			}
		}

		m_kinectImageOutPort->send(m_captureImage);
		m_kinectDepthOutPort->send(m_captureDepth);
		m_kinectIROutPort->send(m_captureIR);
		m_kinectBIMOutPort->send(m_captureBIM);

		m_repositionedBodies = repositionBodies(m_captureBodies);
		m_kinect->setRepositionedBodies(m_repositionedBodies);
		m_kinectBodiesOutPort->send(kh::toGenericBody(m_repositionedBodies));
	}

}

void act::room::KinectRoomNode::draw()
{
	enableStatusColor();

	gl::pushMatrices();
	gl::translate(m_position);
	gl::rotate(m_rotation);
	gl::drawCube(ci::vec3(0.0f), ci::vec3(0.3f, 0.15f, 0.15f));

	m_pointcloudRoomNode->draw();

	gl::color(ColorA(1.0f, 1.0f, 1.0f, 0.6f));

	if (getIsUnfolded())
		gl::color(util::Design::highlightColor(0.85f));
	else
		gl::color(ColorA(1.0f, 1.0f, 1.0f, 0.6f));

	if (!m_isConnected)
		if (getIsUnfolded())
			gl::color(util::Design::errorColor(0.85f));
		else
			gl::color(util::Design::errorColor(0.45f));



	gl::drawFrustum(m_cameraPersp);
	gl::popMatrices();
}

void act::room::KinectRoomNode::drawSpecificSettings()
{
	if (!m_kinect)
		return;

	if (ImGui::Checkbox("provide PointCloud", &m_isProvidingPointCloud))
		m_kinect->setIsProvidingPointCloud(m_isProvidingPointCloud);

}

ci::Json act::room::KinectRoomNode::toParams()
{
	ci::Json json = ci::Json::object();

	json["isProvidingPointCloud"] = m_isProvidingPointCloud;

	return json;
}

void act::room::KinectRoomNode::fromParams(ci::Json json)
{
	util::setValueFromJson(json, "isProvidingPointCloud", m_isProvidingPointCloud);
	if (m_kinect)
		m_kinect->setIsProvidingPointCloud(m_isProvidingPointCloud);
}

void act::room::KinectRoomNode::initCameraPersp(int width, int height)
{
	m_captureSize = ivec2(width, height);
	m_displaySize = ivec2(m_captureSize.x * 0.25, m_captureSize.y * 0.25);
}

std::map<uint32_t, k4abt_skeleton_t> act::room::KinectRoomNode::repositionBodies(std::map<uint32_t, k4abt_skeleton_t> bodyMap)
{
	std::map<uint32_t, k4abt_skeleton_t> retMap;
#ifdef WITHKINECT
	for (auto&& body : bodyMap)
	{
		for (int i = 0; i < 32; i++)
		{
			k4a_float3_t::_xyz kinPos = body.second.joints[i].position.xyz;
			vec3 newPos = calcRoomPos(vec3(kinPos.x, kinPos.y, kinPos.z));

			body.second.joints[i].position.xyz.x = newPos.x;
			body.second.joints[i].position.xyz.y = newPos.y;
			body.second.joints[i].position.xyz.z = newPos.z;
		}

		retMap[body.first] = body.second;
	}
#endif
	return retMap;
}

vec3 act::room::KinectRoomNode::calcRoomPos(vec3 pos)
{
	//TODO: tilt 1.3 degrees if using WFOV depth mode
	//https://docs.microsoft.com/en-us/azure/kinect-dk/coordinate-systems

	//invert y-axis
	//https://docs.microsoft.com/en-us/azure/kinect-dk/media/concepts/concepts-coordinate-systems/coordinate-systems-camera-features.png
	if (!m_kinect->isDummy()) {
		pos.y = pos.y * (-1.0f);
		pos.x = pos.x * (-1.0f);

		pos *= (0.001f); //mm to m
	}

	pos = glm::rotateY(pos, m_rotation.y - m_offsetRotation.y);
	pos += (m_position - m_offsetPosition);

	return pos;
}

act::room::Pointcloud act::room::KinectRoomNode::getWorldSpacePointCloud() {

	/*if (m_isProvidingPointCloud) {
		act::room::Pointcloud  transformedPointCloud = pcl::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
		pcl::copyPointCloud(*m_pointcloud, *transformedPointCloud);

		auto translation = getPosition();
		auto orientation = getOrientation();

		Eigen::Matrix<float,3,1> eigenTranslation(translation.x, translation.y, translation.z);
		Eigen::Quaternionf eigenOrientation(orientation.w, orientation.x, orientation.y, orientation.z);
	
		pcl::transformPointCloud(*m_pointcloud,*transformedPointCloud, eigenTranslation, eigenOrientation);
		return transformedPointCloud;
	}*/

	return nullptr;
}

