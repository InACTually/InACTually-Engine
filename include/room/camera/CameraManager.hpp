
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder/audio/audio.h" 
#include "RoomNodeManagerBase.hpp"


#include "cinder/Capture.h"

#include <opencv2/imgproc/imgproc.hpp>

#include "camera/CameraDevice.hpp"
#include "computing/CameraCalibrator.hpp"

#include "camera/CameraRoomNode.hpp"

namespace act {
	namespace room {

		class CameraManager : public RoomNodeManagerBase {
		public:
			CameraManager();
			~CameraManager();

			static	std::shared_ptr<CameraManager> create() { return std::make_shared<CameraManager>(); };

			void	setup() override;
			// void	update() override;
			// void	draw() override;

			act::room::RoomNodeBaseRef			drawMenu() override;

			virtual ci::Json					toJson();
			virtual void						fromJson(ci::Json json);

			act::room::CameraRoomNodeRef		getCamera(act::UID cameraUID);
			act::room::CameraRoomNodeRef		getCameraByIndex(int index);
			act::proc::ImageOutputPortRef		getCameraPort(act::UID cameraUID);
			std::vector<std::string>			getUsedCameraNames() { return m_usedDevicesNames; }

			bool hasAvailableDevice(std::string deviceName);
			act::room::RoomNodeBaseRef addDevice(std::string deviceName, std::string name);
			act::room::RoomNodeBaseRef addSelectedDevice(std::string deviceName, std::string name);

		private:
			act::room::CameraDeviceRef m_currentCamera;
			void				update();
			void				refreshLists() override;
			bool				m_doCalibrate = false;
			const std::string	kinectNameFilter = "Azure KinectDevice 4K CameraDevice";


			std::vector<ci::Capture::DeviceRef> m_devices;
			std::vector<std::string>			m_availableDeviceNames;

			std::vector<std::string>			m_usedDevicesNames;
			std::vector<act::UID>				m_usedDevicesUID; // correlated to m_usedDevicesNames

			int									m_selectedDevice;
			int									m_prevSelectedDevice;

			glm::ivec2							m_displaySize = glm::ivec2(640, 360);

			CameraCalibratorRef					m_calibrator;

			void setCameraByDeviceName(std::string deviceName);
			

		};
		using CameraManagerRef = std::shared_ptr<CameraManager>;
	}
}