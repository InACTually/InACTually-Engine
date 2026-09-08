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
	namespace system {
		
		/*
		* ImportFixtureListener is an interface requiring an importFixture function which
		* can be called by the FixtureDescriptionImporter upon an import request providing an 
		* internal fixture description
		*/
		class ImportFixtureListener {
		public:
			/*
			* Import the provided fixtureDescription
			*/
			virtual void importFixture(ci::Json fixtureDescription) = 0;
		};

	}
}