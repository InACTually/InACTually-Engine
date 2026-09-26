
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "roompch.hpp"

#include "cinder/Capture.h"
#include "TextureHelper.hpp"

namespace act {
	namespace room {

		class CameraDevice {

		public:
			CameraDevice();
			CameraDevice(ci::Capture::DeviceRef deviceReference);
			~CameraDevice();

			static std::shared_ptr<CameraDevice> create(ci::Capture::DeviceRef deviceRef) { return std::make_shared<CameraDevice>(deviceRef); };
			
			bool update();
			cv::UMat getCurrentImage();
			cv::UMat getUndistortedImage();

			std::string getName() { return m_name; }
			bool hasCapture() { return !!m_capture; }
			bool isCapturing() { return m_capture->isCapturing(); }

			bool isFlipped() { return m_flipped; }
			void setIsFlipped(bool flipped) { m_flipped = flipped; }
			
			std::vector<cv::UMat> m_calibImages;
			std::vector<std::vector<cv::Point2i>> m_calibAreas;
			ci::gl::Texture2dRef m_textureUndist;

			void setCalibration(cv::Mat intrinsic, cv::Mat distCoeffs);
			bool isCalibrated() { return m_isCalibrated; }
			cv::Mat getIntrinsic() { return m_intrinsic; }
			cv::Mat getDistCoeffs() { return m_distCoeffs; }

			glm::ivec2 getCaptureSize() { return m_captureSize; }
			void setCaptureSize(glm::ivec2 size) { m_captureSize = size; m_cvSize = cv::Size(m_captureSize.x, m_captureSize.y); }

		private:
			std::string				m_name;

			ci::CaptureRef			m_capture;
			glm::ivec2				m_captureSize = glm::ivec2(1280, 720);
			cv::Size				m_cvSize = cv::Size(1280, 720);

			bool	m_isCalibrated = false;
			bool	m_flipped = false;

			void	remap(cv::UMat image);
			
			cv::Mat m_intrinsic;
			cv::Mat m_distCoeffs;
			cv::Mat m_map1, m_map2;

			ci::Capture::DeviceRef	m_device;

			bool m_error = false;

			cv::UMat m_currentImage;
			cv::UMat m_undistoretedImage;

			char* m_description;
			

		}; using CameraDeviceRef = std::shared_ptr<CameraDevice>;
	}
}