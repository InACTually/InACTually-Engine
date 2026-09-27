
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "camera/CameraDevice.hpp"


act::room::CameraDevice::CameraDevice() {
}

act::room::CameraDevice::CameraDevice(ci::Capture::DeviceRef deviceReference) {
	this->m_device = deviceReference;
	try {
		m_name = deviceReference->getName();
		m_capture = ci::Capture::create(m_captureSize.x, m_captureSize.y, m_device);
		m_captureSize = m_capture->getSize();
		m_capture->start();
	}
	catch (...) {
		m_error = true;
		CI_LOG_E("Failed to create capture device");
	}

}

act::room::CameraDevice::~CameraDevice() {
	try {
		//if(m_capture && m_capture->isCapturing())
			//m_capture->stop();
	}
	catch (...) {
		m_error = true;
		CI_LOG_E("Failed to stop capture device");
	}
}


bool act::room::CameraDevice::update() {
	if (!m_capture)
		return false;

	if (m_capture && m_capture->checkNewFrame()) {
		auto&& image = ci::toOcv(*m_capture->getSurface()).getUMat(cv::ACCESS_WRITE);

		if (m_flipped) 
			cv::flip(image, image, 1);
		//m_captureci::Surface = m_capture->getSurface();
		
		image.copyTo(m_currentImage);
		if (m_isCalibrated) {
			//cv::flip(image, image, 1);
			remap(m_currentImage);
		}

		return true;
	}
	return false;
}

cv::UMat act::room::CameraDevice::getCurrentImage()
{
	return m_currentImage;
}

cv::UMat act::room::CameraDevice::getUndistortedImage()
{
	//if (m_isCalibrated)
		//remap(m_currentImage);

	return m_undistoretedImage;
}

void act::room::CameraDevice::setCalibration(cv::Mat intrinsic, cv::Mat distCoeffs)
{
	m_intrinsic = intrinsic;
	m_distCoeffs = distCoeffs;
	cv::initUndistortRectifyMap(m_intrinsic, m_distCoeffs, cv::Mat(),
		cv::getOptimalNewCameraMatrix(m_intrinsic, m_distCoeffs, m_cvSize, 1, m_cvSize, 0)
		, m_cvSize, CV_16SC2, m_map1, m_map2);
	m_isCalibrated = true;
}

void act::room::CameraDevice::remap(cv::UMat image)
{
	cv::remap(image, m_undistoretedImage, m_map1, m_map2, cv::INTER_LINEAR);
}



