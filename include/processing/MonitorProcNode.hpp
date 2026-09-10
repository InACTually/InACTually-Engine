
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021ï¿½2025 Lars Engeln, Fabian Tï¿½pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

#include <atomic>
#include <opencv2/core/opengl.hpp>


namespace act {
	namespace proc {

		class MonitorProcNode : public ProcNodeBase
		{
		public:
			MonitorProcNode();
			~MonitorProcNode();

			PROCNODECREATE(MonitorProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			bool m_show;
			bool m_display;
			bool m_fullscreen;
			bool m_projector;
			act::room::DisplayManagerRef		m_displayMgr;
			act::room::ProjectorManagerRef		m_projectorMgr;

			ci::gl::Texture2dRef				m_texture;
			glm::ivec2 adaptSize(glm::ivec2 size);
			float								m_displayScale;

			cv::UMat							m_rgbaBuffer;
			cv::ogl::Texture2D					m_glTex;

			ImageOutputPortRef					m_imagePort;
			OutputPortRef<ci::gl::Texture2dRef>	m_texturePort;

			std::atomic_bool					m_frameInFlight { false };

		}; using MonitorProcNodeRef = std::shared_ptr<MonitorProcNode>;

	}
}