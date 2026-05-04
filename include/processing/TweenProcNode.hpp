/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "EasingProcNode.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class TweenProcNode : public ProcNodeBase
		{
		public:
			TweenProcNode();
			~TweenProcNode();

			PROCNODECREATE(TweenProcNode);

			void setup(act::room::RoomManagers roomMgrs)	override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<number>	m_atValuePort;
			bool m_append;
			float m_duration;
			int m_selectedEasingIndex;

			ci::Anim<number> m_value;
			ci::EaseFn m_easing;

			std::vector<std::string> m_easingNames;
			std::vector<ci::EaseFn> m_easingFunctions;

			void createEasings();

		}; using TweenProcNodeRef = std::shared_ptr<TweenProcNode>;

	}
}
