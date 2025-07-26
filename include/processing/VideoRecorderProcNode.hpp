
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"

#include "camera/CameraManager.hpp"

using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class VideoRecorderProcNode : public ProcNodeBase
		{
		public:
			VideoRecorderProcNode();
			~VideoRecorderProcNode();

			PROCNODECREATE(VideoRecorderProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			InputPortRef<cv::UMat> getVideoInPort() { return m_videoImageInPort; };
			void saveVideo(std::string path, int fps = app::getFrameRate(), bool isColor = true);

			void record();
			void stop();

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			ci::SurfaceRef			m_videoSurface;
			ci::gl::Texture2dRef	m_videoTexture;

			ci::ivec2				m_videoSize;

			InputPortRef<cv::UMat> m_videoImageInPort;

			void onImage(cv::UMat image);

			
			std::string				m_path;
			cv::VideoWriter 		m_outputVideo;
			bool					m_isSaveDialog;
			bool					m_isRecording;
		};

		using VideoRecorderProcNodeRef = std::shared_ptr<VideoRecorderProcNode>;

	}
}