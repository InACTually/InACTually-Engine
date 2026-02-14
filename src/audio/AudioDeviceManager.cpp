
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "AudioDeviceManager.hpp"

#include "cinder/audio/audio.h"
#include "cinder/Log.h"
#include "cinder/app/App.h"

#include "cinder/audio/ContextPortAudio.h"
#include "cinder/audio/DeviceManagerPortAudio.h"

#include "cinder/CinderImGui.h"

using namespace ci;

act::aio::AudioDeviceManager::AudioDeviceManager()
{
}

act::aio::AudioDeviceManager::~AudioDeviceManager()
{
}

void act::aio::AudioDeviceManager::init()
{
	// make ContextPortAudio the master context, overriding cinder's default
#if PA_USE_ASIO 
	try {
		//ci::audio::ContextPortAudio::setAsMaster();
	}
	catch (...) {

	}
#endif

	refreshDeviceList();
	updateInfo();

	std::string defaultOutKey = "";
	std::string defaultInKey = "";
	
	try {
		if (ci::audio::Context::deviceManager()->getDefaultOutput())
			defaultOutKey = ci::audio::Context::deviceManager()->getDefaultOutput()->getKey();
		if (ci::audio::Context::deviceManager()->getDefaultInput())
			defaultInKey = ci::audio::Context::deviceManager()->getDefaultInput()->getKey();
	}
	catch (...) {

	}
	for (int i = 0; i < m_outKeys.size(); i++) {
		CI_LOG_I(m_outNames[i]);
		if (m_outKeys[i] == defaultOutKey) {
			m_outSelection = i;
		}
	}
	for (int i = 0; i < m_inKeys.size(); i++) {
		if (m_inKeys[i] == defaultInKey) {
			m_inSelection = i;
		}
	}

	ci::audio::DeviceRef deviceWithMaxOutputs;

	for (const auto& dev : ci::audio::Device::getDevices()) {
		if (!deviceWithMaxOutputs || deviceWithMaxOutputs->getNumOutputChannels() < dev->getNumOutputChannels())
			deviceWithMaxOutputs = dev;
	}

	// !!! you might want to set your device directly here 
	
	//setDevice("Headphones (High Definition Audio Device)", "Analogue 1 + 2");
	//setDevice(deviceWithMaxOutputs->getKey());
	//setDevice("Dante Virtual Soundcard(x64)");
	//setDevice("0 - ASIO - Focusrite USB ASIO");
	//setDevice("Main", "Input 1/2");
	//setDevice("Lautsprecher", "Mikrofon");
	//setDevice("ASIO - Focusrite", "ASIO - Focusrite");


	printContextInfo();
	CI_LOG_I("Audio devices:\n" << ci::audio::Device::printDevicesToString());

	ci::audio::Context::master()->enable();
}

void act::aio::AudioDeviceManager::drawSettings()
{
	//ImGui::Begin("AudioSettings");
	if (ImGui::Combo("Output Device", &m_outSelection, m_outNames)) {
		setupOutputDevice(m_outKeys[m_outSelection]);
	}
	if (ImGui::Combo("Input Device", &m_inSelection, m_inNames)) {
		setupInputDevice(m_inKeys[m_inSelection]);
	}
	ImGui::Text(m_info.c_str());

	//ImGui::End();
}

void act::aio::AudioDeviceManager::printContextInfo()
{
	std::stringstream str;
	auto ctx = ci::audio::master();

	str << "\n-------------- Context info --------------\n";
	str << "enabled: " << std::boolalpha << ctx->isEnabled() << ", samplerate: " << ctx->getSampleRate() << ", frames per block: " << ctx->getFramesPerBlock() << std::endl;

	str << "-------------- Graph configuration: --------------" << std::endl;
	str << ci::audio::master()->printGraphToString();
	str << "--------------------------------------------------" << std::endl;

	CI_LOG_I(str.str());
}

void act::aio::AudioDeviceManager::refreshDeviceList()
{
	std::string deviceDescriptions = ci::audio::Device::printDevicesToString();
	std::vector<std::string> lines = util::splitString(deviceDescriptions, "\n");

	AudioDevice device;
	std::vector<AudioDevice> devices;

	int lineType = 0;
	for (std::string line : lines) {
		switch (lineType) {
		case 0: // -- name --
			device = AudioDevice();
			device.name = line;
			if (device.name.length() > 6) { // "-- name --" to "name"
				device.name = device.name.substr(3, device.name.length() - 6);
			}
			device.driver = "os";
			break;
		case 1: // key: x - driver - name
			{
				std::vector<std::string> entries = util::splitString(line, " - ");
				device.key		= util::splitString(entries[0], ": ")[1];
				if(entries.size() > 1)
					device.driver	= entries[1];
				if(entries.size() > 2)
					device.name		= entries[2];

				if (device.driver.starts_with("Windows"))
					device.driver = device.driver.substr(8);
			}
			break;
		case 2: // inputs: x, outputs: x
		{
			std::vector<std::string> entries = util::splitString(line, ", ");
			device.inputs = stoi(util::splitString(entries[0], ": ")[1]);
			device.outputs = stoi(util::splitString(entries[1], ": ")[1]);
		}
			break;
		case 3: // samplerate: x, frames per block: x
			std::vector<std::string> entries = util::splitString(line, "frames per block: ");
			device.framesPerBlock = stoi(entries[1]);

			devices.push_back(device);
			break;
		}
		lineType = std::fmodf(++lineType, 4);
	}

	// for GUI:
	for (const auto& device : devices) {
		if (device.outputs > 0) {
			std::stringstream strstr;
			strstr << device.driver << " - [" << device.outputs << "] " << device.name;
			m_outNames.push_back(strstr.str());
			m_outKeys.push_back(device.key);
		}
		if (device.inputs > 0) {
			std::stringstream strstr;
			strstr << device.driver << " - [" << device.inputs << "] " << device.name;
			m_inNames.push_back(strstr.str());
			m_inKeys.push_back(device.key);
		}
	}
}

ci::Json act::aio::AudioDeviceManager::toJson()
{
	auto json = ci::Json::object();

	if(m_outputDeviceNode)
		json["outputDevice"] = m_outputDeviceNode->getDevice()->getKey();
	if (m_inputDeviceNode)
		json["inputDevice"] = m_inputDeviceNode->getDevice()->getKey();

	return json;
}

void act::aio::AudioDeviceManager::fromJson(ci::Json json)
{

	std::string outputDeviceKey = "";
	util::setValueFromJson(json, "outputDevice", outputDeviceKey);
	
	std::string inputDeviceKey = "";
	util::setValueFromJson(json, "inputDevice", inputDeviceKey);

	setupOutputDevice(outputDeviceKey);
	setupInputDevice(inputDeviceKey);
}

void act::aio::AudioDeviceManager::updateInfo()
{
	std::stringstream str;
	if(ci::audio::master() && m_outNames.size())
		str << "Samplerate: " << ci::audio::master()->getSampleRate();
	if (m_outputDeviceNode)
		str << ", Out: " << m_outputDeviceNode->getNumChannels();
	if(m_inputDeviceNode)
		str << ", In: " << m_inputDeviceNode->getNumChannels();
	m_info = str.str();
}

void act::aio::AudioDeviceManager::setFormat(ci::audio::DeviceRef device)
{
	if (device) {
		if (device->getSampleRate() == 48000)
			return;

		ci::audio::Device::Format format;
		format.framesPerBlock(device->getFramesPerBlock());
		format.sampleRate(48000);
		try {
			device->updateFormat(format);
		}
		catch (...) {
			CI_LOG_W("Can not update SampleRate of " << device->getName());
		}
	}
}

void act::aio::AudioDeviceManager::setupOutputDevice(std::string key)
{
	ci::audio::DeviceRef device = ci::audio::master()->deviceManager()->findDeviceByKey(key);// , false, true);
	if (device) {
		setFormat(device);
		m_outputDeviceNode = ci::audio::master()->createOutputDeviceNode(device, ci::audio::Node::Format().channels(device->getNumOutputChannels()));

		auto ctx = ci::audio::master();
		ctx->disable();
		ctx->disconnectAllNodes();

		ci::audio::master()->setOutput(m_outputDeviceNode);
		
		std::stringstream str;
		str << "Output Device updated: " << m_outNames[m_outSelection] << std::endl;
		CI_LOG_I(str.str());
		printContextInfo();

		m_outputDeviceNode->enable();
		ctx->enable();
		updateInfo();

		onOutputDeviceChange(m_outputDeviceNode);
	}
}

void act::aio::AudioDeviceManager::setupInputDevice(std::string key)
{
	ci::audio::DeviceRef device = ci::audio::master()->deviceManager()->findDeviceByKey(key);// , false, true);
	if (device) {
		setFormat(device);
		m_inputDeviceNode = ci::audio::master()->createInputDeviceNode(device, ci::audio::Node::Format().channels(device->getNumInputChannels()));
		
		auto ctx = ci::audio::master();
		ctx->disable();
		//ctx->disconnectAllNodes();

		std::stringstream str;
		str << "Input Device updated: " << m_inNames[m_inSelection] << std::endl;
		CI_LOG_I(str.str());
		printContextInfo();

		m_inputDeviceNode->enable();
		ctx->enable();
		updateInfo();

		onInputDeviceChange(m_inputDeviceNode);
	}
}

void act::aio::AudioDeviceManager::setDevice(std::string key)
{
	for (int i = 0; i < m_outKeys.size(); i++) {
		//CI_LOG_D(m_outNames[i]);
		if (m_outKeys[i].find(key) != std::string::npos) {
			m_outSelection = i;
		}
	}

	setupOutputDevice(key);
	setupInputDevice(m_inKeys[m_inSelection]);
}

void act::aio::AudioDeviceManager::setDevice(std::string outName, std::string inName)
{
	if (m_outSelection >= m_outNames.size())
		m_outSelection = 0;
	if (m_inSelection >= m_inNames.size())
		m_inSelection = 0;

	for (int i = 0; i < m_outNames.size(); i++) {
		CI_LOG_I(m_outNames[i]);
		if (m_outNames[i].find(outName) != std::string::npos) {
			m_outSelection = i;
		}
	}

	for (int i = 0; i < m_inNames.size(); i++) {
		CI_LOG_I(m_inNames[i]);
		if (m_inNames[i].find(inName) != std::string::npos) {
			m_inSelection = i;
		}
	}
	if(m_outKeys.size() > m_outSelection)
		setupOutputDevice(m_outKeys[m_outSelection]);
	if (m_inKeys.size() > m_inSelection)
		setupInputDevice(m_inKeys[m_inSelection]);
}
