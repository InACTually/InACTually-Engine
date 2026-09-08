
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

#pragma once

#include "RoomNodeManagerBase.hpp"
#include "dmx/DMXRoomNodeBase.hpp"
#include "dmx/MovingHeadRoomNode.hpp"
#include "dmx/DimmerRoomNode.hpp"
#include "light/FixtureDescriptionImporter.hpp"

#include "dmx/DMXPro.hpp"

namespace act {
	namespace room {

		class DMXManager : public RoomNodeManagerBase, public std::enable_shared_from_this<DMXManager> {
		public:
			DMXManager();
			~DMXManager();

			static	std::shared_ptr<DMXManager> create() { return std::make_shared<DMXManager>(); };

			void	setup() override;
			void	update() override;
			// void	draw() override;
			void	cleanUp() override;

			act::room::RoomNodeBaseRef drawMenu() override;


			act::room::MovingHeadRoomNodeRef getMovingHeadByIndex(int index);
			act::room::RoomNodeBaseRef getDeviceByMarkerID(int id);
			act::room::DimmerRoomNodeRef getDimmerByIndex(int index);

			std::vector<std::string> getFixtureNames() { return m_availableDeviceNames; }
			int hasAvailableDevice(std::string deviceName);


			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);
			void saveDevicesToJson();
			void importFixture(ci::Json fixtureDescription);
			act::room::RoomNodeBaseRef addDevice(std::string name, int fixtureIndex, int startAddress);
 
		private:
			DMXProRef							m_dmxInterface;
			int									m_selectedInterface;
			void refreshInterfaceNames();
			std::vector<std::string>			m_interfaceNames;
			void changeInterface(std::string interfaceName);

			void loadFixtures();
			void saveFixtures();
			int getFixtureIndexByName(std::string fixtureName);
			std::vector<ci::Json>				m_fixtureDescriptions;
			std::vector<std::string>			m_fixtureNames;
			int									m_selectedFixture;

			act::system::FixtureDescriptionImporterRef m_fixtureDescriptionImporter;

			void refreshLists() override;
			std::vector<std::string>			m_availableDeviceNames;
			int									m_currentAddress;

		
		};
		using DMXManagerRef = std::shared_ptr<DMXManager>;
	}
}