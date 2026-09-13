
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"

#include <string_view>

#include <k4a/k4a.hpp>
#include "kinect/KinectDevice.hpp"

#include "KinectHelper.hpp"
#include "body/Body.hpp"

#include "pointcloud/PointcloudRoomNode.hpp"

#include "PortRefs.hpp"


namespace act {
	namespace room {

		enum kinectImagePort {
			KP_IMAGE,
			KP_DEPTH,
			KP_INFRARED,
			KP_BIM
		};

		enum kinectConnectionState {
			K4_CONNECTED,
			K4_NETWORK_CONNECTED,
			K4_DISCONNECTED
		};



		class KinectManager : public RoomNodeManagerBase {
		public:
			KinectManager();
			~KinectManager();

			static	std::shared_ptr<KinectManager> create() { return std::make_shared<KinectManager>(); };

			void	setup() override;
			void	update() override;
			void	draw() override;

			void	updateKinects();

			act::room::RoomNodeBaseRef drawMenu() override;

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			act::proc::ImageOutputPortRef getKinectPort(std::string kinectName, kinectImagePort port);
			act::proc::OutputPortRef<room::BodyRefList> getBodyPortByName(std::string kinectName);
			act::proc::OutputPortRef<room::BodyRefList> getBodyPort(UID uid);

			std::vector<std::string> getKinectNames() { return m_usedDeviceNames; }

			act::room::KinectDeviceRef getDeviceByName(std::string name);

			glm::vec3 getKinectPositionByUID(act::UID uid);
			glm::vec3 getKinectOrientationByUID(act::UID uid);

			PointcloudRoomNodeRef createPointcloudRoomNode();

			room::BodyRefList getBodies();
			std::map < uint32_t, k4abt_skeleton_t> getBodiesMerged() { return m_bodiesMerged; };

			std::map<std::string, kinectConnectionState> getDevicesAndStates() { return m_devicesAndStates;};
			bool hasAvailableDevice(std::string deviceName);
			act::room::RoomNodeBaseRef	addDevice(std::string deviceName, std::string name);

		private:

			bool m_isDummyOpenDialog = false;
			std::string m_dummyPath = "";
			act::room::RoomNodeBaseRef m_dummyDevice;

			int m_devicesInstalledCount = 0;

			bool cpu_mode = false;

			bool simpleMergeMode = true;
			//When 2 bodies' distances (in meter) are under the threshold, they get merged to one body
			float m_bodyClippingThreshold = 0.3f;

			std::thread	m_updateThread;

			//Parent joints regarding to joint index --> https://docs.microsoft.com/en-us/azure/kinect-dk/body-joints
			const int jointParentLookUp[32] = { 0, 0, 1, 2, 2, 4, 5, 6, 7, 8, 7, 2, 11, 12, 13, 14, 15, 14, 0, 18, 19, 20, 0, 22, 23, 24, 3, 26, 26, 26, 26, 26 };

			std::map < uint32_t, ci::ColorA> m_colorMapping;
			const float COLOROFFSET = 0.56f;

			std::map<uint32_t, k4abt_skeleton_t> m_bodiesMerged;
			std::map<uint32_t, std::pair< k4abt_skeleton_t, int>> m_bodiesWithDuplicates;

			std::map<std::string, kinectConnectionState> m_devicesAndStates;

			std::vector<act::room::KinectDeviceRef>	m_devices;
			std::vector<std::string>	m_availableDeviceNames;
			std::vector<std::string>	m_usedDeviceNames;
			std::vector<std::string>	m_disconnectedDeviceNames;

			int							m_selectedDevice;

			void refreshLists() override;
			void stopDevices();

			std::vector<act::room::KinectDeviceRef>	getDevices();
			void addDummyDevice(std::string path);

			void checkRedundantBodies(std::map<uint32_t, k4abt_skeleton_t> currBodies);

			bool checkConnectionState(std::string kinectName);

			//Eigen::Matrix4f			registerPointClouds(act::room::Pointcloud source, act::room::Pointcloud target);
			//void					performPointCloudRegistration();
			//pcl::PointCloud<pcl::PointXYZ>::Ptr reducePointCloudSize(pcl::PointCloud<pcl::PointXYZ>::Ptr source);

		};
		using KinectManagerRef = std::shared_ptr<KinectManager>;
	}
}