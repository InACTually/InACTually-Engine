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
#include "light/ImportFixtureListener.hpp"
#include "light/OFLDescriptionMapper.hpp"


namespace act {
	namespace system {

		/*
		* The FixtureDescriptionImporter converts external fixture descriptions into the InACTually internal description format.
		* Description mappers are used to convert a specific external format into ours.
		*/
		class FixtureDescriptionImporter {
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
			void	registerImportFixtureListener(std::weak_ptr<ImportFixtureListener> listenerWRef);

			bool m_isShowImporter;

		private:

			std::map<std::string, std::string> m_oflJsonDmpCache;
			std::deque<ofl::OFLFixtureDescriptionRef> m_oflFetchFixtureQueue;
			std::deque<ofl::OFLFixtureDescriptionRef> m_importQueue;
			bool m_isOpenOFLInBrowser = false;
			bool m_isSearchOFLAgain = false;
			bool m_isOFLReparsing = false;
			bool m_isOFLListFilteres = false;
			bool m_hasOFLFixtureFilterChanged = false;
			char m_oflFixtureFilterBuffer[128] = "";
			std::weak_ptr<ImportFixtureListener> m_importFixtureListenerWRef;

			void filterOFLFixtures();
			OFLDescriptionMapperRef m_oflDescriptionMapper;

		}; using FixtureDescriptionImporterRef = std::shared_ptr<FixtureDescriptionImporter>;
	}
}