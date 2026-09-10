
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"

#include <opencv2/opencv.hpp>

#include "pointcloud/PointcloudRoomNode.hpp"


namespace act {
	namespace proc {

		class PointcloudProcNode : public ProcNodeBase
		{
		public:
			PointcloudProcNode();
			~PointcloudProcNode();

			PROCNODECREATE(PointcloudProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams();
			void fromParams(ci::Json json);

		private:
			bool					m_scanWholeRoom;

			OutputPortRef<act::room::Pointcloud> m_pointcloudOutPort;
			ImageInputPortRef  m_colorImageInPort;
			ImageInputPortRef	m_depthImageInPort;
			InputPortRef<glm::vec2>	m_fovInPort;

			cv::UMat m_colorImageCache;
			cv::UMat m_depthImageCache;
			glm::vec2 m_fov;
			act::room::Pointcloud m_pointcloud = nullptr;

			act::room::PointcloudRoomNodeRef m_pointcloudRoomNode;

			int m_threshold = 50;
			float m_boxSize = 0.35;

			void createPointcloud(cv::UMat depthImage, cv::UMat colorImage);
		};

		using PointcloudProcNodeRef = std::shared_ptr<PointcloudProcNode>;

	}
}