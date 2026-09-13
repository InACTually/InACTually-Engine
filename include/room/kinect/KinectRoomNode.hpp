
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeBase.hpp"

#include "cinder/Capture.h"

#include "kinect/KinectDevice.hpp"
#include <k4a/k4a.hpp>

#include "body/Body.hpp"

#include "pointcloud/PointcloudRoomNode.hpp"

#include "PortType.hpp"


namespace act {
	namespace room {

		class KinectRoomNode : public RoomNodeBase
		{
		public:
			KinectRoomNode(KinectDeviceRef kinect, std::string deviceName, std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID = "");
			virtual ~KinectRoomNode();

			static std::shared_ptr<KinectRoomNode> create(KinectDeviceRef kinect, std::string deviceName, std::string name, glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 0.5f, act::UID replyUID = "") { return std::make_shared<KinectRoomNode>(kinect, deviceName, name, position, rotation, radius, replyUID); };

			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

			Pointcloud getWorldSpacePointCloud();

			act::proc::ImageOutputPortRef getKinectImagePort() { return m_kinectImageOutPort; }
			act::proc::ImageOutputPortRef getKinectDepthPort() { return m_kinectDepthOutPort; }
			act::proc::ImageOutputPortRef getKinectIRPort() { return m_kinectIROutPort; }
			act::proc::ImageOutputPortRef getKinectBIMPort() { return m_kinectBIMOutPort; }
			act::proc::OutputPortRef<room::BodyRefList> getBodyPort() { return m_kinectBodiesOutPort; }

		private:
			std::string m_name = "";
			std::string m_deviceName = "";
			std::string m_nameAddition = "[AzureKinect]";

			glm::vec2 asd = glm::vec2(0, 0);

			const char* jointSelection[32] = {
				"01 - PELVIS",
				"02 - SPINE_NAVAL",
				"03 - SPINE_CHEST",
				"04 - NECK",
				"05 - CLAVICLE_LEFT",
				"06 - SHOULDER_LEFT",
				"07 - ELBOW_LEFT",
				"08 - WRIST_LEFT",
				"09 - HAND_LEFT",
				"10 - HANDTIP_LEFT",
				"11 - THUMB_LEFT",
				"12 - CLAVICLE_RIGHT",
				"13 - SHOULDER_RIGHT",
				"14 - ELBOW_RIGHT",
				"15 - WRIST_RIGHT",
				"16 - HAND_RIGHT",
				"17 - HANDTIP_RIGHT",
				"18 - THUMB_RIGHT",
				"19 - HIP_LEFT",
				"20 - KNEE_LEFT",
				"21 - ANKLE_LEFT",
				"22 - FOOT_LEFT",
				"23 - HIP_RIGHT",
				"24 - KNEE_RIGHT",
				"25 - ANKLE_RIGHT",
				"26 - FOOT_RIGHT",
				"27 - HEAD",
				"28 - NOSE",
				"29 - EYE_LEFT",
				"30 - EAR_LEFT",
				"31 - EYE_RIGHT",
				"32 - EAR_RIGHT"
			};

			glm::ivec2 m_captureSize;
			glm::ivec2 m_displaySize;

			bool cameraInit = false;

			cv::UMat				m_captureImage;
			cv::UMat				m_captureDepth;
			cv::UMat				m_captureIR;
			cv::UMat				m_captureBIM;
			Pointcloud m_pointcloud;
			PointcloudRoomNodeRef	m_pointcloudRoomNode;

			std::map<uint32_t, k4abt_skeleton_t> m_captureBodies;
			std::map<uint32_t, k4abt_skeleton_t> m_repositionedBodies;

			KinectDeviceRef			m_kinect;
			glm::vec3				m_offsetPosition = glm::vec3(0.0f);
			glm::vec3				m_offsetRotation = glm::vec3(0.0f);

			bool					m_isProvidingPointCloud;

			ci::CameraPersp			m_cameraPersp;

			act::proc::ImageOutputPortRef	m_kinectImageOutPort;
			act::proc::ImageOutputPortRef	m_kinectDepthOutPort;
			act::proc::ImageOutputPortRef	m_kinectIROutPort;
			act::proc::ImageOutputPortRef	m_kinectBIMOutPort;

			act::proc::OutputPortRef<room::BodyRefList>		m_kinectBodiesOutPort;

			void initCameraPersp(int width, int height);
			std::map<uint32_t, k4abt_skeleton_t> repositionBodies(std::map<uint32_t, k4abt_skeleton_t> bodyMap);
			glm::vec3 calcRoomPos(glm::vec3 pos);			

		}; using KinectRoomNodeRef = std::shared_ptr<KinectRoomNode>;
	}
}