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

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class NumberProcNode : public ProcNodeBase
		{
		public:
			NumberProcNode();
			~NumberProcNode();

			PROCNODECREATE(NumberProcNode);

			void setup(act::room::RoomManagers roomMgrs)	override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<number>	m_outPort;
			float m_number;

		}; using NumberProcNodeRef = std::shared_ptr<NumberProcNode>;

	}
}
