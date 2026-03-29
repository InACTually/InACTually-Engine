
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeBase.hpp"

#include "cinder/Capture.h"

#include "camera/CameraDevice.hpp"
#include "pointcloud/PointcloudRoomNode.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace comp {
		class DepthDetector;
	}

	namespace room {

		class CameraRoomNode : public RoomNodeBase, public std::enable_shared_from_this<CameraRoomNode>
		{
		public:
			CameraRoomNode(CameraDeviceRef camera, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID = "");

			CameraRoomNode(ci::Capture::DeviceRef deviceRef, std::string deviceName, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID = "");
			virtual ~CameraRoomNode();

			static std::shared_ptr<CameraRoomNode> create(CameraDeviceRef camera, std::string name, ci::vec3 position = ci::vec3(0.0f, 1.0f, 0.0f), ci::vec3 rotation = ci::vec3(0.0f,0.0f,0.0f), float radius = 0.5f, act::UID replyUID = "") { return std::make_shared<CameraRoomNode>(camera, name, position, rotation, radius, replyUID); };
			static std::shared_ptr<CameraRoomNode> create(ci::Capture::DeviceRef deviceRef, std::string deviceName, std::string name, ci::vec3 position = ci::vec3(0.0f, 1.0f, 0.0f), ci::vec3 rotation = ci::vec3(0.0f, 0.0f, 0.0f), float radius = 0.5f, act::UID replyUID= "") { return std::make_shared<CameraRoomNode>(deviceRef, deviceName, name, position, rotation, radius, replyUID); };


			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

			std::shared_ptr<proc::OutputPort<proc::image/*, std::shared_ptr<CameraRoomNode>*/>> getCameraImagePort() { return m_cameraImagePort; }
			std::string getDeviceName() { return m_camera->getName(); }
			cv::Mat getRotMat() { return m_rotMat; }
			void setRotMat(cv::Mat rotMat) { m_rotMat = rotMat;	}
			CameraDeviceRef getCamera() { return m_camera; }

			bool isDetectingMarkers() { return m_isDetectingMarkers; }
			void setIsDetectingMarkers(bool isDetectingMarkers) { m_isDetectingMarkers = isDetectingMarkers; }
			bool isDetectingObjects() { return m_isDetectingObjects; }
			void setIsDetectingObjects(bool isDetectingObjects) { m_isDetectingObjects = isDetectingObjects; }
			
		private:
			cv::Mat					m_rotMat;
			ci::ivec2				m_displaySize;

			CameraDeviceRef			m_camera;

			std::thread				m_thread;

			//ci::SurfaceRef			m_captureSurface;
			ci::gl::Texture2dRef	m_captureTexture;
			ci::gl::Texture2dRef	m_depthTexture;

			ci::CaptureRef			m_capture;
			std::string				m_captureName;

			ci::CameraPersp			m_cameraPersp;
		
			std::shared_ptr<proc::OutputPort<proc::image/*, std::shared_ptr<CameraRoomNode>*/>> m_cameraImagePort;

			bool					m_isDetectingMarkers	= true;
			bool					m_isDetectingDepth		= false;
			bool					m_isDetectingObjects	= false;

			std::shared_ptr<comp::DepthDetector>	m_depthDetector;
			Pointcloud m_pointcloud;
			PointcloudRoomNodeRef	m_pointcloudRoomNode;

			Pointcloud toPointCloud(cv::UMat depth, cv::UMat color);

		}; using CameraRoomNodeRef = std::shared_ptr<CameraRoomNode>;
		
	}
}