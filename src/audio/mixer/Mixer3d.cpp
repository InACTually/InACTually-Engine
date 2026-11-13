
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "mixer/Mixer3d.hpp"
#define CONVHULL_3D_ENABLE
#include "convhull_3d/convhull_3d.h"
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <limits>
#include <opencv2/imgproc.hpp>

act::aio::Mixer3d::Mixer3d()
	: MixerBase()
{
	m_maxDistance = 1.0f;
}

act::aio::Mixer3d::~Mixer3d()
{
}

void act::aio::Mixer3d::update()
{
	updateColliders();
	updateMaxDistance();
	float numSpeakersInvTotal = sqrt(1.0f / m_speakers.size());
	auto clamp = [](float val) { return std::clamp(val, 0.0f, 1.0f); };

	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;
		glm::vec3 soundPos = sound.get()->getPosition();
		glm::vec3 scaledSoundPos = m_centroid + ci::normalize(soundPos - m_centroid) * 0.5f * m_minDistance;

		Collider collider = findCollider(scaledSoundPos);
		// case of merged face
		if (collider.mergedFace.has_value()) {
			glm::vec3 speaker1Pos{ collider.speakers[0]->getPosition() };
			glm::vec3 speaker2Pos{ collider.speakers[1]->getPosition() };
			// point of intersection between ray from centroid through sound and outer face
			glm::vec3 planeIntersection{ soundPos != m_centroid ? calculatePlaneIntersection(collider.normal,
				speaker1Pos,soundPos) : m_centroid};
			// tangent vector of the active panning face
			glm::vec3 tangent { ci::normalize(speaker2Pos - speaker1Pos) };
			// bitangent vector of the active panning face
			glm::vec3 bitangent{ ci::cross(collider.normal, tangent) };

			// position of speaker 1 in tangent space
			glm::vec2 speaker2D1{ ci::dot(speaker1Pos, tangent), ci::dot(speaker1Pos, bitangent) };
			// position of speaker 2 in tangent space
			glm::vec2 speaker2D2{ ci::dot(speaker2Pos, tangent), ci::dot(speaker2Pos, bitangent) };
			// position of the face centroid in tangent space
			glm::vec2 faceCentroid2D{ ci::dot(collider.mergedFace.value().planeCentroid, tangent),
									 ci::dot(collider.mergedFace.value().planeCentroid, bitangent) };
			// position of pleneIntersection in tangent space
			glm::vec2 sound2D{ ci::dot(planeIntersection, tangent), ci::dot(planeIntersection, bitangent) };
			// position of pan in tangent space
			glm::vec2 pan2D{ calculateIntersection(speaker2D1, speaker2D2, faceCentroid2D, sound2D) };

			float mixPan{ faceCentroid2D != sound2D ? clamp(glm::length(pan2D - speaker2D1) / glm::length(speaker2D2 - speaker2D1)) : 0.0f};
			float mixCenterRimFace{ faceCentroid2D != sound2D ? clamp(glm::length(sound2D - faceCentroid2D) / glm::length(pan2D - faceCentroid2D)) : 0.0f};
			float mixCenterRimVolume{ planeIntersection != m_centroid ? clamp(glm::length(soundPos - m_centroid) / glm::length(planeIntersection - m_centroid)) : 0.0f};

			float numSpeakersInvFace = sqrt(1.0f / collider.mergedFace->speakers.size());

			for (auto&& speaker : m_speakers) {
				float distance = length(speaker->getPosition() - m_centroid);
				float unitGain = calculateUnitGain(distance);
				float gain{};
				if (speaker == collider.speakers[0]) {
					gain = unitGain * (sqrt(1.0f - mixPan) * sqrt(mixCenterRimFace) * sqrt(mixCenterRimVolume)
						+ sqrt(1.0f - mixCenterRimFace) * sqrt(mixCenterRimVolume) * numSpeakersInvFace
						+ sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else if (speaker == collider.speakers[1]) {
					gain = unitGain * (sqrt(mixPan) * sqrt(mixCenterRimFace) * sqrt(mixCenterRimVolume)
						+ sqrt(1.0f - mixCenterRimFace) * sqrt(mixCenterRimVolume) * numSpeakersInvFace
						+ sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else if (find(begin(collider.mergedFace->speakers)
						, end(collider.mergedFace->speakers), speaker) != end(collider.mergedFace->speakers)) {
					gain = unitGain * (sqrt(1.0f - mixCenterRimFace) * sqrt(mixCenterRimVolume) * numSpeakersInvFace
						+ sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else {
					gain = unitGain * (sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				gain = std::clamp(gain, 0.0f, 1.0f);
				auto gainNode = pair.second[speaker->getUID()];
				if (gainNode && gainNode->getParam()->getNumEvents() == 0)
					gainNode->getParam()->applyRamp(gain, 0.01f);
			}

		} // case of simple face
		else if (collider.speakers.size() == 3) {
			glm::vec3 speaker1Pos{ collider.speakers[0]->getPosition() };
			glm::vec3 speaker2Pos{ collider.speakers[1]->getPosition() };
			glm::vec3 speaker3Pos{ collider.speakers[2]->getPosition() };
			// point of intersection between ray from centroid through sound and outer face
			glm::vec3 planeIntersection{ calculatePlaneIntersection(collider.normal,
				speaker1Pos, soundPos) };
			// tangent vector of the active panning face
			glm::vec3 tangent{ ci::normalize(speaker2Pos - speaker1Pos) };
			// bitangent vector of the active panning face
			glm::vec3 bitangent{ ci::cross(collider.normal, tangent) };

			// position of speaker 1 in tangent space
			glm::vec2 speaker2D1{ ci::dot(speaker1Pos, tangent), ci::dot(speaker1Pos, bitangent) };
			// position of speaker 2 in tangent space
			glm::vec2 speaker2D2{ ci::dot(speaker2Pos, tangent), ci::dot(speaker2Pos, bitangent) };
			// position of speaker 3 in tangent space
			glm::vec2 speaker2D3{ ci::dot(speaker3Pos, tangent), ci::dot(speaker3Pos, bitangent) };
			// position of planeIntersection in tangent space
			glm::vec2 sound2D{ ci::dot(planeIntersection, tangent), ci::dot(planeIntersection, bitangent) };

			cv::Matx22f mat(
				speaker2D1.x - speaker2D3.x, speaker2D2.x - speaker2D3.x,
				speaker2D1.y - speaker2D3.y, speaker2D2.y - speaker2D3.y);

			cv::Vec2f vec(
				sound2D.x - speaker2D3.x,
				sound2D.y - speaker2D3.y);

			cv::Vec2f sol;
			cv::solve(mat, vec, sol);
			float u1{ clamp(sol[0]) };
			float u2{ clamp(sol[1]) };
			float u3{ clamp(1.0f - u1 - u2) };
			float mixCenterRimVolume{ clamp(glm::length(soundPos - m_centroid) / glm::length(planeIntersection - m_centroid)) };

			for (auto&& speaker : m_speakers) {
				float distance = length(speaker->getPosition() - m_centroid);
				float unitGain = calculateUnitGain(distance);
				float gain{};
				if (speaker == collider.speakers[0]) {
					gain = unitGain * (sqrt(u1)*sqrt(mixCenterRimVolume) + sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else if (speaker == collider.speakers[1]) {
					gain = unitGain * (sqrt(u2)*sqrt(mixCenterRimVolume) + sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else if (speaker == collider.speakers[2]) {
					gain = unitGain * (sqrt(u3)*sqrt(mixCenterRimVolume) + sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal);
				}
				else {
					gain = unitGain * sqrt(1.0f - mixCenterRimVolume) * numSpeakersInvTotal;
				}
				gain = std::clamp(gain, 0.0f, 1.0f);
				auto gainNode = pair.second[speaker->getUID()];
				if (gainNode && gainNode->getParam()->getNumEvents() == 0)
					gainNode->getParam()->applyRamp(gain, 0.01f);
			}


		} // case of no found collider
		else {
			for (auto&& speaker : m_speakers) {
				float distance = length(speaker->getPosition() - m_centroid);
				float unitGain = calculateUnitGain(distance);
				float gain{};
				gain = unitGain * numSpeakersInvTotal;
				auto gainNode = pair.second[speaker->getUID()];
				if (gainNode && gainNode->getParam()->getNumEvents() == 0)
					gainNode->getParam()->applyRamp(gain, 0.01f);
			}
		}
	}
}

void act::aio::Mixer3d::clear()
{
	for (auto&& speaker : m_speakers) {
		speaker->disconnectExternals();
	}
	m_speakers.clear();

	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;
		sound->disconnectExternals();
	}
	m_mixMap.clear();
	m_mixMap = std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>();

	m_colliders.clear();
	m_centroid = { 0.0f, 0.0f, 0.0f };

	m_maxDistance = 1.0f;
}

void act::aio::Mixer3d::configure(std::vector<act::room::SpeakerRoomNodeRef> speakers
	, std::vector<act::room::SubwooferRoomNodeRef> subwoofers, std::vector<act::room::SoundRoomNodeRef> sounds)
{
	clear();
	m_speakers = speakers;

	for (auto&& sound : sounds) {
		sound->disconnectExternals();
	}


	auto ctx = ci::audio::Context::master();
	m_channelRouterNode = ctx->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format()
		.channels(ctx->getOutput()->getNumChannels())));

	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			speaker->getOut() >> m_channelRouterNode->route(0, speaker->getChannel()) >> ctx->getOutput();
		}
	}

	for (auto&& subwoofer : subwoofers) {
		if (subwoofer->getChannel() < ctx->getOutput()->getNumChannels()) {
			subwoofer->getOut() >> m_channelRouterNode->route(0, subwoofer->getChannel());
		}
	}

	for (auto&& sound : sounds) {
		connectSound(sound, speakers, subwoofers);
	}

	ctx->enable();
}

void act::aio::Mixer3d::connectSound(act::room::SoundRoomNodeRef sound, std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
{
	m_mixMap[sound] = std::map<act::UID, ci::audio::GainNodeRef>();
	auto ctx = ci::audio::Context::master();
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			auto gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(0.0f));
			m_mixMap[sound][speaker->getUID()] = gain;
			sound->getOut() >> gain >> speaker->getIn();
		}
	}
	for (auto&& subwoofer : subwoofers) {
		if (subwoofer->getChannel() < ctx->getOutput()->getNumChannels()) {
			auto gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(1.0f));
			auto lowPass = ci::audio::Context::master()->makeNode(new ci::audio::FilterLowPassNode);
			lowPass->setCutoffFreq(150.f);
			m_mixMap[sound][subwoofer->getUID()] = gain;
			sound->getOut() >> gain >> lowPass >> subwoofer->getIn();
		}
	}
}

bool act::aio::Mixer3d::calculate_intersection_line_plane(glm::vec3 p0, glm::vec3 p1, ci::vec4 plane, glm::vec3& intersection, float epsilon)
{
	glm::vec3 normal { plane.x, plane.y, plane.z };
	auto u = p1 - p0;
	auto dot = ci::dot(normal, u);

	if (abs(dot) > epsilon) {
		auto p_onPlane = -normal * plane.w;

		auto w = p0 - p_onPlane;
		auto factor = -ci::dot(normal, w) / dot;
		u = u * factor;
		intersection = p0 + u;
		return true;
	}
	else return false;
}

void act::aio::Mixer3d::updateColliders()
{
	// clear colliders beforehand
	m_colliders.clear();

	std::vector<ch_vertex> vertices{};

	for (auto&& speaker : m_speakers) {
		auto pos{ speaker->getPosition() };
		vertices.push_back({ pos.x, pos.y, pos.z });
	}

	// calculate centroid
	m_centroid = { 0.0f, 0.0f, 0.0f };
	for (auto&& vert : vertices) {
		glm::vec3 p = { vert.x, vert.y, vert.z };
		m_centroid += p;
	}
	m_centroid /= float(vertices.size());

	int* faces;
	int numFaces{};
	convhull_3d_build(vertices.data(), (int)m_speakers.size(), &faces, &numFaces);

	// list of tuples containing the indices per face
	std::vector<std::tuple<int, int, int>> faceList{};
	for (int i{ 0 }; i < numFaces; ++i) {
		faceList.push_back({ faces[i * 3], faces[i * 3 + 1], faces[i * 3 + 2] });
	}

	// Find adjacent faces in save them as index pairs in adjacencies
	std::vector<std::pair<int, int>> adjacencies{};
	for (int i{ 0 }; i < numFaces; ++i) {
		for (int j = i; j < numFaces; ++j) {
			int countCommonVerts{ 0 };
			// check all pairs of points for both faces for equality
			for (int f1{ 0 }; f1 < 3; ++f1) {
				for (int f2{ 0 }; f2 < 3; ++f2) {
					if (faces[i * 3 + f1] == faces[j * 3 + f2]) {
						++countCommonVerts;
					}
				}
			}
			if (countCommonVerts == 2) {
				adjacencies.push_back({ i,j });
			}
		}
	}

	free(faces);

	// Calculate normals
	std::vector<glm::vec3> normals{};
	for (auto&& face : faceList) {
		auto& [i0, i1, i2] = face;
		auto v0 = vertices[i0];
		auto v1 = vertices[i1];
		auto v2 = vertices[i2];
		glm::vec3 p0 = { v0.x, v0.y, v0.z };
		glm::vec3 p1 = { v1.x, v1.y, v1.z };
		glm::vec3 p2 = { v2.x, v2.y, v2.z };
		glm::vec3 normal = ci::normalize(ci::cross(p1 - p0, p2 - p0));
		if (ci::dot(normal, p0 - m_centroid) < 0.0f) {
			normal = -normal;
		}
		normals.push_back(normal);
	}

	// Calculate least distance to the convex hull
	m_minDistance = std::numeric_limits<float>::max();
	for (int i = 0; i < normals.size(); ++i) {
		m_minDistance = std::min(m_minDistance, glm::length(ci::dot(normals[i], m_speakers[std::get<0>(faceList[i])]->getPosition() - m_centroid)));
	}

	// Find faces to merge
	std::vector<std::set<int>> mergedFaces{};
	for (auto&& adj : adjacencies) {
		float angle = acosf(ci::dot(normals[adj.first], normals[adj.second])) * 180.f / (float)M_PI;
		if (angle < 10.f) {
			bool foundSet{ false };
			for (auto&& faceSet : mergedFaces) {
				if (faceSet.contains(adj.first) || faceSet.contains(adj.second)) {
					faceSet.insert(adj.first);
					faceSet.insert(adj.second);
					foundSet = true;
					break;
				}
			}
			if (!foundSet) {
				mergedFaces.push_back({ adj.first, adj.second });
			}
			// handle case of first insertion if they should be merged
			if (mergedFaces.size() == 0) {
				mergedFaces.push_back({ adj.first, adj.second });
			}
		}
	}

	// create colliders for merged faces
	for (auto&& currentMergedFace : mergedFaces) {
		// contains all indices of speakers belonging to a merged face
		std::set<int> speakers;

		glm::vec3 normal, tangent;
		for (auto face : currentMergedFace) {
			auto [p0, p1, p2]  { faceList[face] };
			auto vert0{ vertices[p0] };
			auto vert1{ vertices[p1] };
			normal = normals[face];
			tangent = {vert1.x-vert0.x, vert1.y - vert0.y, vert1.z - vert0.z };
			tangent = ci::normalize(tangent);
			speakers.insert(p0);
			speakers.insert(p1);
			speakers.insert(p2);
		}
		glm::vec3 bitangent{ ci::cross(normal, tangent) };
		// vector containing indices of speakers belonging to merged face
		std::vector<int> speakerList(begin(speakers), end(speakers));

		std::vector<cv::Vec2f> transformedPositions;
		for (auto speakerIndex : speakers) {
			auto pos = vertices[speakerIndex];
			glm::vec3 position = { pos.x, pos.y, pos.z };
			transformedPositions.push_back({ ci::dot(position,tangent), ci::dot(position, bitangent) });
		}
		std::vector<int> convexHullIndices;
		cv::convexHull(transformedPositions, convexHullIndices);

		glm::vec3 face_centroid = { 0.0f, 0.0f, 0.0f };
		// calculate centroid of mergedFace
		for (auto speaker : speakers) {
			auto vert = vertices[speaker];
			face_centroid += glm::vec3{vert.x, vert.y, vert.z};
		}
		face_centroid /= static_cast<float>(speakers.size());

		MergedFace mergedFace = { face_centroid, {} };
		for (auto speaker : speakers) {
			mergedFace.speakers.push_back(m_speakers[speaker]);
		}

		// add a collider for every edge in the convex hull
		for (int i{ 0 }; i < convexHullIndices.size(); ++i) {
			Collider collider{ {}, mergedFace, normals[*begin(currentMergedFace)]};
			collider.speakers.push_back(m_speakers[speakerList[convexHullIndices[i]]]);
			collider.speakers.push_back(m_speakers[speakerList[convexHullIndices[(i+1)%convexHullIndices.size()]]]);
			m_colliders.push_back(collider);
		}
	}
	

	// create colliders for non-merged faces
	
	// vector of indices of faces that aren't merged
	std::vector<int> nonMergedFaces{};
	for (int i = 0; i < faceList.size(); ++i) {
		nonMergedFaces.push_back(i);
	}
	for (auto&& mergedFace : mergedFaces) {
		std::vector<int> tempNonMerged{};
		std::set_difference(begin(nonMergedFaces), end(nonMergedFaces),
			begin(mergedFace), end(mergedFace), std::inserter(tempNonMerged, begin(tempNonMerged)));
		nonMergedFaces = tempNonMerged;
	}

	for (auto face : nonMergedFaces) {
		Collider collider{ {}, std::nullopt, normals[face]};
		auto [i0, i1, i2] {faceList[face]};
		collider.speakers.push_back(m_speakers[i0]);
		collider.speakers.push_back(m_speakers[i1]);
		collider.speakers.push_back(m_speakers[i2]);
		m_colliders.push_back(collider);
	}
	std::cout << "okay";
}

act::aio::Mixer3d::Collider act::aio::Mixer3d::findCollider(glm::vec3 soundPos)
{
	for (auto&& collider : m_colliders) {
		glm::vec3 p0, p1, p2, p3;
		// case for colliders of merged faces
		if (collider.mergedFace.has_value()) {
			p0 = collider.speakers[0].get()->getPosition();
			p1 = collider.speakers[1].get()->getPosition();
			p2 = m_centroid;
			p3 = collider.mergedFace.value().planeCentroid;
		} // case for colliders of simple faces
		else {
			p0 = collider.speakers[0].get()->getPosition();
			p1 = collider.speakers[1].get()->getPosition();
			p2 = collider.speakers[2].get()->getPosition();
			p3 = m_centroid;
		}
		cv::Matx33f mat(
			p1.x - p0.x, p2.x - p0.x, p3.x - p0.x,
			p1.y - p0.y, p2.y - p0.y, p3.y - p0.y,
			p1.z - p0.z, p2.z - p0.z, p3.z - p0.z
		);

		cv::Vec3f vec(
			soundPos.x - p0.x,
			soundPos.y - p0.y,
			soundPos.z - p0.z
		);

		cv::Vec3f sol;

		cv::solve(mat, vec, sol);

		std::vector<float> us;
		sol.col(0).copyTo(us);
		us.push_back(1.0f - (float)cv::sum(sol)[0]);

		if (*std::min_element(begin(us),end(us)) >= 0.0f) { return collider; }
	}
	return Collider{ {}, std::nullopt };
}

glm::vec3 act::aio::Mixer3d::calculatePlaneIntersection(const glm::vec3& normal, const glm::vec3& planePoint, const glm::vec3& soundPos)
{
	glm::vec3 l{ soundPos - m_centroid };
	float d{ ci::dot(planePoint - m_centroid, normal)
			/ ci::dot(l,normal) };
	return m_centroid + d*l;
}

void act::aio::Mixer3d::updateMaxDistance() {
	m_maxDistance = 0.0f;
	for (auto&& speaker : m_speakers) {
		glm::vec3 speakerPos{ speaker->getPosition().x, speaker->getPosition().y, speaker->getPosition().z };
		float length = glm::length(speakerPos - m_centroid);
		if (length > m_maxDistance) {
			m_maxDistance = length;
		}
	}
}

glm::vec2 act::aio::Mixer3d::calculateIntersection(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	float determinant{ (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x) };
	float x{ ((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x))
			/ determinant };
	float y{ ((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x))
		/ determinant };
	return { x, y };
}

float act::aio::Mixer3d::calculateUnitGain(float distance)
{
	return ci::audio::decibelToLinear(100 - 20 * log10f(m_maxDistance / distance));
}
