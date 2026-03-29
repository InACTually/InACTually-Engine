/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
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

		class VideoMixerProcNode : public ProcNodeBase
		{
		public:
			VideoMixerProcNode();
			~VideoMixerProcNode();

			PROCNODECREATE(VideoMixerProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<image> m_output;

			float m_mixValue = 0.0f;
			image m_primaryImage;
			image m_secondaryImage;
			image m_mixedImage;

			void mix();

		}; using VideoMixerProcNodeRef = std::shared_ptr<VideoMixerProcNode>;

	}
}