
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		enum DistributionOptions {
			DO_ALL = 0,
			DO_ROUNDROBIN,
			DO_RANDOM
		};

		class DistributorProcNode : public ProcNodeBase
		{
		public:
			DistributorProcNode();
			~DistributorProcNode();

			PROCNODECREATE(DistributorProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			std::vector<OutputPortRef<bool>> m_outputs;
			std::vector<std::string> m_options;
			int m_selectedOption = 0;

			int currentRRIndex = -1;

			void distribute(bool val);

		}; using DistributorProcNodeRef = std::shared_ptr<DistributorProcNode>;

	}
}