
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Fabian Töpfer - baniaf@uber.space
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "computing/PythonAdaptor.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/imgproc.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class MediaPipeProcNode : public ProcNodeBase
		{
		public:
			MediaPipeProcNode();
			~MediaPipeProcNode();

			PROCNODECREATE(MediaPipeProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);


			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			act::comp::PythonAdpatorRef m_adaptor;
			OutputPortRef<cv::UMat>	m_outputPort;

			cv::UMat	m_input;
			cv::UMat	m_output;

		}; using MediaPipeProcNodeRef = std::shared_ptr<MediaPipeProcNode>;

	}
}