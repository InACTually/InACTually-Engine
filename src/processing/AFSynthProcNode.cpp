
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
#include "AFSynthProcNode.hpp"


act::proc::AFSynthProcNode::AFSynthProcNode() : ProcNodeBase("AFSynth") {

	m_drawSize = glm::ivec2(250, 250);

	auto fm = createNumberInput("FM Value", [&](float value) { this->onModValue(value); });
	auto am = createNumberInput("AM Value", [&](float value) { this->onModValue(value); });
	auto mod = createNumberInput("mod", [&](float value) { this->onModValue(value); });

	m_audioOutPort = createAudioNodeOutput("audioNode");
	m_volumePort = createNumberOutput("volume");

	m_scaleValue = 1.0f;



	ci::audio::Context* ctx = ci::audio::master();

	m_osc		= ctx->makeNode(new ci::audio::GenTriangleNode);
	m_modFM		= ctx->makeNode(new ci::audio::GenSineNode);
	m_modAM		= ctx->makeNode(new ci::audio::GenSineNode);
	m_add		= ctx->makeNode(new ci::audio::AddNode);
	m_mul		= ctx->makeNode(new ci::audio::MultiplyNode);
	m_gain		= ctx->makeNode(new ci::audio::GainNode);
	m_gainFM	= ctx->makeNode(new ci::audio::GainNode);
	m_lowP		= ctx->makeNode(new ci::audio::FilterLowPassNode);
	auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	m_monitor	= ctx->makeNode(new ci::audio::MonitorNode(monitorFormat));

	//m_sine >> m_gain;

	m_osc->setFreq(220);
	m_modFM->setFreq(220);
	m_modAM->setFreq(20);
	m_add->setValue(200);
	m_mul->setValue(50);
	m_gain->setValue(1.0f);
	m_gainFM->setValue(200);

	m_modFM >> m_gainFM;
	m_osc->getParamFreq()->setProcessor(m_gainFM);

	m_mul->getParam()->setProcessor(m_modAM);

	m_osc >> m_mul >> m_lowP >> m_gain >> m_monitor;

	m_osc->enable();
	m_modFM->enable();
	m_modAM->enable();
	m_gain->enable();


	m_audioOutPort->send(m_gain);
}

act::proc::AFSynthProcNode::~AFSynthProcNode() {
}

void act::proc::AFSynthProcNode::onFMValue(float value)
{
	m_freqFM = (value) * 100;
}

void act::proc::AFSynthProcNode::onAMValue(float value)
{
	m_freqAM = value * 75;
}

void act::proc::AFSynthProcNode::onModValue(float value)
{
	m_freqFM = (value) * 120;
	m_freqAM = value * 80;
	m_modStrength = std::clamp(std::clamp((value), 0.0f, 1.0f) * 1000 + (value * 1000) + 30, 0.0f, 1500.f);
	m_volume = value;
}

void act::proc::AFSynthProcNode::update() {
	m_modAM->setFreq(m_freqAM);
	m_modFM->setFreq(m_freqFM);
	m_gainFM->setValue(m_modStrength);

	m_lowP->setCutoffFreq(m_cutOff);
	m_lowP->setQ(m_Q);

	m_gain->setValue(m_volume);

	m_volumePort->send(std::clamp((m_monitor->getVolume()+100)*0.01f, 0.0f, 1.0f));
}

void act::proc::AFSynthProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - 50);

	ImGui::SetNextItemWidth(m_drawSize.x * 2);
	preventDrag(ImGui::SliderFloat("scale value", &m_scaleValue, 0.1f, 5.0f));

	preventDrag(ImGui::SliderFloat("m_volume value", &m_volume, 0.0f, 1.0f));
	preventDrag(ImGui::SliderInt("m_cutOff value", &m_cutOff, 0, 15000));
	preventDrag(ImGui::SliderInt("m_Q value", &m_Q, 1, 10));
	preventDrag(ImGui::SliderInt("m_freqAM value", &m_freqAM, 0, 150));
	preventDrag(ImGui::SliderInt("m_freqFM value", &m_freqFM, 0, 150));
	preventDrag(ImGui::SliderInt("m_modStrength value", &m_modStrength, 0, 1500));

	endNodeDraw();
}

void act::proc::AFSynthProcNode::setup(act::room::RoomManagersRef rmgr)
{
	
}

ci::Json act::proc::AFSynthProcNode::toParams() {
	ci::Json json = ci::Json::object();
 
	return json; 
}

void act::proc::AFSynthProcNode::fromParams(ci::Json json) {
}
