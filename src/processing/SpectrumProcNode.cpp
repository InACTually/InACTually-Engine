
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

#include "procpch.hpp"
#include "SpectrumProcNode.hpp"

#include "implot.h"

act::proc::SpectrumProcNode::SpectrumProcNode() : ProcNodeBase("Spectrum") {
	
	m_drawSize = ivec2(600, 400);

	m_show = false;

	m_volumeOutPort   = createNumberOutput("volume");
	m_spectrumOutPort = createNumberListOutput("spectrum");
	m_centroidOutPort = createNumberOutput("centroid");


	auto audioNodeIn = createAudioNodeInput(
		"audioNode",
		[&](audio::NodeRef audioNode) {
		audioNode >> m_monitorSpectralNode;
		}
	);

}

act::proc::SpectrumProcNode::~SpectrumProcNode() {

}

void act::proc::SpectrumProcNode::setup(act::room::RoomManagers roomMgrs) {
	auto ctx = audio::Context::master();
	
	auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize(4096).windowSize(2048);
	m_monitorSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(monitorFormat));
}

void act::proc::SpectrumProcNode::update() {
	if (m_monitorSpectralNode && m_monitorSpectralNode->isEnabled()) {
		m_volume = m_monitorSpectralNode->getVolume();
		m_spectrum = m_monitorSpectralNode->getMagSpectrum();
		m_centroid = m_monitorSpectralNode->getSpectralCentroid();

		m_volumeOutPort->send(m_volume);
		m_spectrumOutPort->send(m_spectrum);
		m_centroidOutPort->send(m_centroid);
	}
}

void act::proc::SpectrumProcNode::draw() {
	beginNodeDraw();

	if (ImGui::Checkbox("show", &m_show)) {
		// checkbox was clicked
	}

	//ImPlot::FitNextPlotAxes(true, false, false, false);


	int SR = ci::audio::master()->getSampleRate();
	int fftSize = m_spectrum.size();
	double ticks[] = { 0, fftSize * 0.015625, fftSize * 0.03125, fftSize * 0.0625, fftSize * 0.125, fftSize * 0.25, fftSize * 0.5, fftSize * 1.0 };
	const char* labels[] = { "0",  "375",  "750", "1.5k", "3k", "6k", "12k", "24k" };
	//if (ImPlot::BeginPlot("Spectrum", "Hz", "dB", ImVec2(-1, 0), 0, ImPlotAxisFlags_LogScale)) {

	if (m_spectrum.size() && ImPlot::BeginPlot("Plot", m_drawSize)) {
		ImPlot::SetupAxesLimits(0.0f, m_spectrum.size(), 0.0f, 0.05f, ImGuiCond_Always);
		ImPlot::SetupAxisTicks(ImAxis_X1, ticks, 7, labels, false);
		ImPlot::SetupAxis(ImAxis_X1, "Hz");

		ImGui::SetNextItemWidth(m_drawSize.x);

		ImPlot::PlotLine("spectrum", m_spectrum.data(), m_spectrum.size());

		ImPlot::EndPlot();
	}

	ImGui::SliderFloat("volume", &m_volume, 0.0f, 120.f);

	if (m_show) {
		gl::pushMatrices();
		
		gl::pushMatrices();
	}

	endNodeDraw();
}



ci::Json act::proc::SpectrumProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::SpectrumProcNode::fromParams(ci::Json json) {

}
