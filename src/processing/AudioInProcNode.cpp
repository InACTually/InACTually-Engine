
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "AudioInProcNode.hpp"

#include "cinder/gl/Fbo.h"
#include "cinder/audio/Param.h"

#include "math.h"

const double UPPER_HZ = 4186.0; // C8
const double LOWER_HZ = 27.5; // C1
// A * log2(B * x)
const double B = (1.0 / LOWER_HZ);
const double A = 1 / log2(B * UPPER_HZ);

act::proc::AudioInProcNode::AudioInProcNode() : ProcNodeBase("AudioIn") {

	m_path			= "";
	m_isOpenDialog	= false;

	m_isPlaying = false;
	
	m_drawSize	= glm::ivec2(500, 150);
	
	auto ctx = ci::audio::Context::master();

	m_bufferPlayer = ctx->makeNode(new ci::audio::BufferPlayerNode());
	//m_bufferPlayer >> ctx->getOutput();

	auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(4096).windowSize(2048);
	m_spectralNode = ctx->makeNode(new ci::audio::MonitorSpectralNode(monitorFormat));

	m_volumeNode = ctx->makeNode(new ci::audio::MonitorNode(monitorFormat));

	m_bufferPlayer >> m_spectralNode;
	m_bufferPlayer >> m_volumeNode;

	m_audioNodePort = createAudioNodeOutput("audioNode");

	m_sharpnessPort		= createOutputPort("sharpness");
	m_regularityPort	= createOutputPort("regularity");
	m_sizePort			= createOutputPort("size");
	m_temperaturePort	= createOutputPort("temperature");
	m_contrastPort		= createOutputPort("contrast");
	m_saturationPort	= createOutputPort("saturation");

}

act::proc::AudioInProcNode::~AudioInProcNode() {
	
}

void act::proc::AudioInProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	
}

void act::proc::AudioInProcNode::init() {
	loadSound(m_path);
}

void act::proc::AudioInProcNode::update() {
	if(m_isOpenDialog) {
		m_isOpenDialog = false;
		m_path = ci::app::getOpenFilePath().string();
		loadSound(m_path);
	}
	if (m_bufferPlayer && m_isPlaying) {

		calculateFeatures();

		m_sharpnessPort->send(m_sharpnessFeature);
		m_regularityPort->send(m_regularityFeature);
		m_sizePort->send(m_sizeFeature);
		m_temperaturePort->send(m_temperatureFeature);
		m_contrastPort->send(m_contrastFeature);
		m_saturationPort->send(m_saturationFeature);

		m_audioNodePort->send(m_bufferPlayer);

	}
}

void act::proc::AudioInProcNode::draw() {

	beginNodeDraw();

	if(ImGui::Button(" load ")) {
		m_isOpenDialog = true;
	}

	ImGui::NewLine();

	if (m_isPlaying) {
		if (ImGui::Button(" stop ")) {
			onTrigger(true);
		}
	}
	else{
		if (ImGui::Button(" play ")) {
			onTrigger(true);
		}
	}
	ImGui::SameLine();
	ImGui::Text("File: ");
	ImGui::SameLine();
	if (m_path != "") {
		ImGui::Text(m_path.c_str());
	}
	else {
		ImGui::Text("no file selected");
	}
	
	ImGui::NewLine();

	if(m_waveformTex)
		ImGui::Image(m_waveformTex, m_waveformTex->getSize(), glm::vec2(0, 1), glm::vec2(1, 0));

	ImGui::NewLine();
	ImGui::PushItemWidth(0.6 * m_drawSize.x);
	
	if (m_path != "") {
		ImGui::SliderFloat("size", &m_sizeFeature, 0.0f, 1.0f);
		ImGui::SliderFloat("temperature", &m_temperatureFeature, 0.0f, 1.0f);
		ImGui::SliderFloat("saturation", &m_saturationFeature, 0.0f, 1.0f);
		ImGui::SliderFloat("regularity", &m_regularityFeature, 0.0f, 1.0f);
		ImGui::SliderFloat("contrast", &m_contrastFeature, 0.0f, 1.0f);
		ImGui::SliderFloat("sharpness", &m_sharpnessFeature, 0.0f, 1.0f);
	}

	endNodeDraw();
}

void act::proc::AudioInProcNode::onTrigger(bool event) {
	if (!m_isPlaying) {
		m_isPlaying = true;
		m_bufferPlayer->start();
	}
	else {
		m_isPlaying = false;
		m_bufferPlayer->stop();
	}
}

void act::proc::AudioInProcNode::loadSound(std::string path) {
	if(path != "") {
		try {
			m_buffer = ci::audio::load(ci::loadFile(path))->loadBuffer();
			m_bufferPlayer->setBuffer(m_buffer);

			auto waveform = WaveformPlot();
			waveform.load(m_buffer, ci::Rectf(glm::vec2(0, 0), m_drawSize));

			ci::gl::Fbo::Format format;
			format.setSamples( 4 );
			auto fbo = ci::gl::Fbo::create(m_drawSize.x, m_drawSize.y, format);
			{
				ci::gl::ScopedFramebuffer fbScp(fbo);
				ci::gl::ScopedViewport scpVp(glm::ivec2(0), fbo->getSize());

				ci::gl::ScopedMatrices scpMatrices;
				ci::gl::setMatricesWindow(fbo->getSize(), true);

				ci::gl::clear(util::Design::backgroundColor());
			
				waveform.draw();
			}
			m_waveformTex = fbo->getColorTexture();

			m_sharpnessFeature = 0.0f;
			m_regularityFeature	= 0.0f;
			m_sizeFeature = 0.0f;
			m_temperatureFeature = 0.0f;
			m_contrastFeature = 0.0f;
			m_saturationFeature	= 0.0f;

			m_spectrum = m_spectralNode->getMagSpectrum();

		} catch(...) {
		}
	}
}

void act::proc::AudioInProcNode::calculateFeatures() {
	
	std::vector<float> lastSpectrum = m_spectrum;
	m_spectrum = m_spectralNode->getMagSpectrum();
	m_centroid = ci::audio::dsp::spectralCentroid(m_spectrum.data(), m_spectrum.size(), ci::audio::Context::master()->getSampleRate());
	m_volume = m_volumeNode->getVolume();

	int spectrumSize = m_spectrum.size();
	bool up = true;
	float peakSum = 0.0f;
	std::vector<float> peaks;
	float peakPosSum = 0.0f;
	std::vector<int> peakPos;
	std::vector<float> scaledPos;
	int largestPeakIndex = 0;
	float lagestPeakSize = 0.0f;
	float spectralFluxSum = 0.0f;

	for (unsigned int i = 2; i < m_spectrum.size(); i++) {
		float current = m_spectrum[i];
		spectralFluxSum += powf(lastSpectrum[i] - current, 2);
		if (current < 0.0001) {
			continue;
		}
		float last = m_spectrum[i - 1];
		if (last > current && up) {
			// berechnung höchster Peak bis
			// index 170 (sind ca 2000 hz um harmonieanalyse einzuschränken)
			if (i < 170) {
				if (current > lagestPeakSize) {
					lagestPeakSize = current;
					largestPeakIndex = peaks.size();
				}
			}

			peaks.push_back(last);
			peakSum += last;

			peakPos.push_back(i-1);

			float scaledPeakPos = log2(i - 1);
			scaledPos.push_back(scaledPeakPos);
			peakPosSum += scaledPeakPos;

			up = false;
		}
		else if (last < current && !up) {
			up = true;
		}
	}
	int peakNum = peaks.size();
	float spectralFlux = spectralFluxSum;
	
	calcSize();
	calcTemperature();
	calcSaturation(peakNum, spectrumSize);
	calcContrast(scaledPos, peaks, peakPosSum);
	calcRegularity(spectralFlux);
	calcSharpness(peaks, peakPos, largestPeakIndex);
}

void act::proc::AudioInProcNode::calcRegularity(float spectralFlux) {
	float scaledFlux = spectralFlux * 100;
	if (scaledFlux > 1.0f) {
		m_regularityFeature = 0.0f;
	}
	else {
		m_regularityFeature = 1 - scaledFlux;
	}
}

void act::proc::AudioInProcNode::calcSize() {
	m_sizeFeature = m_volume;
}

void act::proc::AudioInProcNode::calcTemperature() {
	if (m_centroid >= UPPER_HZ) {
		m_temperatureFeature = 0.0f;
	}
	else if (m_centroid <= LOWER_HZ) {
		m_temperatureFeature = 1.0f;
	}
	else {
		m_temperatureFeature = 1.0f - (A * log2(B * m_centroid));
	}
}

void act::proc::AudioInProcNode::calcContrast(std::vector<float> peakPos, std::vector<float> peaks, float peakPosSum) {
	int peakNum = peakPos.size();
	if (peakNum <= 1) {
		m_contrastFeature = 0.0f;
	} 
	else {
		float peakPosMean = peakPosSum / peakNum;
		float peakPosDivSum = 0.0f;
		float ampSum = 0.0f;

		for (unsigned int i = 0; i < peakNum; i++) {
			float amp = powf(peaks[i], 2);
			peakPosDivSum += powf((peakPos[i] - peakPosMean), 2) * amp;
			ampSum += amp;
		}

		float peakPosDiv = (peakPosDivSum / ampSum) / peakNum;
		if (peakPosDiv > 1.0f) {
			m_contrastFeature = 1.0f;
		}
		else {
			m_contrastFeature = peakPosDiv;
		}
	}	
}

void act::proc::AudioInProcNode::calcSaturation(int peakNum, int spectrumSize) {
	float upperLimit = spectrumSize / 6.0f;
	if (peakNum == 0) {
		m_saturationFeature = 0.0f;
	}
	else if (peakNum > upperLimit) {
		m_saturationFeature = 1.0f;
	}
	else {
		m_saturationFeature = peakNum / upperLimit;
	}
}

void act::proc::AudioInProcNode::calcSharpness(std::vector<float> peakSize, std::vector<int> peakPos, int startPeakIndex) {
	float peakNum = peakPos.size();
	if (peakNum <= 1) {
		m_sharpnessFeature = 0.0f;
		return;
	}
	int f0 = peakPos[startPeakIndex];
	int lastVisitedPos = startPeakIndex-1;
	float diviation = 0;
	float ampSum = 0;
	bool noPeaksLeft = false;
	for (unsigned int h = 1; h < 13; h++) {
		int fH = f0 * h;
		int actualH = 0;
		for (unsigned int i = lastVisitedPos + 1; i < peakNum; i++) {
			if (i == peakNum - 1) {
				noPeaksLeft = true;
				break;
			}
			else if (peakPos[i] >= fH + f0) {
				break;
			}
			else if (peakPos[i] >= fH) {
				actualH = peakPos[i];
				lastVisitedPos = i;
				float ampH = powf(peakSize[lastVisitedPos], 2);
				diviation += abs(actualH - fH) * ampH;
				ampSum += ampH;
				break;
			}
		}
		if (noPeaksLeft) {
			break;
		}
	}

	if (diviation == 0) {
		m_sharpnessFeature = 0.0f;
	}
	else {
		float inHarmonie = (2.0 / f0) * (diviation / ampSum);
		if (inHarmonie > 1.0f) {
			m_sharpnessFeature = 1.0f;
		}
		else {
			m_sharpnessFeature = inHarmonie;
		}	
	}
}

ci::Json act::proc::AudioInProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["path"] = m_path;
	return json;
}

void act::proc::AudioInProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "path", m_path);
	init();
}

act::proc::OutputPortRef<float> act::proc::AudioInProcNode::createOutputPort(std::string featureName)
{
	auto port = createNumberOutput(featureName);
	return port;
}
