
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "NetworkModule.hpp"
#include "cinder/CinderImGui.h"
#include "imnodes.h"
#include "Design.hpp"



act::mod::NetworkModule::NetworkModule() {
	setName("Network");

};

act::mod::NetworkModule::~NetworkModule() {
}


void act::mod::NetworkModule::setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) {
	m_roomMgrs = roomMgrs;
	m_networkMgr = networkMgr;

	fs::path path = app::getAssetPath("recentNetwork.json");
	
	if (path.empty()) {
		path = app::getAssetPath("").string() + "recentNetwork.json";
		writeJson(path,""); // touch
		saveToFile(path);
	}

	loadFromFile(path);
}

void act::mod::NetworkModule::cleanUp() {
	saveToFile(app::getAssetPath("recentNetwork.json"));
}


void act::mod::NetworkModule::update() {
	
}

void act::mod::NetworkModule::draw() {
	
}

void act::mod::NetworkModule::drawGUI() {	

	drawNetworkState();
}


void act::mod::NetworkModule::drawNetworkState() {

	ImGui::Begin("NetworkState");
		
	//if(ImGui::Button("refresh network")) {	
	//}

	m_networkMgr->drawGUI();

	//ImGui::NewLine();
	

	ImGui::End();
}


void act::mod::NetworkModule::saveToFile(fs::path path) {
	ci::writeJson(path, getFullDescription());
}

ci::Json act::mod::NetworkModule::getFullDescription()
{
	ci::Json networkInformation = ci::Json::object();

	networkInformation["network"] = m_networkMgr->toJson(); // networkManager:

	return networkInformation;
}

void act::mod::NetworkModule::loadFromFile(fs::path path) {

	ci::Json networkInformation = ci::loadJson(loadFile(path));

	if (networkInformation.contains("networkManager")) {
		m_networkMgr->fromJson(networkInformation["networkManager"]);
	}
	
}