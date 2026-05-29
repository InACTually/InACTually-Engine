
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
#include "dmx/DMXManager.hpp"
#include "dmx/MovingHeadRoomNode.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class MovingHeadProcNode : public ProcNodeBase
		{
		public:
			MovingHeadProcNode();
			~MovingHeadProcNode();

			PROCNODECREATE(MovingHeadProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onDimmer(float dim);
			void onZoom(float zoom);
			void onColor(ci::Color color);
			void onLookAt(vec3 lookAt);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			float	m_dim;
			float	m_zoom;
			ci::Anim<Color>	m_color;
			vec3	m_lookAt;
			bool	m_isLookingAt;
			bool	m_isUpsideDown;

			room::DMXManagerRef m_dmxMgr;

			int		m_selectedMovingHead;
			room::MovingHeadRoomNodeRef m_movingHead;

		}; using MovingHeadProcNodeRef = std::shared_ptr<MovingHeadProcNode>;

	}
}