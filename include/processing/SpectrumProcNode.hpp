
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"


using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class SpectrumProcNode : public ProcNodeBase
		{
		public:
			SpectrumProcNode();
			~SpectrumProcNode();

			PROCNODECREATE(SpectrumProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			bool							m_show;

			audio::MonitorSpectralNodeRef	m_monitorSpectralNode;
			number							m_volume = 0;
			numberList						m_spectrum;
			number							m_centroid;

			OutputPortRef<number>			m_volumeOutPort;
			OutputPortRef<numberList>		m_spectrumOutPort;
			OutputPortRef<number>			m_centroidOutPort;

		};

		using SpectrumProcNodeRef = std::shared_ptr<SpectrumProcNode>;

	}
}