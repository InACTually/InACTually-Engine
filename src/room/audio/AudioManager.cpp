
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "audio/AudioManager.hpp"

#include "audio/SpeakerRoomNode.hpp"
#include "audio/SubwooferRoomNode.hpp"
#include "audio/HeadphoneRoomNode.hpp"

#include "cinder/audio/Context.h"

act::room::AudioManager::AudioManager()
	: RoomNodeManagerBase("audioManager")
{
	m_samplerate = act::Settings::get().samplerate;

	m_audioDeviceMgr = aio::AudioDeviceManager::create();
	m_audioDeviceMgr->addListener(this);

	m_mixerMgr = aio::MixerManager::create();

	m_selectedInputChannel = 0;
	m_selectedOutputChannel = 0;
	m_selectedTypeIndex = 0;

	refreshLists();
}

act::room::AudioManager::~AudioManager()
{
}

void act::room::AudioManager::setup()
{
	m_audioDeviceMgr->init();
}

void act::room::AudioManager::update()
{
	m_mixerMgr->update();
}

act::room::SoundRoomNodeRef act::room::AudioManager::createSound(vec3 position, float radius, std::string name)
{
	auto sound = room::SoundRoomNode::create(position, radius, name);
	m_sounds.push_back(sound);
	m_nodes.push_back(sound);
	m_mixerMgr->connectSound(sound);
	return sound;
}

act::room::SoundFileRoomNodeRef act::room::AudioManager::createSoundFile(vec3 position, std::filesystem::path path, float radius, std::string name, bool noTimestretch)
{
	auto sound = room::SoundFileRoomNode::create(position, path, radius, name, noTimestretch);
	m_sounds.push_back(sound);
	m_nodes.push_back(sound);
	m_mixerMgr->connectSound(sound);
	return sound;
}

act::room::SpeakerRoomNodeRef act::room::AudioManager::addSpeaker(int channel)
{
	auto speaker = SpeakerRoomNode::create(m_selectedOutputChannel, vec3(0.0f, 1.0f, 0.0f), 0.15f);
	m_speakers.push_back(speaker);
	m_nodes.push_back(speaker);

	m_mixerMgr->configure(m_speakers, m_subwoofers, m_sounds);

	return speaker;
}

act::room::SubwooferRoomNodeRef act::room::AudioManager::addSubwoofer(int channel)
{
	auto subwoofer = SubwooferRoomNode::create(m_selectedOutputChannel, vec3(0.0f, 1.0f, 0.0f), 0.3f);
	m_subwoofers.push_back(subwoofer);
	m_nodes.push_back(subwoofer);

	m_mixerMgr->configure(m_speakers, m_subwoofers, m_sounds);

	return subwoofer;
}

act::room::MicrophoneRoomNodeRef act::room::AudioManager::addMicrophone(int channel)
{
	MicrophoneRoomNodeRef mic = room::MicrophoneRoomNode::create(channel, vec3(0.0f, 1.0f, 0.0f), 0.15f);
	m_microphones.push_back(mic);
	m_nodes.push_back(mic);

	mapMicrophones();

	return mic;
}

act::room::RoomNodeBaseRef act::room::AudioManager::drawMenu()
{
	m_audioDeviceMgr->drawSettings();
	ImGui::Text("");
	m_mixerMgr->drawSettings();

	ImGui::Text("");

	if(ImGui::InputInt("Output Channel", &m_selectedOutputChannel, 1, 1)) {
		m_selectedOutputChannel = std::clamp(m_selectedOutputChannel, 0, 242);
	};

	ImGui::Combo("Type", &m_selectedTypeIndex, { "Speaker","Subwoofer" }); // ignoring Headphones

	act::room::RoomNodeBaseRef node = nullptr;

	if (ImGui::Button("add Speaker")) {
		switch (m_selectedTypeIndex) {
		case 0: // Speaker
			node = addSpeaker(m_selectedOutputChannel);
			break;
		case 1: // Subwoofer
			node = addSubwoofer(m_selectedOutputChannel);
			break;
		}
	}

	ImGui::Separator();

	if (ImGui::InputInt("Input Channel", &m_selectedInputChannel, 1, 1)) {
		m_selectedInputChannel = std::clamp(m_selectedInputChannel, 0, 242);
	};
	if (ImGui::Button("add Mic")) {
		node = addMicrophone(m_selectedInputChannel);
	}

	return node;
}

ci::Json act::room::AudioManager::toJson()
{
	auto json = ci::Json::object();

	json["audioDevice"] = m_audioDeviceMgr->toJson();

	auto mixer = ci::Json::object();
	mixer["type"] = m_mixerMgr->getMixerType();
	json["mixer"] = mixer;

	ci::Json speakers = ci::Json::array();
	for (auto&& speaker : m_speakers) {
		auto nodeJson = speaker->toJson();
		speakers.push_back(nodeJson);
	}
	json["speakers"] = speakers;

	ci::Json subwoofers = ci::Json::array();
	for (auto&& subwoofer : m_subwoofers) {
		auto nodeJson = subwoofer->toJson();
		subwoofers.push_back(nodeJson);
	}
	json["subwoofers"] = subwoofers;

	ci::Json microphones = ci::Json::array();
	for (auto&& mic : m_microphones) {
		auto nodeJson = mic->toJson();
		microphones.push_back(nodeJson);
	}
	json["microphones"] = microphones;

	return json;
}

void act::room::AudioManager::fromJson(ci::Json json)
{
	if (json.contains("speakers")) {
		auto devicesJson = json["speakers"];
		for (auto&& json : devicesJson) {
			int channel = 0;
			util::setValueFromJson(json, "channel", channel);
			auto speaker = addSpeaker(channel);

			speaker->fromJson(json);
		}
	}

	if (json.contains("subwoofers")) {
		auto devicesJson = json["subwoofers"];
		for (auto&& json : devicesJson) {
			int channel = 0;
			util::setValueFromJson(json, "channel", channel);
			auto subwoofer = addSubwoofer(channel);

			subwoofer->fromJson(json);
		}
	}

	if (json.contains("microphones")) {
		auto devicesJson = json["microphones"];
		for (auto&& json : devicesJson) {
			int channel = 0;
			util::setValueFromJson(json, "channel", channel);
			auto mic = addMicrophone(channel);

			mic->fromJson(json);
		}
	}

	if (json.contains("mixer")) {
		auto mixerJson = json["mixer"];
		int type = aio::MT_DISTANCE;
		util::setValueFromJson(mixerJson, "type", type);
		m_mixerMgr->setMixer((aio::MixerType)type);
	}

	if (json.contains("audioDevice")) {
		auto audioDeviceJson = json["audioDevice"];
		m_audioDeviceMgr->fromJson(audioDeviceJson);;
	}
}

void act::room::AudioManager::onOutputDeviceChange(ci::audio::OutputDeviceNodeRef outputDevice)
{
	m_mixerMgr->configure(m_speakers, m_subwoofers, m_sounds);
}

void act::room::AudioManager::onInputDeviceChange(ci::audio::InputDeviceNodeRef inputDevice)
{
	auto ctx = audio::Context::master();
	m_inputDeviceNode = inputDevice;
	m_inputRouter = ctx->makeNode(new audio::ChannelRouterNode(audio::Node::Format()
		.channels(m_inputDeviceNode->getNumChannels())));

	m_inputDeviceNode >> m_inputRouter;

	mapMicrophones();
}

act::room::MicrophoneRoomNodeRef act::room::AudioManager::getMicrophoneByIndex(unsigned int index)
{
	if (index >= m_microphones.size())
		return nullptr;

	return m_microphones[index];
}

act::room::MicrophoneRoomNodeRef act::room::AudioManager::getMicrophoneByUID(UID uid)
{
	room::MicrophoneRoomNodeRef mic;

	auto it = std::find_if(m_microphones.begin(), m_microphones.end(), [uid](room::MicrophoneRoomNodeRef m) { return m->getUID() == uid; });
	if (it != m_microphones.end())
		mic = *it;

	return mic;
}

void act::room::AudioManager::mapMicrophones()
{
	m_microphoneNames.resize(0);
	if (!m_inputRouter)
		return;

	for (auto mic : m_microphones) {
		if (mic->getChannel() >= m_inputDeviceNode->getNumChannels())
			continue;

		m_inputDeviceNode >> m_inputRouter->route(mic->getChannel(), 0) >> mic->getIn();

		std::stringstream strstr;
		strstr << mic->getChannel() << ": " << mic->getName();
		m_microphoneNames.push_back(strstr.str());
	}
}

void act::room::AudioManager::refreshLists()
{

}

