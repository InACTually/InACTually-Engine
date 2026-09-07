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
#include "dmx/OFLDescriptionMapper.hpp"
#include "utils/RGBAWHelper.h"
#include "utils/jsonHelper.hpp"

act::room::OFLDescriptionMapper::OFLDescriptionMapper()
{
	m_ofl.name = "Open Fixture Library";
	m_ofl.isParsed = false;
	searchLibraryPath();
}

act::room::OFLDescriptionMapper::~OFLDescriptionMapper()
{
}

std::string act::room::OFLDescriptionMapper::getName()
{
	return m_ofl.name;
}

bool act::room::OFLDescriptionMapper::searchLibraryPath()
{
	ci::fs::path usualPath = ci::app::getAssetPath("dmx/ofl_export_ofl");
	return setLibraryPath(usualPath);
}

ci::fs::path act::room::OFLDescriptionMapper::getLibraryPath()
{
	return m_ofl.libraryPath;
}

bool act::room::OFLDescriptionMapper::setLibraryPath(ci::fs::path path)
{
	// Check if a manufacturer index exists at provided path
	ci::fs::path check = path;
	check.append(m_ManufacturerIdxFileName);
	if (!ci::fs::exists(check))
	{
		CI_LOG_W("No Open Fixture Library found at path '" << path.string() << "'");
		return false;
	}
	m_ofl.libraryPath = path;
	CI_LOG_I("Found Open Fixture Library:" << m_ofl.libraryPath);
	return true;
}

bool act::room::OFLDescriptionMapper::getIsParsed()
{
	return m_ofl.isParsed;
}

std::vector<act::room::ofl::OFLManufacturerRef> act::room::OFLDescriptionMapper::getManufacturers(bool allowParsingMeta)
{
	// If the library is not parsed jet but we are allowed to we parse it first
	if (!m_ofl.isParsed && allowParsingMeta)
		parseLibraryMeta();

	return m_ofl.manufacturers;

}

bool act::room::OFLDescriptionMapper::parseLibraryMeta()
{
	ci::Json manufacturersJson;
	ci::fs::path manufacturersDescriptionPath = m_ofl.libraryPath;
	manufacturersDescriptionPath.append(m_ManufacturerIdxFileName);
	try
	{
		manufacturersJson = ci::loadJson(ci::loadFile(manufacturersDescriptionPath));
	}
	catch (cinder::Exception e) {
		CI_LOG_E(e.what());
		return false;
	}

	m_ofl.isParsed = false;
	m_ofl.manufacturers.clear();

	for (ci::Json::iterator it = manufacturersJson.begin(); it != manufacturersJson.end(); ++it)
	{
		if (!it.value().is_object())
			continue;

		if (!it.value()["name"].is_string()) {
			CI_LOG_E("OFLImport: Error while parsing ofl manufacturers.json! Manufacturer name at position " << it.key() << " is not a string.");
			continue;
		}

		ofl::OFLManufacturer oflManufacturer = {
			.key = it.key(),
			.name = it.value()["name"]
		};

		// check if the key corresponds to a directors (as it should)
		ci::fs::path manufacturerDirectoryPath = m_ofl.libraryPath;
		manufacturerDirectoryPath.append(oflManufacturer.key);
		if (!ci::fs::is_directory(manufacturerDirectoryPath))
		{
			CI_LOG_E("OFLImport: Could not find directory: "<< manufacturerDirectoryPath);
			continue;
		}

		// iterate files in the directors and use their names as fixture names
		// this might not be the real fixture name but otherwise all json descriptons
		// would have to be parsed, most of which will not be needed
		for (const auto& entry : ci::fs::directory_iterator(manufacturerDirectoryPath))
		{
			if (!entry.path().has_extension() || entry.path().extension() != ".json")
				continue;

			// aproximating fixture name out of filename
			std::string filename = entry.path().filename().stem().string();
			std::replace(filename.begin(), filename.end(), '_', ' ');
			std::replace(filename.begin(), filename.end(), '-', ' ');

			ofl::OFLFixtureDescription oflDescription = {
				.uid = oflManufacturer.key + "-" + filename,
				.name = filename,
				.descriptionPath = entry.path()
			};

			oflManufacturer.fixtures.push_back(std::make_shared<ofl::OFLFixtureDescription>(oflDescription));
		}

		m_ofl.manufacturers.push_back(std::make_shared<ofl::OFLManufacturer>(oflManufacturer));
	}

	m_ofl.isParsed = true;

	return true;
}

bool act::room::OFLDescriptionMapper::parseFixtureDescription(ofl::OFLFixtureDescriptionRef fixtureDescription)
{
	CI_LOG_I("Parsing Description of '" << fixtureDescription->name << "' ...");
	fixtureDescription->externalDescription.clear();
	fixtureDescription->modes.clear();
	fixtureDescription->hasError = true;

	if (!fixtureDescription->descriptionPath.has_extension() || fixtureDescription->descriptionPath.extension() != ".json")
	{
		CI_LOG_E("OFLDescriptionMapper::parseBasicDescription: Could not load fixture description because path does not lead to a json file!");
		return false;
	}

	try
	{
		fixtureDescription->externalDescription = ci::loadJson(ci::loadFile(fixtureDescription->descriptionPath));
	}
	catch (const std::exception& e)
	{
		CI_LOG_E("Could not load fixture description for path " << fixtureDescription->descriptionPath << ": " << e.what());
		return false;
	}

	ci::Json& externalDesc = fixtureDescription->externalDescription;

	if (!externalDesc.is_object()) 
		return false;

	if (!externalDesc.contains("availableChannels") || !externalDesc["availableChannels"].is_object())
		return false;

	//=== Check and set fixture type
	if (!externalDesc.contains("categories") || !externalDesc["categories"].is_array())
	{
		CI_LOG_E("External description does not contain array of fixture categories!");
		return false;
	}
		
	// Check if it is a laser and abort transltion if it is
	if (act::util::isInJsonArray("Laser", externalDesc["categories"]))
	{
		fixtureDescription->type = "laser";
		fixtureDescription->isSupportedType = false;
		CI_LOG_W("Fixture is or contains a laser! InACTually currently can not handle lasers so it will not be converted.");
		return false;
	}

	// Check if it is one of the supported types
	if (externalDesc["categories"].size() == 1 && externalDesc["categories"][0] == "Dimmer")
		fixtureDescription->type = "dimmer";
	else if (act::util::isInJsonArray("Moving Head", externalDesc["categories"]) || act::util::isInJsonArray("Color Changer", externalDesc["categories"]))
		fixtureDescription->type = "mv";
	else if (fixtureDescription->forceConverted)
		fixtureDescription->type = "mv";
	else
	{
		CI_LOG_W("Fixture type is not supported");
		return false;
	}

	fixtureDescription->isSupportedType = true;

	//=== Check and set modes and corresponding channels
	if (!externalDesc.contains("modes") || !externalDesc["modes"].is_array())
		return false;

	for (int modeIndex = 0; modeIndex < externalDesc["modes"].size(); modeIndex++)
	{
		ci::Json const& modeDesc = externalDesc["modes"][modeIndex];
		if (!modeDesc.is_object() || !modeDesc.contains("name") || !modeDesc.contains("channels") || !modeDesc["channels"].is_array())
			return false;

		ofl::OFLMode oflMode = {
			.name = modeDesc["name"]
		};
		ofl::OFLModeRef oflModeRef = std::make_shared<ofl::OFLMode>(oflMode);
		fixtureDescription->modes.push_back(oflModeRef);

		// Convert ofl description into inACTually description (mode specific)
		try
		{
			convertOFLModeToInternal(modeDesc, fixtureDescription, oflModeRef, modeIndex);
			fixtureDescription->hasError = false;
		}
		catch (const std::exception& e)
		{
			fixtureDescription->hasError = true;
			CI_LOG_E("Could not convert fixture: '" << fixtureDescription->name << "'! : " << e.what());
			continue;
		}
	}

	return true;
}

bool act::room::OFLDescriptionMapper::convertOFLModeToInternal(ci::Json const& modeDesc, ofl::OFLFixtureDescriptionRef fixtureDescription, ofl::OFLModeRef modeRef, int modeIndex)
{
	ci::Json const& externalDesc = fixtureDescription->externalDescription;
	modeRef->internalDescBase = ci::Json::object();

	//Fixture Name (always mode specific)
	modeRef->internalDescBase["name"] = fixtureDescription->name + " (" + modeRef->name + " mode)";

	modeRef->internalDescBase["type"] = fixtureDescription->type;

	// Determine number of channels
	if (!modeDesc.contains("channels") || !modeDesc["channels"].is_array())
		throw std::invalid_argument("No ChannelArray found for selected mode.");

	modeRef->internalDescBase["channel"] = modeDesc["channels"].size();
		
	// Convert channels for selected mode to mapping
	// adding other information to the description if needed
	if (!externalDesc.contains("availableChannels") || !externalDesc["availableChannels"].is_object())
		throw std::invalid_argument("AvailableChannels missing in externalDescription");

	for (int dmxOffset = 0; dmxOffset < modeDesc["channels"].size(); dmxOffset++)
	{
		if (!modeDesc["channels"][dmxOffset].is_string())
		{
			if(!modeDesc["channels"][dmxOffset].is_null()) // null is specified value if channel is empty so dont log a warning
				CI_LOG_W("Unexpected channels format in external fixture defifition! Skipping non-string entries.");
			continue;
		}

		const std::string& channelKey = modeDesc["channels"][dmxOffset];

		ofl::OFLChannelDescPatchRef descPatchRef = std::make_shared<ofl::OFLChannelDescPatch>();
		descPatchRef->includePatch = false;

		// Check if there is already a description patch for this channel
		// checkOtherAffectedChannels could produce that
		ofl::OFLChannelRef channelRef;
		if (!modeRef->channelMapping.contains(dmxOffset))
		{
			ofl::OFLChannel channelDescriptionPatch;
			channelDescriptionPatch.oflChannelKey = channelKey;
			channelDescriptionPatch.channelDescPatch = descPatchRef;

			channelRef = std::make_shared<ofl::OFLChannel>(channelDescriptionPatch);
			modeRef->channelMapping.insert({ dmxOffset, channelRef });
		}
		else
		{
			channelRef = modeRef->channelMapping.at(dmxOffset);
			if (channelRef->oflChannelKey == "unknown")
				channelRef->oflChannelKey = channelKey;
		}

		// Channels are arbitrary keys so we need to lookup type of the capability which should follow a standardised naming
		// So lets check if the channel key is in the available channels and has a capability with a type
		if (!externalDesc["availableChannels"].contains(channelKey))
		{
			if(channelKey.substr(channelKey.length() - 4, 4) != "fine") // fine channels end with "fine" and get resolved within the channel description of the main channel
				CI_LOG_W("Could not find channl key '" << channelKey << "' in availableChannels! Skipping channel.");
			continue;
		}

		ci::Json internalDescPatch = ci::Json::object();

		try
		{
			internalDescPatch = convertChannel(externalDesc["availableChannels"][channelKey], externalDesc, dmxOffset + 1, modeIndex, channelKey);
		}
		catch (const std::exception& e)
		{
			CI_LOG_W("Could not convert channel '" << channelKey << "' :" << e.what());
		}
			

		if (internalDescPatch.is_null())
		{
			CI_LOG_E("Json Patch is Null! This would destroy the internalDescription and is therefore ignored! channelkey:" << channelKey);
			continue;
		}

		if (fixtureDescription->forceConverted)
			modeRef->internalDescBase["notes"]["forceConverted"] = "CHECK DESCRIPTION BEFORE USING IT! Fixture type is not a supported fixture type. This description was force converted by the user.";

		descPatchRef->descriptionPatch = internalDescPatch;

		// Add description to all channels in the mapping of the description patch
		addDescToOtherAffectedChannels(modeRef, descPatchRef, channelKey, dmxOffset);

		if (!internalDescPatch.empty())
		{
			// Check if notes were added while converting the fixture. If so there is probably something the user should actively notice
			// so we dont include the patch by default
			if (!internalDescPatch.contains("notes"))
				descPatchRef->includePatch = true;
			else
				CI_LOG_W("Channel " << channelKey << " wont be included in final internal description by default because notes were produced while converting the channel.");
		}
	}
	return true;
}

ci::Json act::room::OFLDescriptionMapper::getInternalDescription(ofl::OFLFixtureDescriptionRef fixture)
{
	ofl::OFLModeRef modeRef = fixture->modes.at(fixture->selectedMode);
	if (modeRef->internalDescBase.empty() || !modeRef->internalDescBase.contains("name"))
	{
		CI_LOG_E("Malformed internalDescBase provided, no internal description producable!");
		return ci::Json::object();
	}

	ci::Json internalDescription = modeRef->internalDescBase;

	for (auto const& [channel, channelRef] : modeRef->channelMapping)
	{
		//TODO check constraints like only one dimmer etc.
		if (channelRef->channelDescPatch->includePatch)
			internalDescription.merge_patch(channelRef->channelDescPatch->descriptionPatch);
	}

	return internalDescription;
}

void act::room::OFLDescriptionMapper::addDescToOtherAffectedChannels(ofl::OFLModeRef modeRef, ofl::OFLChannelDescPatchRef channelDescPatchRef, std::string const& channelKey, int primaryDmxOffset)
{
	// Loop over Mapping and add description to corresponding mappings
	if (channelDescPatchRef->descriptionPatch.is_null() || channelDescPatchRef->descriptionPatch.empty())
		return;

	if (!channelDescPatchRef->descriptionPatch.contains("mapping") || !channelDescPatchRef->descriptionPatch["mapping"].is_object()) {
		CI_LOG_E("Internal Description does not contain mapping object! channelKey:" << channelKey);
		return;
	}

	for (auto const& [parameter, inACTparamOffset] : channelDescPatchRef->descriptionPatch["mapping"].items())
	{
		if (!inACTparamOffset.is_number())
		{
			CI_LOG_E("Can not add internalDesc to all mappings because mapping value is not a number for parameter " << parameter << " of " << channelKey);
			return;
		}

		int paramDmxOffset = inACTparamOffset - 1; // arrays etc start with 0, inACTually uses 1 as first address

		if (paramDmxOffset == primaryDmxOffset)
			continue; // The description is set already by the convertOFLtoInternal function

		if (modeRef->channelMapping.contains(paramDmxOffset))
		{
			// There is already a channel description registered at this offset
			ofl::OFLChannelRef channelRef = modeRef->channelMapping.at(paramDmxOffset);

			// Check if it has already an internalDescription
			if (channelRef->channelDescPatch && !channelRef->channelDescPatch->descriptionPatch.empty())
			{
				// we already have a desription and it is not empty. This should not happen
				// So we dont include both by default and add a warning
				std::string note = "The parameter has two conflicting descriptions! Caused by parameter '" + parameter + "'!";
				channelDescPatchRef->includePatch = false;
				ofl::OFLHelper::attachNoteToChannelDesc(channelDescPatchRef->descriptionPatch, inACTparamOffset, note);

				channelRef->channelDescPatch->includePatch = false;
				ofl::OFLHelper::attachNoteToChannelDesc(channelRef->channelDescPatch->descriptionPatch, inACTparamOffset, note);
				continue;
			}

			// if there is no channel Descirption we can add the new one
			channelRef->channelDescPatch = channelDescPatchRef;
		}
		else
		{
			// otherwise we need to create a new channel description and append that
			ofl::OFLChannelRef channelDescriptionRef = std::make_shared<ofl::OFLChannel>();
			channelDescriptionRef->oflChannelKey = "unknown";
			channelDescriptionRef->channelDescPatch = channelDescPatchRef;
			modeRef->channelMapping.insert({ paramDmxOffset, channelDescriptionRef });
		}
	}
}

std::map<std::string, int> act::room::OFLDescriptionMapper::resolveFineChannels(ci::Json const& fullExtDesc, int modeIndex, ci::Json const& extChannelDesc, int maxAliases)
{
	if (!fullExtDesc.contains("modes") || !fullExtDesc["modes"].is_array() || fullExtDesc["modes"].size() <= modeIndex
		|| !fullExtDesc["modes"][modeIndex].contains("channels") || !fullExtDesc["modes"][modeIndex]["channels"].is_array())
	{
		CI_LOG_E("resolveFineChannel got malformed fullExtDesc! Skipping fine channel resolution.");
		return std::map<std::string, int>();
	}

	if (!extChannelDesc.contains("fineChannelAliases") || !extChannelDesc["fineChannelAliases"].is_array())
		return std::map<std::string, int>(); // Nothing to resolve

	std::map<std::string, int> retLookup;

	for (int i = 0; i < extChannelDesc["fineChannelAliases"].size(); i++)
	{
		if (i > maxAliases - 1)
		{
			CI_LOG_W("Found to many fineChannelAliases! Skipping all above " << i);
			break;
		}

		if (extChannelDesc["fineChannelAliases"][i].is_string())
		{
			std::string fineChannelAlias = extChannelDesc["fineChannelAliases"][i];
			auto const& channelList = fullExtDesc["modes"][modeIndex]["channels"];

			auto iter = std::find(channelList.begin(), channelList.end(), fineChannelAlias);
			if (iter != channelList.end())
			{
				retLookup.insert({ fineChannelAlias, std::distance(channelList.begin(), iter)});
			}
			// If the fineChannelAlias is not found in channels list this mode does not contain this fine channel so we ignore it
		}
		else
			CI_LOG_W("Found non string fineChannelAlias! Skipping fine channel.");
	}

	return retLookup;
}

// Determine the capability or capabilities of a channel and call the correct convert function
// Register new convert... functions in this method.
// For a list of current capability types see https://github.com/OpenLightingProject/open-fixture-library/blob/master/docs/capability-types.md
ci::Json act::room::OFLDescriptionMapper::convertChannel(ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{
	if (extChannelDesc.contains("capability")) // === Convert single capability
	{
		ci::Json const& extCapabilityDesc = extChannelDesc["capability"];

		if (!extCapabilityDesc.contains("type") || !extCapabilityDesc["type"].is_string())
			throw std::invalid_argument("Capability has to have a type attribute that is a string!");

		std::string const& capabilityType = extCapabilityDesc["type"];

		if (capabilityType == "NoFunction")
			return ci::Json::object(); // NoFunction can be ignored except for some special cases which should not matter for our purpose
		else if (capabilityType == "Intensity")
			return convertIntensityCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Pan" || capabilityType == "Tilt")
			return convertPanTiltCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Zoom")
			return convertZoomCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "ColorIntensity")
			return convertColorIntensityCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "PanTiltSpeed")
			return convertPanTiltSpeedCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
	}
	else if (extChannelDesc.contains("capabilities")) // === Convert capabilities list
	{
		ci::Json const& extCapabilitiesDesc = extChannelDesc["capabilities"];

		if (isColorWheel(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return convertColorWheelCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
		else if (isGoboWheel(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return convertGoboWheelCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
		else if (isShutterStrobeCapability(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return convertShutterStrobeCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
	}

	CI_LOG_W("No mapping exists for channel '" << channelName << "'");

	return ci::Json::object();
}

ci::Json act::room::OFLDescriptionMapper::convertIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	descPatch["mapping"]["dimmer"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineDimmer"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::convertPanTiltCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	// Converting Pan and Tilt is so similar it is combined into one function

	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("type") || (extCapabilityDesc["type"] != "Pan" && extCapabilityDesc["type"] != "Tilt"))
		throw std::exception("convertPanTiltCapability called but capability is neither pan or tilt!");

	std::string panTiltIdentifier = extCapabilityDesc["type"]; // panTileIdentifier holds the information if we are converting pan or tilt
	std::string panTiltIdentifierL = panTiltIdentifier; // panTiltIdentifierL is the lowercase equivalent
	std::transform(panTiltIdentifierL.begin(), panTiltIdentifierL.end(), panTiltIdentifierL.begin(), [](unsigned char c) {return std::tolower(c);});

	// Check angle start and angle end to calculate Range
	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string()) 
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = ofl::OFLHelper::convertDegStrToInt(extCapabilityDesc["angleStart"]);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = ofl::OFLHelper::convertDegStrToInt(extCapabilityDesc["angleEnd"]);

	if (angleEnd < angleStart) throw std::exception("angleStart is larger that angleEnd in pan or tilt capability!");

	descPatch[panTiltIdentifierL +"Range"] = angleEnd - angleStart;

	descPatch["mapping"][panTiltIdentifierL] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fine"+ panTiltIdentifier] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::convertZoomCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string())
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = ofl::OFLHelper::beamAngleToDegRange(extCapabilityDesc["angleStart"], modeIndex, fullExtDesc);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = ofl::OFLHelper::beamAngleToDegRange(extCapabilityDesc["angleEnd"], modeIndex, fullExtDesc);

	if (angleStart > angleEnd)
	{
		descPatch["beamAngleMax"] = angleStart;
		descPatch["beamAngleMin"] = angleEnd;
	}
	else 
	{
		descPatch["beamAngleMax"] = angleEnd;
		descPatch["beamAngleMin"] = angleStart;
	}

	descPatch["beamAngle"] = descPatch["beamAngleMin"];

	descPatch["mapping"]["zoom"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineZoom"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::convertColorIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("color") || !extCapabilityDesc["color"].is_string())
		throw std::invalid_argument("Capability Description does not contain color entry or color entry is not a string!");

	// OFL color key to internal color key
	std::map<std::string, std::string> colorKeyLookup = { {"Red", "R"}
															,{"Green", "G"}
															,{"Blue", "B"}
															,{"Amber", "A"}
															,{"White", "W"}
															,{"UV", "UV"}
															};

	if (colorKeyLookup.find(extCapabilityDesc["color"]) != colorKeyLookup.end())
	{
		std::string color = colorKeyLookup.at(extCapabilityDesc["color"]);
		descPatch["mapping"][color] = dmxOffset;

		auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
		if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
			descPatch["mapping"]["fine" + color] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0
	}
	else
		CI_LOG_W("Color type '" << extCapabilityDesc["color"] << "' is not supported.");


	// Not resolving brightnessStart and brightnessEnd because InACTually currently can not use them

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::convertPanTiltSpeedCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();
	
	descPatch["mapping"]["speed"] = dmxOffset;

	return descPatch;
}

// Checks if there is a capability with array of type wheelSlot and a corresponding wheel description with slots of type color
bool act::room::OFLDescriptionMapper::isColorWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false; // A Color Wheeel has to have an array of WheelSlot capabilities

	int idx = act::util::isInJsonObjArray("type", "WheelSlot", extChannelDesc["capabilities"]);

	if (idx < 0)
		return false; // No WheelSlot in capabilities

	std::string WheelName = ofl::OFLHelper::getWheelNameKey(channelName, extChannelDesc["capabilities"][idx]);

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].contains(WheelName) || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
		return false; // Color wheels always have a seperate description holding the information about the colors

	if(act::util::isInJsonObjArray("type", "Color", fullExtDesc["wheels"][WheelName]["slots"]) < 0)
		return false; // if no slots of the wheel holds color it is not a color wheel
	
	return true; // Otherwise it is a color wheel
}

ci::Json act::room::OFLDescriptionMapper::convertColorWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") ||  !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("A Color Wheeel has to have an array of WheelSlot capabilities");

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].is_object())
		throw std::invalid_argument("For a Color Wheel a wheels object has to be present in the external description");

	std::list<std::string> notes = std::list<std::string>();

	ci::Json descPatch = ci::Json::object();

	ci::Json colorMap = ci::Json::object();
	colorMap["colors"] = ci::Json::array();

	int amount = 0;
	int start = -1;
	int end = -1;

	for (auto const& [idx, capability] : extChannelDesc["capabilities"].items())
	{
		if (!capability.contains("type") || !capability["type"].is_string())
		{
			CI_LOG_W("Found no or non string type in capability of color wheel! Skipping capability.");
			continue;
		}
		if(capability["type"] != "WheelSlot")
		{
			CI_LOG_W("Found non Wheel Slot Type '" << capability["type"] << "' in color wheel! Skipping capability");
			continue;
		}

		std::string WheelName = (capability.contains("wheel") && capability["wheel"].is_string()) ? capability["wheel"].get<std::string>() : channelName;

		if (!fullExtDesc["wheels"].contains(WheelName))
			throw std::invalid_argument("Wheel with name '" + WheelName + "' could not be found in wheels object.");

		if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2
			|| !capability["dmxRange"][0].is_number() || !capability["dmxRange"][1].is_number())
			throw std::invalid_argument("Malformed dmxRange in capability!");

		int dmxMin = capability["dmxRange"][0];
		int dmxMax = capability["dmxRange"][1];
		int dmxValue = ofl::OFLHelper::dmxRangeToDmxValue(capability["dmxRange"]);

		if (!capability.contains("slotNumber") || !capability["slotNumber"].is_number_integer())
		{
			CI_LOG_W("Color Wheel: WheelSlot at position " << idx << " does not specify single slotNumbers! Half frames currently not supported, skipping slot.");

			if (  (capability.contains("slotNumberStart") && capability.contains("slotNumberEnd")) // Split slots can be identified by two slot numbers
				|| capability.contains("slotNumber") && capability["slotNumber"].is_number_float()) // or by a float as slot number
				notes.push_back("Dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " unavailable, split slots not supported.");
			else
				notes.push_back("Dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " unavailable, no unique slot identified.");

			continue;
		}

		int slotIdx = capability["slotNumber"] - 1; //SlotNumber starts at 1 but is referencing an array

		if (!fullExtDesc["wheels"][WheelName].is_object() || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
			throw std::invalid_argument("Malformed wheels wheel '" + WheelName + "'");

		if (slotIdx >= fullExtDesc["wheels"][WheelName]["slots"].size())
			throw std::invalid_argument("Channel '" + channelName + "' specifies slotNumber greater than available slots in wheels description!");

		auto const& wheelSlotDesc = fullExtDesc["wheels"][WheelName]["slots"][slotIdx];

		if (!wheelSlotDesc.contains("type") || !wheelSlotDesc["type"].is_string() || wheelSlotDesc["type"] != "Color")
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' is not of type 'Color'! Skipping slot.");
			notes.push_back("Slot at index " + std::to_string(slotIdx) + ", dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " is unavailable because it is not a color.");
			continue;
		}

		if (!wheelSlotDesc.contains("name") || !wheelSlotDesc["name"].is_string())
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' does not contain a name! Skipping slot.");
			notes.push_back("Slot at index " + std::to_string(slotIdx) + ", dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " is unavailable because it does not have a name.");
			continue;
		}

		if (!wheelSlotDesc.contains("colors") || !wheelSlotDesc["colors"].is_array() || wheelSlotDesc["colors"].size() < 1)
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' does not contain a or an empty colors array! Skipping slot.");
			notes.push_back("Slot at index " + std::to_string(slotIdx) + ", dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " is unavailable because no single color could be determined.");
			continue;
		}

		auto const& colorHex = wheelSlotDesc["colors"][0].get<std::string>();
		ci::Color const& colorRGB = RGBAWHelper::HEXtoRGB(colorHex);

		ci::Json colorObj = ci::Json::object();
		colorObj["value"] = dmxValue;
		colorObj["label"] = wheelSlotDesc["name"];
		colorObj["hex"] = colorHex;
		colorObj["R"] = (uint8_t)(colorRGB.r * 255);
		colorObj["G"] = (uint8_t)(colorRGB.g * 255);
		colorObj["B"] = (uint8_t)(colorRGB.b * 255);

		colorMap["colors"].push_back(colorObj);
		amount++;

		if (start == -1 || dmxValue < start)
			start = dmxValue;
		if (dmxValue > end)
			end = dmxValue;
	}

	if (amount < 1 || start < 0 || end < 0 || start > end)
		throw std::exception("Amount, start and/or end are not plausible!");

	colorMap["amount"] = amount;
	colorMap["start"] = start;
	colorMap["end"] = end;

	descPatch["colorMap"] = colorMap;
	descPatch["mapping"]["color"] = dmxOffset;

	if (!notes.empty())
		descPatch["notes"]["channel-" + std::to_string(dmxOffset)] = notes;

	return descPatch;
}

bool act::room::OFLDescriptionMapper::isGoboWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false; // A Gobo Wheeel has to have an array of WheelSlot capabilities

	int idx = act::util::isInJsonObjArray("type", "WheelSlot", extChannelDesc["capabilities"]);

	if (idx < 0)
		return false; // No WheelSlot in capabilities

	// WheelName can be channel name or stated explicitly in the description
	std::string WheelName = ofl::OFLHelper::getWheelNameKey(channelName, extChannelDesc["capabilities"][idx]);

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].contains(WheelName) || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
		return false; // Gobo wheels always have a seperate description holding the information about the gobos

	if (act::util::isInJsonObjArray("type", "Gobo", fullExtDesc["wheels"][WheelName]["slots"]) < 0)
		return false; // if no slots of the wheel holds gobo it is not a gobo wheel

	return true; // Otherwise it is a gobo wheel
}

ci::Json act::room::OFLDescriptionMapper::convertGoboWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") || !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("A Gobo Wheeel has to have an array of WheelSlot capabilities");

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].is_object())
		throw std::invalid_argument("For a Gobo Wheel a wheels object has to be present in the external description");

	std::list<std::string> notes = std::list<std::string>();

	ci::Json descPatch = ci::Json::object();

	int amount = 0;
	int goboStart = -1, goboEnd = -1;
	int shakeStart = -1, shakeEnd = -1;

	for (auto const& [idx, capability] : extChannelDesc["capabilities"].items())
	{
		if (!capability.contains("type") || !capability["type"].is_string())
		{
			CI_LOG_W("Found no or non string type in capability of gobo wheel! Skipping capability.");
			continue;
		}
		if (capability["type"] != "WheelSlot" && capability["type"] != "WheelShake")
		{
			CI_LOG_W("Found unexpected type '" << capability["type"] << "' in gobo wheel! Skipping capability");
			continue;
		}

		std::string WheelName = (capability.contains("wheel") && capability["wheel"].is_string()) ? capability["wheel"].get<std::string>() : channelName;

		if (!fullExtDesc["wheels"].contains(WheelName))
			throw std::invalid_argument("Wheel with name '" + WheelName + "' could not be found in wheels object.");

		if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2
			|| !capability["dmxRange"][0].is_number() || !capability["dmxRange"][1].is_number())
			throw std::invalid_argument("Malformed dmxRange in capability!");

		int dmxMin = capability["dmxRange"][0];
		int dmxMax = capability["dmxRange"][1];
		if (dmxMax < dmxMin)
		{
			dmxMax = dmxMin;
			dmxMin = capability["dmxRange"][1];
		}

		if (!capability.contains("slotNumber") || !capability["slotNumber"].is_number())
		{
			CI_LOG_W("Gobo Wheel capability at position " << idx << " does not specify slotNumber, skipping slot!");
			notes.push_back("Wheel slot " + idx + " unavailable, no slot number provided!");
			continue;
		}

		int slotIdx = capability["slotNumber"] - 1; //SlotNumber starts at 1 but is referencing an array

		if (!fullExtDesc["wheels"][WheelName].is_object() || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
			throw std::invalid_argument("Malformed wheels wheel '" + WheelName + "'");

		if (slotIdx >= fullExtDesc["wheels"][WheelName]["slots"].size())
			throw std::invalid_argument("Channel '" + channelName + "' specifies slotNumber greater than available slots in wheels description!");

		auto const& wheelSlotDesc = fullExtDesc["wheels"][WheelName]["slots"][slotIdx];

		if (!wheelSlotDesc.contains("type") || !wheelSlotDesc["type"].is_string() || (wheelSlotDesc["type"] != "Gobo" && wheelSlotDesc["type"] != "Open"))
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' is not of type 'Gobo' or 'Open'! Skipping slot.");
			continue;
		}

		// InACTually currently expects a continious range for gobo selection and wheelshake
		// so we have to ensure the new dmx range is at the lower or upper bound of the knwon range

		if (capability["type"] == "WheelSlot")
		{
			bool foundPlace = false;
			if (goboStart < 0 || (dmxMax + 1) == goboStart)
			{
				// gobo sits below the current range
				goboStart = dmxMin;
				foundPlace = true;
			}
			if (goboEnd < 0 || (dmxMin - 1) == goboEnd)
			{
				// gobo sits above the current range
				goboEnd = dmxMax;
				foundPlace = true;
			}

			if (foundPlace)
				amount++;
			else
			{
				CI_LOG_W("Gobo at slot index " << slotIdx << " does not fit into a continous range of gobos so it will be ignored!");
				notes.push_back("Gobo at slot " + std::to_string(slotIdx + 1) + " (dmx Values " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + ") will be unavailable because it's not in a continous dmx gobo range.");
				continue;
			}
		}
		else if (capability["type"] == "WheelShake")
		{
			bool foundPlace = false;
			if (shakeStart < 0 || (dmxMax + 1) == shakeStart)
			{
				// gobo sits below the current range
				shakeStart = dmxMin;
				foundPlace = true;
			}
			if (shakeEnd < 0 || (dmxMin - 1) == shakeEnd)
			{
				// gobo sits above the current range
				shakeEnd = dmxMax;
				foundPlace = true;
			}

			if (!foundPlace)
			{ 
				CI_LOG_W("Wheel shake at slot index " << slotIdx << " does not fit into a continous range of wheek shakes so it will be ignored!");
				notes.push_back("Wheel shake at slot " + std::to_string(slotIdx + 1) + " (dmx Values "+std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + ") will be unavailable because it's not in a continous dmx range.");
				continue;
			}
		}
	}

	if (amount < 1 || goboStart < 0 || goboEnd < 0 || goboEnd < goboStart)
		throw std::exception(("Gobo Wheel at dmx offset" + std::to_string(dmxOffset) + " did not result in plausible values, aborting!").c_str());

	descPatch["goboMap"] = ci::Json::object();
	descPatch["goboMap"]["amount"] = amount;
	descPatch["goboMap"]["start"] = goboStart;
	descPatch["goboMap"]["end"] = goboEnd;

	if (shakeStart >= 0 && shakeEnd >= 0 && shakeStart < shakeEnd)
	{
		descPatch["goboMap"]["shake-min"] = shakeStart;
		descPatch["goboMap"]["shake-max"] = shakeEnd;
	}

	descPatch["mapping"]["gobo"] = dmxOffset;

	if(!notes.empty())
		descPatch["notes"]["channel-" + std::to_string(dmxOffset)] = notes;

	return descPatch;
}

bool act::room::OFLDescriptionMapper::isShutterStrobeCapability(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	// Determine if there are capabilities with an open state and a dedicated strobe state with speedStart and speedEnd
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false;

	bool hasOpenState = false;
	bool hasStrobeWithSpeed = false;

	for (auto const& capability : extChannelDesc["capabilities"])
	{
		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;
		
		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Open")
			hasOpenState = true;
		else if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Strobe"
			&& capability.contains("speedStart") && capability.contains("speedEnd"))
			hasStrobeWithSpeed = true;

		if (hasOpenState && hasStrobeWithSpeed)
			break;
	}

	return hasOpenState && hasStrobeWithSpeed;
}

ci::Json act::room::OFLDescriptionMapper::convertShutterStrobeCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("No capabilities array found in capabolities description!");

	ci::Json descPatch = ci::Json::object();

	// NOTE: Currently the strobe parameter of InACTually is rather limited 
	// so the only thing we can convert is a open state as no strobe and an strobe state
	// which is controling the speed directly
	// no strobe type like ramp up etc. or strobe speed controlled with different parameter is
	// possible at the moment

	int strobeIdx = -1;

	// search strobe capability
	for (int idx = 0; idx < extChannelDesc["capabilities"].size(); idx++)
	{
		auto const& capability = extChannelDesc["capabilities"][idx];

		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;

		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Strobe")
		{
			if (capability.contains("speedStart") && capability.contains("speedEnd"))
			{
				strobeIdx = idx;
				break;
			}
		}
	}

	if (strobeIdx < 0)
		throw std::exception(("Encountered incompatible strobe capabilities list at dmxOffset " + std::to_string(dmxOffset)).c_str());

	// find neares open position
	ci::Json const& strobeCapability = extChannelDesc["capabilities"][strobeIdx];
	if (!strobeCapability.contains("dmxRange") || !strobeCapability["dmxRange"].is_array() || strobeCapability["dmxRange"].size() != 2)
		throw std::exception("Could not find well formatted dmxRange in strobe capability!");

	int openStateIdxAndDistance[2] = { -1, -1 };

	for (int idx = 0; idx < extChannelDesc["capabilities"].size(); idx++)
	{
		auto const& capability = extChannelDesc["capabilities"][idx];

		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;

		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Open")
		{
			if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2)
			{
				CI_LOG_W("Found capability with ShutterEffect Open but malformed dmxRange! Skipping capability.");
				continue;
			}

			int newDistance = abs(ofl::OFLHelper::dmxRangeDistance(capability["dmxRange"], strobeCapability["dmxRange"]));
			if (openStateIdxAndDistance[0] < 0 || newDistance < openStateIdxAndDistance[1])
			{
				openStateIdxAndDistance[0] = idx;
				openStateIdxAndDistance[1] = newDistance;
			}


			if (openStateIdxAndDistance[1] == 1)
				break; // If the distance is 1 the OpenState is directly next to the strobeState
					   // So we don't have to look further
		}
	}

	if (openStateIdxAndDistance[0] < 0 || openStateIdxAndDistance[1] < 0)
		throw std::exception(("Did not found any open state for strobe at dmxOffset: "+std::to_string(dmxOffset)+" skipping strobe!").c_str());

	descPatch["strobeMap"] = ci::Json::object();
	int minValue, maxValue = -1;
	int noneValue = ofl::OFLHelper::dmxRangeToDmxValue(extChannelDesc["capabilities"][openStateIdxAndDistance[0]]["dmxRange"]);
	descPatch["strobeMap"]["none"] = noneValue;
	
	// Check if speedStart is lower than speedEnd
	std::string str_speedStart = strobeCapability["speedStart"].get<std::string>();
	std::string str_speedEnd = strobeCapability["speedEnd"].get<std::string>();
	float speedStart, speedEnd;

	if (str_speedStart == "slow")
		speedStart = -2;
	else if (str_speedStart == "fast")
		speedStart = -1;
	else
		speedStart = ofl::OFLHelper::speedToFloat(str_speedStart);

	if (str_speedEnd == "slow")
		speedEnd = -2;
	else if (str_speedEnd == "fast")
		speedEnd = -1;
	else
		speedEnd = ofl::OFLHelper::speedToFloat(str_speedEnd);

	if (speedStart < speedEnd)
	{
		minValue = strobeCapability["dmxRange"][0].get<int>();
		maxValue = strobeCapability["dmxRange"][1].get<int>();

		if(speedEnd > 0)
			descPatch["strobeSpeed"] = speedEnd;
	}
	else
	{
		minValue = strobeCapability["dmxRange"][1].get<int>();
		maxValue = strobeCapability["dmxRange"][0].get<int>();

		if (speedStart > 0)
			descPatch["strobeSpeed"] = speedStart;
	}
	descPatch["strobeMap"]["min"] = minValue;
	descPatch["strobeMap"]["max"] = maxValue;

	descPatch["mapping"]["strobe"] = dmxOffset;
	std::list<std::string> notes = std::list<std::string>();
	notes.push_back("Strobe is limited to none, min and max!");
	notes.push_back("Using dmx values: no-strobe="+std::to_string(noneValue)+", min="+ std::to_string(minValue)+", max="+ std::to_string(maxValue));
	descPatch["notes"]["channel-"+std::to_string(dmxOffset)] = notes;

	return descPatch;
}
