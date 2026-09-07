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


#pragma once

#include "roompch.hpp"
#include "dmx/OFLHelper.hpp"

namespace act {
	namespace room {
		class OFLDescriptionMapper
		{
		public:
			OFLDescriptionMapper();
			~OFLDescriptionMapper();

			static std::shared_ptr<OFLDescriptionMapper> create() { return std::make_shared<OFLDescriptionMapper>(); };

			std::string getName();
			bool searchLibraryPath();
			ci::fs::path getLibraryPath();
			bool setLibraryPath(ci::fs::path path);
			bool getIsParsed();
			std::vector<ofl::OFLManufacturerRef> getManufacturers(bool allowParsing);

			// Parse OFL meta information like which manufacturers there are and which fixtures they contain
			bool parseLibraryMeta();

			// Parse description of an OFL fixture like name and name of modes
			// calling convertOFLModeToInternal for each mode
			// returns true if successful, false if errors occure, errors get logged
			bool parseFixtureDescription(ofl::OFLFixtureDescriptionRef fixtureDescription);

			// Converts the mode description of a fixture into an internal representation and sets it to the fixtureModeRef
			// throws exceptions on errors
			bool convertOFLModeToInternal(ci::Json const& modeDesc, ofl::OFLFixtureDescriptionRef fixtureDescription, ofl::OFLModeRef fixtureModeRef, int modeIndex);

			// Returns one combined inACTually internal fixture description 
			// depending on which channels are selected to be included
			static ci::Json getInternalDescription(ofl::OFLFixtureDescriptionRef fixture);

		private:
			std::string m_ManufacturerIdxFileName = "manufacturers.json";
			ofl::OpenFixtureLibrary m_ofl;

			//Converts a channel into a json patch of the internal description calling the corresponding convert methods
			ci::Json convertChannel(ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);
			
			// Checks all channels in mapping of description patch and adds the descriptionRef to those channels that do not have an internal description jet
			// The mapping with the primaryDMXOffset will be ignored because it should already have the description
			void addDescToOtherAffectedChannels(ofl::OFLModeRef modeRef, ofl::OFLChannelDescPatchRef channelDescPatchRef, std::string const& channelKey, int primaryDmxOffset);

			// Takes fineChannelAliases array and resolves them to dmx Offset in the channels List of the given mode
			// Returns map of fine channel alias with corresponding int or -1 if fineChannelAlias could not be resolved
			std::map<std::string, int> resolveFineChannels(ci::Json const& fullExtDesc, int mode, ci::Json const& extChannelDesc, int maxAliases = 1);

			/* Convert... methods should all use the same signature to provide fast access to relevant context information
			 * (  ci::Json const& extCapabilityDesc	reference to the capability which should be converted
				, ci::Json const& extChannelDesc	reference to the full channel description in which the capability to convert is used
				, ci::Json const& fullExtDesc		reference to the full external description if other information is needed
				, int dmxOffset						dmx offset = key of the channel in the channels list of the mode
				, int modeIndex						key of the mode to convert
				, std::string const& channelName	Optional, for converting capabilities
			  )
			*/
			ci::Json convertIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertPanTiltCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertZoomCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertColorIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertPanTiltSpeedCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			
			bool isColorWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertColorWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			bool isGoboWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertGoboWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			bool isShutterStrobeCapability(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json convertShutterStrobeCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

		}; using OFLDescriptionMapperRef = std::shared_ptr<OFLDescriptionMapper>;
	}
}