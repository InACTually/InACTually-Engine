/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	ein-christoph
*/

#include "roompch.hpp"
#include "light/FixtureDescriptionImporter.hpp"
#include "dmx/DMXManager.hpp"


act::system::FixtureDescriptionImporter::FixtureDescriptionImporter() {
	m_oflDescriptionMapper = OFLDescriptionMapper::create();
	m_isShowImporter = false;
}

act::system::FixtureDescriptionImporter::~FixtureDescriptionImporter() {
}

void act::system::FixtureDescriptionImporter::draw() {
	if (ImGui::Button("Import Fixture Description"))
		m_isShowImporter = true;

	if (!m_isShowImporter)
		return;

	ImGui::OpenPopup("Fixture Import");

	if (ImGui::BeginPopupModal("Fixture Import")) {

		drawOFLImport();

		if (ImGui::Button("Close Importer")) {
			m_isShowImporter = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void act::system::FixtureDescriptionImporter::update() {
	if (m_isOpenOFLInBrowser) {
		CI_LOG_D("Opening Open Fixture Library Website");
		ci::app::Platform::get()->launchWebBrowser(ci::Url("https://open-fixture-library.org/"));
		m_isOpenOFLInBrowser = false;
	}
	if (m_isSearchOFLAgain) {
		CI_LOG_D("Trying to find Open Fixture Library again...");
		m_oflDescriptionMapper->searchLibraryPath(); // Set Library path
		m_oflDescriptionMapper->getManufacturers(true); // And parse manufacturers
		m_isSearchOFLAgain = false;
	}
	if (m_hasOFLFixtureFilterChanged) {
		filterOFLFixtures();
		m_hasOFLFixtureFilterChanged = false;
	}

	if (m_isOFLReparsing) {
		m_oflDescriptionMapper->getManufacturers(true);
		m_isOFLReparsing = false;
	}

	while (m_oflFetchFixtureQueue.size() > 0) {
		auto const& fixture = m_oflFetchFixtureQueue.front();
		bool success = false;
		// Fetch and parse description if not present
		if (fixture->externalDescription.empty())
			success = m_oflDescriptionMapper->parseFixtureDescription(fixture);
		else
			success = true;

		if (success) {
			// we have a valid description converted
			std::string dmpKey = fixture->uid + "-" + fixture->modes.at(fixture->selectedMode)->name;
			m_oflJsonDmpCache[dmpKey] = OFLDescriptionMapper::getInternalDescription(fixture).dump(3);
		}
		fixture->isQueuedForLoading = false;
		m_oflFetchFixtureQueue.pop_front();
	}

	while (m_importQueue.size() > 0) {
		auto const& fixture = m_importQueue.front();
		ci::Json internalDesc = OFLDescriptionMapper::getInternalDescription(fixture);

		if (auto const dmxManagerRef = m_importFixtureListenerWRef.lock())
			dmxManagerRef->onImportFixture(internalDesc);

		m_importQueue.pop_front();
	}
}

void act::system::FixtureDescriptionImporter::drawOFLImport() {
	if (ImGui::CollapsingHeader(m_oflDescriptionMapper->getName().c_str())) {
		// Check if the ofl library is present
		if (m_oflDescriptionMapper->getLibraryPath().empty()) {
			ImGui::Text("No Path to the Open Fixture Library Found!");
			ImGui::Spacing();
			ImGui::TextWrapped("Please download the 'Open Fixture Library JSON' ZIP-Archive from https://open-fixture-library.org/ and extract it as 'ofl_export_ofl' into the 'dmx' subfolder of the assets folder.");
			std::string assetsPath = "Current assets folder: " + ci::app::getAssetPath("").string();
			ImGui::Spacing();
			ImGui::TextWrapped(assetsPath.c_str());
			if (ImGui::Button("Open OFL Website"))
				m_isOpenOFLInBrowser = true;
			ImGui::SameLine();
			if (ImGui::Button("Search Again"))
				m_isSearchOFLAgain = true;
			ImGui::Spacing();
			return;
		}

		//== Header with library path an search box
		ImGui::Text(("Library Path: " + m_oflDescriptionMapper->getLibraryPath().string()).c_str());

		ImGui::Spacing();
		if (ImGui::InputText("Search OFL Fixture", m_oflFixtureFilterBuffer, IM_ARRAYSIZE(m_oflFixtureFilterBuffer)))
			m_hasOFLFixtureFilterChanged = true;
		ImGui::Spacing();

		if (!m_oflDescriptionMapper->getIsParsed()) {
			if (!m_isOFLReparsing) m_isOFLReparsing = true;
			ImGui::Text("Open Fixture Library not loaded jet. Queued for loading.");
		}

		//== Loop over all manufacturers
		std::vector<ofl::OFLManufacturerRef> manufacturers = m_oflDescriptionMapper->getManufacturers(false);
		for (int manufacturerId = 0; manufacturerId < manufacturers.size(); manufacturerId++) {
			ofl::OFLManufacturerRef manufacturer = manufacturers.at(manufacturerId);

			if (m_isOFLListFilteres && !manufacturer->isShowInListing)
				continue; // Skipp if we filter and manufacturer is not to be shown


			if (m_isOFLListFilteres && manufacturer->isExpandInListing)
				ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNode(manufacturer->name.c_str())) {
				ImGui::Indent(1);

				//== And all fixtures to display them in a tree hirarchy
				bool fixtureShown = false;
				for (int fixtureId = 0; fixtureId < manufacturer->fixtures.size(); fixtureId++) {
					ofl::OFLFixtureDescriptionRef fixture = manufacturer->fixtures.at(fixtureId);

					if (m_isOFLListFilteres && !fixture->isShowInListing)
						continue; // Skipp if we filter and fixture is not to be shown

					fixtureShown = true;
					//== Draw the details of the fixture including mode selector and import button
					if (ImGui::TreeNode(fixture->name.c_str())) {
						ImGui::Indent(1);
						drawOFLFixtureDetails(fixture, manufacturerId, fixtureId);
						ImGui::TreePop();
						ImGui::Spacing();
					}
				}

				if (!fixtureShown)
					ImGui::Text("No fixture matching the search term found!");

				ImGui::TreePop();
				ImGui::Spacing();
			}
		}
	}
}

void act::system::FixtureDescriptionImporter::drawOFLFixtureDetails(ofl::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId) {
	if (fixture->isQueuedForLoading) {
		// Fixture is queued for loading
		ImGui::Text("... Fixture details loading...");
		return;
	}

	// Check if there is an Description ready
	if (fixture->externalDescription.empty() && !fixture->isQueuedForLoading) {
		fixture->isQueuedForLoading = true;
		m_oflFetchFixtureQueue.push_back(fixture);
		return;
	}

	if (!fixture->isSupportedType) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::Text("Fixture type is not supported!");
		ImGui::PopStyleColor();
		if (fixture->type == "laser") {
			ImGui::Text("InACTually can not handle lasers so they won't be converted.");
			return;
		}

		bool checkBoxBefore = fixture->isForceConverted;
		ImGui::Checkbox(("Force convert '" + fixture->name + "' anyway").c_str(), &fixture->isForceConverted);
		if (fixture->isForceConverted != checkBoxBefore && fixture->isForceConverted) {
			// set externalDescription to empty to force new traslation
			fixture->externalDescription = ci::Json();
			fixture->isQueuedForLoading = true;
			m_oflFetchFixtureQueue.push_back(fixture);
		}

		if (!fixture->isForceConverted) // Continue only if we force converted the fixture
			return;
	}

	if (fixture->hasError) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::Text("Error while parsing the fixture description!");
		ImGui::PopStyleColor();
		return;
	}

	//== Mode selection combo box
	const char* preview_val = fixture->selectedMode >= 0 && fixture->selectedMode < fixture->modes.size()
		? fixture->modes.at(fixture->selectedMode)->name.c_str() : "Select Mode...";

	if (ImGui::BeginCombo("Mode", preview_val)) {
		for (int i = 0; i < fixture->modes.size(); i++) {
			if (ImGui::Selectable(fixture->modes.at(i)->name.c_str(), fixture->selectedMode == i))
				fixture->selectedMode = i;

			if (fixture->selectedMode == i)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndCombo();
	}

	if (fixture->selectedMode >= fixture->modes.size()) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Can not display selected mode!");
		return;
	}

	//== Table for displaying the fixtures
	drawOFLFixtureTable(fixture, manufacturerId, fixtureId);

	ImGui::Spacing();
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Not all OFL fixtures are fully supported. OFL descriptions are converted as best as possible to the InACTually internal description.");

	std::string internalDescKey = fixture->uid + "-" + fixture->modes.at(fixture->selectedMode)->name;

	//== Show internal description in readonly textbox
	if (ImGui::TreeNode("Internal fixture Description")) {
		ImGui::Indent(1);

		// store json dump in map so the inputTextMultline callback has something to work with

		if (m_oflJsonDmpCache.find(internalDescKey) == m_oflJsonDmpCache.end()
			&& !fixture->isQueuedForLoading) {
			fixture->isQueuedForLoading = true;
			m_oflFetchFixtureQueue.push_back(fixture);
		}
		else {
			ImGui::InputTextMultiline(internalDescKey.c_str(), &m_oflJsonDmpCache.at(internalDescKey), ImVec2(-FLT_MIN, 300), ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::TreePop();
		ImGui::Spacing();
	}

	// === Import Button
	ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Always check plausibility of the internal fixture description before importing the fixture!");
	if (ImGui::Button("Add Fixture To Project") && !fixture->isQueuedForLoading) {
		m_importQueue.push_back(fixture);
		m_isShowImporter = false;
	}
}

void act::system::FixtureDescriptionImporter::drawOFLFixtureTable(ofl::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId) {
	bool showMindNotes = false;

	//== Table for displaying the fixture details
	if (ImGui::BeginTable((fixture->name + "Mode Details").c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Include");
		ImGui::TableSetupColumn("Channel");
		ImGui::TableSetupColumn("OFL Channel Name");
		ImGui::TableSetupColumn("InACTually Parameter Name");
		ImGui::TableHeadersRow();


		for (auto const& [dmxOffset, modeRef] : fixture->modes.at(fixture->selectedMode)->channelMapping) {
			bool isSupported = modeRef->channelDescPatch->descriptionPatch.contains("mapping");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (isSupported) {
				std::string checkboxlabel = "##" + fixture->uid + std::to_string(dmxOffset);
				bool isIncludedPre = modeRef->channelDescPatch->isIncludePatch;
				ImGui::SetWindowFontScale(0.3f);
				ImGui::Checkbox(checkboxlabel.c_str(), &modeRef->channelDescPatch->isIncludePatch);
				ImGui::SetWindowFontScale(1.0f);
				if (isIncludedPre != modeRef->channelDescPatch->isIncludePatch && !fixture->isQueuedForLoading) {
					fixture->isQueuedForLoading = true;
					m_oflFetchFixtureQueue.push_back(fixture);
				}
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%i", dmxOffset + 1);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(modeRef->oflChannelKey.c_str());
			ImGui::TableSetColumnIndex(3);

			if (!isSupported) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Supported!");
				continue;
			}

			std::string internalParamsStr = "";
			for (auto const& [parameterKey, channel] : modeRef->channelDescPatch->descriptionPatch["mapping"].items()) {
				if (internalParamsStr != "")
					internalParamsStr += ", ";

				internalParamsStr += parameterKey;

				if (channel.is_number()) {
					int channelNr = channel.get<int>();
					if (channel != dmxOffset + 1)
						internalParamsStr += " (mapped to channel " + std::to_string(channelNr) + ")";
				}
				else
					internalParamsStr += " (MAPPING ERROR: Not a Number!)";
			}

			if (!modeRef->channelDescPatch->descriptionPatch.contains("notes")
				|| !modeRef->channelDescPatch->descriptionPatch["notes"].contains("channel-" + std::to_string(dmxOffset + 1))) {
				ImGui::Text(internalParamsStr.c_str());
				continue;
			}

			// The parameter is supported but has notes atached so we should display them to the user
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), internalParamsStr.c_str());


			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    NOTE:");

			for (auto const& note : modeRef->channelDescPatch->descriptionPatch["notes"]["channel-" + std::to_string(dmxOffset + 1)]) {
				if (!note.is_string())
					ImGui::TextColored(ImVec4(1.0f, 8.0f, 0.0f, 1.0f), "Can not display non string note, please refer to the internal fixture description!");
				else
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ("    " + note.get<std::string>()).c_str());
			}
			showMindNotes = true;
		}

		ImGui::EndTable();

		if (showMindNotes)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow colored parameters have notes attached to them. They might not be fully supported!");
	}
}

void act::system::FixtureDescriptionImporter::registerImportFixtureListener(std::weak_ptr<ImportFixtureListener> listenerWRef) {
	m_importFixtureListenerWRef = listenerWRef;
}

void act::system::FixtureDescriptionImporter::filterOFLFixtures() {
	std::string filterTerm(m_oflFixtureFilterBuffer);
	std::transform(filterTerm.begin(), filterTerm.end(), filterTerm.begin(), [](unsigned char c) {return std::tolower(c);});
	if (filterTerm.size() < 3) {
		// For performance reasons just filter if the search string has at least three characters
		m_isOFLListFilteres = false;
		return;
	}

	m_isOFLListFilteres = true;

	for (auto const& manufacturer : m_oflDescriptionMapper->getManufacturers(false)) {
		bool fixtureWithText = false;

		std::string manufacturerLowerCase = manufacturer->name;
		std::transform(manufacturerLowerCase.begin(), manufacturerLowerCase.end(), manufacturerLowerCase.begin(), [](unsigned char c) {return std::tolower(c);});

		bool forceShowFixtures = manufacturerLowerCase == filterTerm; // Show all Fixtures if manufacturer name matches search term exactly

		for (auto const& fixture : manufacturer->fixtures) {
			if (fixture->name.find(filterTerm) != std::string::npos || forceShowFixtures) {
				fixtureWithText = true;
				fixture->isShowInListing = true;
			}
			else
				fixture->isShowInListing = false;
		}

		manufacturer->isExpandInListing = fixtureWithText;

		if (fixtureWithText || manufacturerLowerCase.find(filterTerm) != std::string::npos)
			manufacturer->isShowInListing = true;
		else
			manufacturer->isShowInListing = false;
	}
}
