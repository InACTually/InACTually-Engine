
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class IfProcNode : public ProcNodeBase
		{
		public:
			IfProcNode();
			~IfProcNode();

			PROCNODECREATE(IfProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<bool>	m_resultPort;
			OutputPortRef<bool>	m_resultPort_neg;

			float			m_compareValue;
			int				m_compareOptionSelection;
			std::vector<std::string>	m_compareOptions;
			
			std::string		m_compareText;
			float			m_compareVec3[3] = {0.,0.,0.};
			bool			m_not;
			float			m_precision = 0.1;
			bool			m_fireOnlyOnce = false;
			bool			m_lastCompareResult = false;

			bool compareValue(float number);
			bool compareText(std::string text);
			bool compareVec(vec3 pos);

			void sendResult(bool val);

			void onNumber(float number);
			void onText(std::string text);
			void onFeature(feature f);
			void onFeatures(featureList fList);
			void onVec(vec3 pos);

		}; using IfProcNodeRef = std::shared_ptr<IfProcNode>;

	}
}