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


namespace act {
	namespace room {
		namespace ofl {

			struct OFLChannelDescPatch {
				ci::Json descriptionPatch;
				bool isIncludePatch = true;
			}; using OFLChannelDescPatchRef = std::shared_ptr<OFLChannelDescPatch>;

			struct OFLChannel {
				std::string oflChannelKey;
				OFLChannelDescPatchRef channelDescPatch;
			}; using OFLChannelRef = std::shared_ptr<OFLChannel>;

			struct OFLMode {
				std::string name;
				std::map<int, OFLChannelRef> channelMapping; // cache for the converted dmx channel to internal parameter name
				ci::Json internalDescBase;
			}; using OFLModeRef = std::shared_ptr<OFLMode>;

			struct OFLFixtureDescription {
				act::UID uid;
				std::string name;
				ci::fs::path descriptionPath;
				ci::Json externalDescription;
				std::vector<OFLModeRef> modes;
				std::string type;
				bool isSupportedType = false;
				bool isForceConverted = false;
				int selectedMode;
				bool isQueuedForLoading = false;
				bool isQueuedForImport = false;
				bool hasError = false;
				bool isShowInListing = true; // used for filtering in UI
			}; using OFLFixtureDescriptionRef = std::shared_ptr<OFLFixtureDescription>;

			struct OFLManufacturer {
				std::string key;
				std::string name;
				std::vector<OFLFixtureDescriptionRef> fixtures;
				bool isShowInListing = true; // used for filtering UI
				bool isExpandInListing = false; // used to automatically and show fixtures in UI
			}; using OFLManufacturerRef = std::shared_ptr<OFLManufacturer>;

			/** @brief The OpenFixtureLibrary struct holds a vector of manufacturers
			* which each hold a vector of fixtures
			* each holding a vector of modes
			*/
			struct OpenFixtureLibrary {
				std::string name;
				ci::fs::path libraryPath;
				std::vector<OFLManufacturerRef> manufacturers{};
				bool isParsed;
			}; using OpenFixtureLibraryRef = std::shared_ptr<OpenFixtureLibrary>;

			/*
			* OFLHelper contains a collection of static methods used to handle the Open Fixture Library
			*/
			class OFLHelper
			{
			public:

				// Adds a note at internalDescPatch["notes"]["channel-<dmxOffsett>"]
				static void attachNoteToChannelDesc(ci::Json& internalDescPatch, int dmxOffset, std::string note)
				{
					if (!internalDescPatch.contains("notes"))
						internalDescPatch["notes"] = ci::Json::object();

					std::string channelKey = "channel-" + std::to_string(dmxOffset);

					if (internalDescPatch["notes"].contains(channelKey) && !internalDescPatch["notes"][channelKey].is_array())
					{
						// There is already a notes list for that channel but it is not an array
						CI_LOG_E("could not attach note to channel " << channelKey << " because there are notes but not as array! note:" << note);
						return;
					}
					else
					{
						// There is no notes list jet so we create an array
						internalDescPatch["notes"][channelKey] = ci::Json::array();
					}
					internalDescPatch["notes"][channelKey].push_back(note);
				};

				// Checks if there is a wheelName specified for the capabilities list, returns the channelname otherwise
				// WheelName can be channel name or stated explicitly in the description
				static std::string getWheelNameKey(std::string const& channelName, ci::Json const& extCapabilities)
				{
					return (extCapabilities.contains("wheel") && extCapabilities["wheel"].is_string()) ? extCapabilities["wheel"].get<std::string>() : channelName;
				};

				// Converts a degree string into an integer
				// NOTE: angles in OFL can also be in % but since inACTually currently can not handle 
				// proportional angles it wont be converted but result in an exception
				static int convertDegStrToInt(std::string degStr, std::string decimalpoint = ".")
				{
					size_t degPos = degStr.find("deg");

					if (degPos == std::string::npos) throw std::invalid_argument("degStr does not contain 'deg'!");
					if (degStr.length() - degPos != 3) throw std::invalid_argument("degStr does not contain 'deg' as last three characters!");

					degStr.erase(degStr.length() - 3, 3);

					if (degStr.find(decimalpoint) != std::string::npos)
						return static_cast<int>(std::round(std::stof(degStr)));
					else
						return std::stoi(degStr);
				};

				// Tries to convert a BeamAngle into a degree range
				// BeamAngles can be in deg or in % (also specified as 0% = "closed", 1% = "narrow, 100% = "wide"
				// If a % BeamAngle is found the function looks if deg can be retrived from the physical lens description
				static int beamAngleToDegRange(std::string const& beamAngle, int modeIdx, ci::Json const& fullExtDesc) {
					if (beamAngle.find("deg") != std::string::npos)
						return convertDegStrToInt(beamAngle);

					// we dont have the degree directly specified so lets try to find the physical description of the lens
					ci::Json physicalLensDesc = ci::Json();
					// in rare cases it can be mode specific so lets search there first
					if (fullExtDesc.contains("modes") && modeIdx >= 0 && fullExtDesc["modes"] > modeIdx
						&& fullExtDesc["modes"][modeIdx].contains("physical")
						&& fullExtDesc["modes"][modeIdx]["physical"].contains("lens"))
					{
						physicalLensDesc = fullExtDesc["modes"][modeIdx]["physical"]["lens"];
					}
					else if (fullExtDesc.contains("physical") && fullExtDesc["physical"].contains("lens")) // otherwise look for a physical description at first level
					{
						physicalLensDesc = fullExtDesc["physical"]["lens"];
					}
					else
						throw std::invalid_argument("beamAngle does not specify deg explicit or implicit (by a physical lens description).");

					// check for degreesMinMax
					if (!physicalLensDesc.is_object()
						|| !physicalLensDesc.contains("degreesMinMax")
						|| !physicalLensDesc["degreesMinMax"].is_array()
						|| physicalLensDesc["degreesMinMax"].size() != 2)
					{
						throw std::invalid_argument("beamAngle does not specify deg explicit or implicit (physical lens description does not contain degreesMinMax).");
					}

					if (!physicalLensDesc["degreesMinMax"][0].is_number() || !physicalLensDesc["degreesMinMax"][1].is_number())
						throw std::exception("physical lens description contains non number degreesMinMax!");

					if (physicalLensDesc["degreesMinMax"][0] > physicalLensDesc["degreesMinMax"][1])
						throw std::exception("physical lens description degreesMinMax in wrong order!");

					// Determine which degree to take
					// NOTE: possible float to int conversion but InACTually currently can not handle float zoom values
					if (beamAngle == "narrow" || beamAngle == "1%")
						return physicalLensDesc["degreesMinMax"][0];
					else if (beamAngle == "wide" || beamAngle == "100%")
						return physicalLensDesc["degreesMinMax"][1];
					else if (beamAngle == "closed")
						return 0;
					else
						throw std::exception("Could not map physical lens description degreesMinMax to a degree!");
					// NOTE: x% outside of 1% and 100% is not mapped to specifies range because values other than those
					// three were not present for beam angles as of writing this function
					// they would however be spec-compliant

				};

				// converts an ofl speed string into a float
				static float speedToFloat(std::string speed)
				{
					// Speed can have Hz, bpm or % as units
					size_t HzPos = speed.find("Hz");
					if (HzPos != std::string::npos)
						speed.erase(HzPos, 2);

					size_t bpmPos = speed.find("bpm");
					if (bpmPos != std::string::npos)
						speed.erase(bpmPos, 3);

					size_t percPos = speed.find("%");
					if (percPos != std::string::npos)
						speed.erase(percPos, 1);

					return std::stof(speed);
				};

				// Calculates the distance in address offset from baseRange to distantRange
				static int dmxRangeDistance(ci::Json const& distantRange, ci::Json const& baseRange)
				{
					if (!distantRange.is_array() || !baseRange.is_array()
						|| distantRange.size() != 2 || baseRange.size() != 2)
						throw new std::invalid_argument("Malformed dmxRanges!");

					if (distantRange[1] < baseRange[0])
						return distantRange[1].get<int>() - baseRange[0].get<int>(); // distantRange below baseRange
					if (baseRange[1] < distantRange[0])
						return distantRange[0].get<int>() - baseRange[1].get<int>(); // disntantRange above baseRange 

					return 0; // distantRange within baseRange (/or overlapping)
				};

				// Averages a dmx range to one dmx value
				static int dmxRangeToDmxValue(ci::Json const& dmxRange)
				{
					if (!dmxRange.is_array() || dmxRange.size() != 2)
						throw std::invalid_argument("Malformed dmxRange!");

					return std::round((dmxRange[0].get<int>() + dmxRange[1].get<int>()) * 0.5);
				};
			};
		}
	}
}