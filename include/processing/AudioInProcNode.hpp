
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

#pragma once

#include "ProcNodeBase.hpp"

#include "cinder/audio/audio.h"
#include "../3rd/AudioDrawUtils.h"

#include "cinder/Timeline.h"


namespace act {
	namespace proc {

		class AudioInProcNode : public ProcNodeBase
		{
		public:
			AudioInProcNode();
			~AudioInProcNode();

			PROCNODECREATE(AudioInProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onTrigger(bool event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			void init();

			std::string							m_path;
			ci::audio::BufferRef				m_buffer;
			ci::audio::BufferPlayerNodeRef		m_bufferPlayer;
			ci::audio::MonitorSpectralNodeRef	m_spectralNode;
			ci::audio::MonitorNodeRef			m_volumeNode;

			std::vector<float>					m_spectrum;
			number								m_centroid;
			number								m_volume;

			float								m_sharpnessFeature;
			float								m_regularityFeature;
			float								m_sizeFeature;
			float								m_temperatureFeature;
			float								m_contrastFeature;
			float								m_saturationFeature;


			WaveformPlot						m_waveform;
			ci::gl::Texture2dRef					m_waveformTex;

			OutputPortRef<ci::audio::NodeRef>		m_audioNodePort;
			
			// OutputPortRef<float>				m_arousalPort;
			// OutputPortRef<float>				m_speedPort;
			// OutputPortRef<float>				m_dynamicPort;
			// OutputPortRef<float>				m_linearityPort;
			// OutputPortRef<float>				m_periodicityPort;
			// OutputPortRef<float>				m_frequencyPort;
			// OutputPortRef<float>				m_structurePort;
			// OutputPortRef<float>				m_clarityPort;
			// OutputPortRef<float>				m_complexityPort;
			OutputPortRef<float>				m_sharpnessPort;
			OutputPortRef<float>				m_regularityPort;
			// OutputPortRef<float>				m_orientationPort;
			// OutputPortRef<float>				m_granularityPort;
			// OutputPortRef<float>				m_hierarchyPort;
			// OutputPortRef<float>				m_dominancePort;
			OutputPortRef<float>				m_sizePort;
			// OutputPortRef<float>				m_intensityPort;
			OutputPortRef<float>				m_temperaturePort;
			OutputPortRef<float>				m_contrastPort;
			OutputPortRef<float>				m_saturationPort;
			

			void loadSound(std::string path);

			bool m_isOpenDialog;
			bool m_isPlaying;

			OutputPortRef<float> createOutputPort(std::string featureName);

			void calculateFeatures();

			void calcRegularity(float spectralFlux);
			void calcSize();
			void calcTemperature();
			void calcContrast(std::vector<float> peakPos, std::vector<float> peaks, float peakPosSum);
			void calcSaturation(int peakNum, int spectrumSize);
			void calcSharpness(std::vector<float> peaksSize, std::vector<int> peakPos, int startPeakIndex);
		};

		using AudioInProcNodeRef = std::shared_ptr<AudioInProcNode>;

	}
}