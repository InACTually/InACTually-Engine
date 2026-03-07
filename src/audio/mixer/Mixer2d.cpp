
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


#include "opencv2/imgproc.hpp"
#include <functional>
#include <vector>
#include <numeric>
#include <algorithm>
#include <glm/glm.hpp>
#include "mixer/Mixer2d.hpp"
#include "iostream"

using namespace ci;

act::aio::Mixer2d::Mixer2d()
	: MixerBase()
{
	m_maxDistance = 1.0f;
	m_pi = (float)M_PI;
}

act::aio::Mixer2d::~Mixer2d()
{
}

void act::aio::Mixer2d::update()
{
	updateCentroid();
	updateMaxDistance();
	updatePanningPairs();
	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;
		vec2 soundPos = { sound.get()->getPosition().x, sound.get()->getPosition().z };

		float mixPan, mixCenterRim;
		act::room::SpeakerRoomNodeRef pan1{};
		act::room::SpeakerRoomNodeRef pan2{};
		std::optional<PanningPair> currentPan{};
		bool foundOutside{ false };

		// get the pair of speakers closest to the sounds position
		auto activePan = std::find_if(begin(m_panningPairs), end(m_panningPairs),
			[soundPos](const PanningPair& panPair) {
				return panPair.collider.contains(soundPos);
			});

		if (activePan != end(m_panningPairs)) {
			currentPan = *activePan;
		}
		else {
			ci::vec2 scaledSoundPos = m_centroid + (soundPos - m_centroid) * 0.5f * m_maxDistance / length(soundPos - m_centroid);
			activePan = std::find_if(begin(m_panningPairs), end(m_panningPairs),
				[scaledSoundPos](const PanningPair& panPair) {
					return panPair.collider.contains(scaledSoundPos);
				});

			if (activePan != end(m_panningPairs)) {
				currentPan = *activePan;
				foundOutside = true;
			}
			else if (m_lastPanningPair.has_value()) {
				currentPan = m_lastPanningPair;
			}
		}

		if (currentPan.has_value()) {
			m_lastPanningPair = currentPan;
			pan1 = currentPan->speaker1;
			pan2 = currentPan->speaker2;
			vec2 speaker1Pos{ pan1.get()->getPosition().x, pan1.get()->getPosition().z };
			vec2 speaker2Pos{ pan2.get()->getPosition().x, pan2.get()->getPosition().z };
			vec2 panPos = calculateIntersection(speaker1Pos, speaker2Pos, m_centroid, soundPos);
			mixCenterRim = foundOutside  ? 1.0f : std::clamp(length(soundPos - m_centroid) / length(panPos - m_centroid), 0.0f, 1.0f);
			mixPan = std::clamp(length(panPos - speaker1Pos) / length(speaker2Pos - speaker1Pos), 0.0f, 1.0f);
		}
		else {
			mixPan = 0.0f;
			mixCenterRim = 0.0f;
		}
		if (ci::length(soundPos - m_centroid) < 0.0001f) {
			mixPan = 0.0f;
			mixCenterRim = 0.0f;
		}
		float numSpeakersInv = sqrt(1.0f / m_speakers.size());

		for (auto&& speaker : m_speakers) {
			
			vec2 speakerPos{ speaker->getPosition().x, speaker->getPosition().z };
			float distance = length(speakerPos - m_centroid);
			float unitGain = calculateUnitGain(distance);
			float gain{};
			if (speaker == pan1) {
				gain = unitGain * (cosf(mixPan * m_pi * 0.5f) * sinf(mixCenterRim * m_pi * 0.5f) + cosf(mixCenterRim * m_pi * 0.5f) * numSpeakersInv);
			}
			else if (speaker == pan2) {
				gain = unitGain * (sinf(mixPan * m_pi * 0.5f) * sinf(mixCenterRim * m_pi * 0.5f) + cosf(mixCenterRim * m_pi * 0.5f) * numSpeakersInv);
			}
			else {
				gain = unitGain * cosf(mixCenterRim * m_pi * 0.5f) * numSpeakersInv;
			}
			if (gain < 0.0f)
				gain = 0.0f;
			else if (gain > 1.0f)
				gain = 1.0f;

			auto gainNode = pair.second[speaker->getUID()];
			if (gainNode && gainNode->getParam()->getNumEvents() == 0)
				gainNode->getParam()->applyRamp(gain, 0.01f);
		}
	}
}

void act::aio::Mixer2d::clear()
{
	for (auto&& speaker : m_speakers) 
	{
		speaker->disconnectExternals();
	}
	m_speakers.clear();

	for (auto&& pair : m_mixMap) 
	{
		auto&& sound = pair.first;
		sound->disconnectExternals();
	}
	m_mixMap.clear();
	m_mixMap = std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>();

	m_panningPairs.clear();
	m_centroid = { 0.0f, 0.0f };

	m_maxDistance = 1.0f;
}

void act::aio::Mixer2d::configure(std::vector<act::room::SpeakerRoomNodeRef> speakers
	, std::vector<act::room::SubwooferRoomNodeRef> subwoofers
	, std::vector<act::room::SoundRoomNodeRef> sounds)
{
	clear();


	for (auto&& sound : sounds) {
		sound->disconnectExternals();
	}


	auto ctx = audio::Context::master();
	m_channelRouterNode = ctx->makeNode(new audio::ChannelRouterNode(audio::Node::Format()
		.channels(ctx->getOutput()->getNumChannels())));

	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			speaker->getOut() >> m_channelRouterNode->route(0, speaker->getChannel()) >> ctx->getOutput();
			m_speakers.push_back(speaker);
		}
	}

	for (auto&& sound : sounds) {
		connectSound(sound, speakers, subwoofers);
	}

	ctx->enable();
}

void act::aio::Mixer2d::connectSound(act::room::SoundRoomNodeRef sound
	, std::vector<act::room::SpeakerRoomNodeRef> speakers
	, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
{
	m_mixMap[sound] = std::map<act::UID, ci::audio::GainNodeRef>();
	auto ctx = audio::Context::master();
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			auto gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(0.0f));
			m_mixMap[sound][speaker->getUID()] = gain;
			sound->getOut() >> gain >> speaker->getIn();
		}
	}
}

void act::aio::Mixer2d::updatePanningPairs() {
	// Prepare vector of speakerpositions in opencv format, so we can use it's convex hull routine
	m_panningPairs.clear();

	if (m_speakers.size() == 0)
		return;

	std::vector<cv::Vec2f> speakerPositions;
	for (auto&& speaker : m_speakers) {
		auto pos = speaker.get()->getPosition();
		speakerPositions.push_back({ pos.x, pos.z });
	}
	std::vector<int> convexHullIndices;
	cv::convexHull(speakerPositions, convexHullIndices);

	// construct the m_panningPairs based on the convex hull
	for (int i{ 0 }; i < (convexHullIndices.size()); ++i) {
		auto speaker1index{ convexHullIndices.at(i) };
		auto speaker2index{ convexHullIndices.at((i + 1) % convexHullIndices.size()) };
		// convert speaker positions to cinder vec2s
		vec2 speakerPos1 = {
			speakerPositions.at(speaker1index)[0],
			speakerPositions.at(speaker1index)[1],
		};
		vec2 speakerPos2 = {
			speakerPositions.at(speaker2index)[0],
			speakerPositions.at(speaker2index)[1],
		};

		cinder::Shape2d collider{};
		collider.moveTo(m_centroid);
		collider.lineTo(speakerPos1);
		collider.lineTo(speakerPos2);
		collider.close();
		m_panningPairs.push_back({ std::move(collider)
			, m_speakers.at(speaker1index)
			, m_speakers.at(speaker2index) 
			}
		);
	}
}

void act::aio::Mixer2d::updateMaxDistance() {
	m_maxDistance = 0.0f;
	for (auto&& speaker : m_speakers) {
		vec2 speakerPos { speaker->getPosition().x, speaker->getPosition().z };
		float length = ci::length(speakerPos - m_centroid);
		if (length > m_maxDistance) {
			m_maxDistance = length;
		}
	}
}

void act::aio::Mixer2d::updateCentroid() {
	std::vector<glm::vec2> speakerPositions;
	for (auto&& speaker : m_speakers) {
		auto pos = speaker.get()->getPosition();
		speakerPositions.push_back({ pos.x, pos.z });
	}
	glm::vec2 centroid = std::accumulate(speakerPositions.begin(), speakerPositions.end(), glm::vec2{});
	centroid /= static_cast<float>(speakerPositions.size());
	m_centroid = centroid;
}

glm::vec2 act::aio::Mixer2d::calculateIntersection(vec2 p1, vec2 p2, vec2 p3, vec2 p4) {
	float determinant{ (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x) };
	float x{ ((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x))
			/ determinant };
	float y{ ((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x))
		/ determinant };
	return { x, y };
}

float act::aio::Mixer2d::calculatePanningFactor(vec2 longVec, vec2 shortVec) 
{
	return length(shortVec) / length(longVec);
}

float act::aio::Mixer2d::calculateUnitGain(float distance) 
{
	return audio::decibelToLinear(100 - 20 * log10f(m_maxDistance / distance));
}