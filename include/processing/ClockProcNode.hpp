
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
//#include "cinder/Tween.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class ClockProcNode : public ProcNodeBase
		{
		public:
			ClockProcNode();
			~ClockProcNode();

			PROCNODECREATE(ClockProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<float>	m_valuePort;
			OutputPortRef<bool>		m_bangPort;
			float m_value;
			float m_startedAt;
			float m_elapsed;

			bool m_hasStarted = false;

			float m_timeUntil;

			bool m_bang = false;

			void start();
			void stop();
		}; using ClockProcNodeRef = std::shared_ptr<ClockProcNode>;

	}
}