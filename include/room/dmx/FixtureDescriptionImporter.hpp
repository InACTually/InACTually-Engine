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
#include "dmx/OFLDescriptionMapper.hpp"


namespace act {
	namespace room {

		/* 
		* Forward declaration, DMXManager is observer of FixtureDescriptionImporter
		*/
		class DMXManager;

		class FixtureDescriptionImporter
		{
		public:
			FixtureDescriptionImporter();
			~FixtureDescriptionImporter();

			static std::shared_ptr<FixtureDescriptionImporter> create() { return std::make_shared<FixtureDescriptionImporter>(); };

			void	draw();
			void	update();
			void	drawOFLImport();
			void	drawOFLFixtureDetails(ofl::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId);
			void	drawOFLFixtureTable(ofl::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId);
			
			/* Register function for observer.
			*  Should provide importFixture(ci::Json) function, which will be called upon a fixture import. 
			*/
			void	registerDMXManager(std::weak_ptr<DMXManager> dmxManagerWRef);

			bool m_showImporter;

		private:

			std::map<std::string, std::string> m_oflJsonDmpCache;
			std::list<ofl::OFLFixtureDescriptionRef> m_oflFetchFixtureQueue;
			std::list<ofl::OFLFixtureDescriptionRef> m_importQueue;
			bool m_openOFLInBrowser = false;
			bool m_searchOFLAgain = false;
			bool m_oflReparse = false;
			bool m_isOFLListFilteres = false;
			bool m_oflFixtureFilterChanged = false;
			char m_oflFixtureFilterBuffer[128] = "";
			std::weak_ptr<DMXManager> m_dmxManagerWRef;

			void filterOFLFixtures();
			OFLDescriptionMapperRef m_oflDescriptionMapper;

		}; using FixtureDescriptionImporterRef = std::shared_ptr<FixtureDescriptionImporter>;
	}
}