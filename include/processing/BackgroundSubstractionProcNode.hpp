
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

#include "opencv2/core.hpp"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/imgproc.hpp"


namespace act {
	namespace proc {

		class BackgroundSubstractionProcNode : public ProcNodeBase
		{
		public:
			BackgroundSubstractionProcNode();
			~BackgroundSubstractionProcNode();

			PROCNODECREATE(BackgroundSubstractionProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			bool valuesChanged;

			int m_historyLength;
			float m_threshold;
			bool m_detectShadows;
			float m_learningRate;

			ci::gl::Texture2dRef	m_texture_bg;
			ci::gl::Texture2dRef	m_texture_fgMask;
			ci::gl::Texture2dRef	m_texture_bgMask;
			ci::gl::Texture2dRef	m_texture_fgCutout;
			ci::gl::Texture2dRef	m_texture_bgCutout;

			cv::Ptr<cv::BackgroundSubtractor> m_bgModel;
			cv::UMat	m_foregroundMask;
			cv::UMat	m_backgroundMask;
			cv::UMat	m_foregroundCutout;
			cv::UMat	m_backgroundCutout;
			cv::UMat	m_backgroundImg;

			ImageOutputPortRef	m_fgMaskPort;
			ImageOutputPortRef	m_fgCutoutPort;
			ImageOutputPortRef	m_bgCutoutPort;

		}; using BackgroundSubstractionProcNodeRef = std::shared_ptr<BackgroundSubstractionProcNode>;

	}
}