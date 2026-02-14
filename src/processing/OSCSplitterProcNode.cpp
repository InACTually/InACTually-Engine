
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

#include "procpch.hpp"
#include "OSCSplitterProcNode.hpp"

#include "processing/MatToBase64.hpp"


act::proc::OSCSplitterProcNode::OSCSplitterProcNode() : ProcNodeBase("OSCSplitter") {

	{
		x = createNumberOutput("x");
		y = createNumberOutput("y");
		z = createNumberOutput("z");

		s1 = createBoolOutput("s1");
		s2 = createBoolOutput("s2");
		s3 = createBoolOutput("s3");
		s4 = createBoolOutput("s4");
		s5 = createBoolOutput("s5");
		s6 = createBoolOutput("s6");
		s7 = createBoolOutput("s7");
		s8 = createBoolOutput("s8");
		s9 = createBoolOutput("s9");
		sIntensity = createNumberOutput("intensity");

		{
			auto input = InputPort<ci::osc::Message>::create(PT_OSC, "vec3", [&](ci::osc::Message msg) {
				float val1 = msg[0].int32();
				float val2 = msg[1].int32();
				float val3 = msg[2].int32();

				if(initval1 <= -1) {
					initval1 = val1;
					initval2 = val2;
					initval3 = val3;
				}

				val1 -= initval1;
				val2 -= initval2;
				val3 -= initval3;

				val1 /= 400.0f;
				val2 /= 400.0f;
				val3 /= 400.0f;

				x->send(val2 + val3);
				//y->send(val2 / 400.0f);
				//z->send(val3 / 400.0f);
			});
			m_inputPorts.push_back(input);
		}

		{
			auto input = InputPort<ci::osc::Message>::create(PT_OSC, "int9", [&](ci::osc::Message msg) {
				int thresh = 80;

				if (initval1 <= -1) {
					initval1 = msg[0].int32();
					initval2 = msg[1].int32();
					initval3 = msg[2].int32();
					initval4 = msg[3].int32();
					initval5 = msg[4].int32();
					initval6 = msg[5].int32();
					initval7 = msg[6].int32();
					initval8 = msg[7].int32();
					initval9 = msg[8].int32();
			
				}
			
				int val2 = msg[1].int32() - initval1;
				int val3 = msg[2].int32() - initval2;
				int val4 = msg[3].int32() - initval3;
				int val1 = msg[0].int32() - initval4;
				int val5 = msg[4].int32() - initval5;
				int val6 = msg[5].int32() - initval6;
				int val7 = msg[6].int32() - initval7;
				int val8 = msg[7].int32() - initval8;
				int val9 = msg[8].int32() - initval9;
				
				s1->send(val1 > thresh);
				s2->send(val2 > thresh);
				s3->send(val3 > thresh);
				s4->send(val4 > thresh);
				s5->send(val5 > thresh);
				s6->send(val6 > thresh);
				s7->send(val7 > thresh);
				s8->send(val8 > thresh);
				s9->send(val9 > thresh);

				float intensity =
					 (val1
					+ val2
					+ val3
					+ val4
					+ val5
					+ val6
					+ val7
					+ val8
					+ val9) / 9.0f;
				sIntensity->send(intensity);
			});
			m_inputPorts.push_back(input);
		}

		squeak = createNumberOutput("squeak");
		{
			auto input = InputPort<ci::osc::Message>::create(PT_OSC, "value", [&](ci::osc::Message msg) {
				squeak->send(msg[0].int32() / 4096.0f);
				});
			m_inputPorts.push_back(input);
		}

	}

}

act::proc::OSCSplitterProcNode::~OSCSplitterProcNode() {
}

void act::proc::OSCSplitterProcNode::update() {

}

void act::proc::OSCSplitterProcNode::draw() {
	beginNodeDraw();
	
	endNodeDraw();
}


ci::Json act::proc::OSCSplitterProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::OSCSplitterProcNode::fromParams(ci::Json json) {
}
