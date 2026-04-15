
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class MonitorProcNode : public ProcNodeBase
		{
		public:
			MonitorProcNode();
			~MonitorProcNode();

			PROCNODECREATE(MonitorProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			bool m_show;
			bool m_display;
			bool m_projector;
			act::room::DisplayManagerRef	m_displayMgr;
			act::room::ProjectorManagerRef	m_projectorMgr;

			ci::gl::Texture2dRef	m_texture;
			ci::ivec2 adaptSize(ci::ivec2 size);
			float	m_displayScale;

			ImageOutputPortRef				m_imagePort;
			OutputPortRef<ci::gl::Texture2dRef>	m_texturePort;

		}; using MonitorProcNodeRef = std::shared_ptr<MonitorProcNode>;

	}
}