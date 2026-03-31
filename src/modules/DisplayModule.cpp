
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "DisplayModule.hpp"
#include "cinder/CinderImGui.h"
#include "imnodes.h"
#include "Design.hpp"



act::mod::DisplayModule::DisplayModule() {
	setName("Displaying");

};

act::mod::DisplayModule::~DisplayModule() {
}

void act::mod::DisplayModule::setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) {
	m_roomMgrs = roomMgrs;
	m_networkMgr = networkMgr;

	fs::path path = app::getAssetPath("displaying.json");

	if (path.empty()) {
		path = app::getAssetPath("").string() + "displaying.json";
		ci::writeJson(path, ""); // touch
		saveToFile(path);
	}

	loadFromFile(path);

}

void act::mod::DisplayModule::cleanUp() {
	saveToFile(app::getAssetPath("displaying.json"));
}


void act::mod::DisplayModule::update() {

}

void act::mod::DisplayModule::draw() {

}

void act::mod::DisplayModule::drawGUI() {
	ImGui::Begin("Monitor Display");
	ImGuiID dockspace_id = ImGui::GetID("DisplayDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

	for(auto source : m_roomMgrs.displayMgr->getSources()) {
		ImGui::Begin(source.first.c_str());
		ImVec2 region = ImGui::GetContentRegionAvail();
		vec2 size = vec2(region.x, region.y);

		ci::vec2 texSize = Rectf(source.second->getBounds()).getCenteredFit(ci::Rectf(ivec2(0, 0), size), true).getSize();

		gl::pushMatrices();
			gl::rotate(toRadians(180.0f));
			ImGui::Image(source.second, texSize, vec2(1, 1), vec2(0, 0));
		gl::popMatrices();
		ImGui::End();
	}

	ImGui::End();
}

void act::mod::DisplayModule::saveToFile(fs::path path) {
	ci::writeJson(path, getFullDescription());
}

ci::Json act::mod::DisplayModule::getFullDescription()
{
	ci::Json description = ci::Json::object();
	description["isActive"] = m_isActive;

	return description;
}

void act::mod::DisplayModule::loadFromFile(fs::path path) {

	ci::Json description = ci::loadJson(loadFile(path));

	util::setValueFromJson(description, "isActive", m_isActive);
	/*for (auto&& childs : nodeConfiguration.getChild("monitoring").getChildren()) {
	}*/
}
