
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "ModuleBase.hpp"

void act::mod::ModuleBase::loadRecentProject() {
	std::string recentProjectPath = "recent" + getName() + ".json";
	ci::fs::path path = ci::app::getAssetPath(recentProjectPath);

	if (path.empty()) {
		path = ci::app::getAssetPath("").string() + recentProjectPath;
		ci::writeJson(path, ""); // touch
		save(path);
	}

	load(path);
}