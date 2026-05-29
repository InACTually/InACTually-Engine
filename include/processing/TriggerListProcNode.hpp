
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

#include "ProcNodeBase.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class TriggerListProcNode : public ProcNodeBase
		{
		public:
			TriggerListProcNode();
			~TriggerListProcNode();

			PROCNODECREATE(TriggerListProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:

			bool init;

			int m_nextTrigger = 0;
			bool m_isTriggering = false;
			bool m_startAtRandom = false;

			InputPortRef<bool> m_fireInputPort;
			InputPortRef<bool> m_stepUpInputPort;
			InputPortRef<bool> m_stepDownInputPort;
			InputPortRef<bool> m_resetInputPort;

			void fireTrigger();
			void stepUp();
			void stepDown();
			void setNextTrigger(int index);
			void reset();

			void addTrigger(int index = -1); // inserts if index != -1 
			void setTriggerLabel(int index, std::string label);
			void removeTrigger(int index);
			void moveTriggerUp(int index);
			void moveTriggerDown(int index);
			void moveTriggerTo(int index, int to);


		}; using TriggerListProcNodeRef = std::shared_ptr<TriggerListProcNode>;

	}
}