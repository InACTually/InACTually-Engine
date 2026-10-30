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

#include "camera/CameraManager.hpp"
#include "marker/MarkerRoomNode.hpp"
#include "camera/CameraRoomNode.hpp"
#include "processing/TextureHelper.hpp"
#include <condition_variable>
#include <mutex>
#if __has_include(<opencv2/aruco.hpp>)
#define WITHARUCO
#endif

#ifdef WITHARUCO
#include <opencv2/aruco.hpp>
#endif


namespace act {
	namespace comp {

		template <class C>
		class DetectorBase {
		public:
			DetectorBase(std::string name, room::CameraRoomNodeRef camera = nullptr)
				: m_name(name)
			{
				m_cameraImageInPort = proc::ImageInputPort::create(proc::PT_IMAGE, "cameraImage", [&](cv::UMat image) {
					if (image.empty())
						return;

					{
						std::lock_guard<std::mutex> lock(m_stateMutex);
						m_currentImage = image;
						m_hasNewImage = true;
					}
					m_stateChanged.notify_one();
					});

				if (camera)
					setCamera(camera);
			}

			~DetectorBase() {
				doDetecting(false);
			};

			virtual ci::Json toJson() { return ci::Json::object(); };
			virtual void fromJson(ci::Json json) {};

			std::string getName() { return m_name; }

			room::CameraRoomNodeRef getCamera() { return m_camera; };

			void setCamera(room::CameraRoomNodeRef camera, bool isDetecting = true) {
				if (camera && m_camera && camera->getUID() == m_camera->getUID()) {
					doDetecting(isDetecting);
					return;
				}

				if (m_camera)
					m_camera->getCameraImagePort()->disconnect(m_cameraImageInPort);

				m_camera = camera;

				if (camera) {
					m_camera->getCameraImagePort()->connect(m_cameraImageInPort);
					doDetecting(isDetecting);
				}
				else {
					doDetecting(false);
				}
			}

			cv::UMat getFeedbackImage() { return m_feedbackImage; }

			cv::ogl::Texture2D getFeedbackTexture() {
				if (!m_textureHelper)
					m_textureHelper = proc::TextureHelper::create();

				auto tex = m_textureHelper->toTexture(m_feedbackImage);
				return tex;
			}


			void doDetecting(bool detecting = true) {
				{
					std::lock_guard<std::mutex> lock(m_stateMutex);
					if (m_detecting == detecting)
						return;
					m_detecting = detecting;
				}

				if (detecting) {
					m_thread = std::thread([this]() {
						while (true) {
							{
								std::unique_lock<std::mutex> lock(m_stateMutex);
								m_stateChanged.wait(lock, [this]() { return !m_detecting || (m_hasNewImage && m_isInitialized); });
								if (!m_detecting)
									break;
								m_hasNewImage = false;
							}
							detect();
						}
						});
				}
				else {
					m_stateChanged.notify_all();
					if (m_thread.joinable())
						m_thread.join();
				}
			}

			bool isDetecting() {
				std::lock_guard<std::mutex> lock(m_stateMutex);
				return m_detecting;
			}

			bool hasNewCandidates() { return m_areNewCandidatesAvailable; }
			std::vector<C> getCandidates() { m_areNewCandidatesAvailable = false; return m_currentCandidates; }

		protected:
			bool m_isInitialized = false;

			cv::UMat m_currentImage;
			cv::UMat m_feedbackImage;
			room::CameraRoomNodeRef m_camera;

			std::vector<C> m_currentCandidates;
			bool m_areNewCandidatesAvailable = false;

			virtual void detect() = 0;

		private:
			std::string m_name;

			proc::ImageInputPortRef m_cameraImageInPort;
			proc::TextureHelperRef m_textureHelper;
			bool m_hasNewImage = false;

			bool m_detecting = false;
			std::thread m_thread;
			std::mutex m_stateMutex;
			std::condition_variable m_stateChanged;

		};
	}
}
