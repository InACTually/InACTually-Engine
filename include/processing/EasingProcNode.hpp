
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
//#include "cinder/Tween.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class EasingProcNode : public ProcNodeBase
		{
		public:
			EasingProcNode();
			~EasingProcNode();

			PROCNODECREATE(EasingProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			float ease(float value);

			std::vector<std::string>& getEasingNames() { return m_easingNames; }
			void setEasing(int index);

		private:
			OutputPortRef<float>	m_valuePort;
			float m_value;

			int m_selectedEasing = 0;
			std::vector<std::string> m_easingNames;
			std::map<std::string, std::function<float(float)>> m_easingsMap;
			void createEasingsList();

		}; using EasingProcNodeRef = std::shared_ptr<EasingProcNode>;

	}
}