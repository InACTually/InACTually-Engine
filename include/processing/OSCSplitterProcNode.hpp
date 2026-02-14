
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
#include "Osc.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class OSCSplitterProcNode : public ProcNodeBase
		{
		public:
			OSCSplitterProcNode();
			~OSCSplitterProcNode();

			PROCNODECREATE(OSCSplitterProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			std::string m_msgName;
			OutputPortRef<ci::osc::Message>	m_oscPort;
			std::vector<PortBaseRef> m_allInputPorts;

			OutputPortRef<float> x;
			OutputPortRef<float> y;
			OutputPortRef<float> z;
			OutputPortRef<bool> s1;
			OutputPortRef<bool> s2;
			OutputPortRef<bool> s3;
			OutputPortRef<bool> s4;
			OutputPortRef<bool> s5;
			OutputPortRef<bool> s6;
			OutputPortRef<bool> s7;
			OutputPortRef<bool> s8;
			OutputPortRef<bool> s9;
			OutputPortRef<float> sIntensity;
			OutputPortRef<float> squeak;

			float initx = -1;
			float inity;
			float initz;

			int initval1 = -1;
			int initval2;
			int initval3;
			int initval4;
			int initval5;
			int initval6;
			int initval7;
			int initval8;
			int initval9;
				

		}; using OSCSplitterProcNodeRef = std::shared_ptr<OSCSplitterProcNode>;

	}
}