
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"

#include "camera/CameraManager.hpp"


namespace act {
	namespace proc {

		class CameraProcNode : public ProcNodeBase
		{
		public:
			CameraProcNode();
			~CameraProcNode();

			PROCNODECREATE(CameraProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			act::room::CameraManagerRef		m_camMgr;
			int								m_selectedCamera = 0;
			std::string						m_selectedCameraName;
			act::UID						m_selectedCameraUID;

			ci::SurfaceRef					m_captureSurface;
			ci::gl::Texture2dRef			m_captureTexture;

			glm::ivec2						m_captureSize;

			bool							m_show;

			cv::VideoWriter					m_writer;

			ImageOutputPortRef				m_cameraImageOutPort;
			ImageInputPortRef				m_cameraImageInPort;

			void attachCamera(act::room::CameraRoomNodeRef camera);
			act::room::CameraRoomNodeRef	m_cameraRoomNode;
		};

		using CameraProcNodeRef = std::shared_ptr<CameraProcNode>;

	}
}