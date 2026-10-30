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

#pragma once

#include "RoomNodeManagerBase.hpp"

#include "DetectorBase.hpp"
#include "camera/CameraRoomNode.hpp"

namespace act {
	namespace comp {


		class DepthDetector : public DetectorBase<cv::UMat> {
		public:
			DepthDetector();
			DepthDetector(room::CameraRoomNodeRef camera);
			~DepthDetector();

			static	std::shared_ptr<DepthDetector> create(room::CameraRoomNodeRef camera) { return std::make_shared<DepthDetector>(camera); };

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			void setInputSize(cv::Size inputSize);
			cv::Size getInputSize() const { return m_blobSize; }

		private:			
			static constexpr const int m_defaultWidth = 192; //238;
			static constexpr const int m_defaultHeight = 192; // 238;
			float mean[3] = { 0.485f, 0.456f, 0.406f };
			float std[3] = { 0.229f, 0.224f, 0.225f };

			cv::UMat				m_colorImage;
			cv::dnn::Net			m_net;

			std::string				m_outputLayer;
			bool					m_isUsingTiny = true;
			void initNetwork();

			bool m_detecting = true;

			std::vector<cv::UMat>	m_detection;
			cv::Size				m_blobSize = cv::Size(m_defaultWidth, m_defaultHeight);
			void detect() override;

			glm::ivec2 m_displaySize = glm::ivec2(640, 360);

		};
		using DepthDetectorRef = std::shared_ptr<DepthDetector>;
	}
}
