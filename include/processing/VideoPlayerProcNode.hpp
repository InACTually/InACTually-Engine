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
#include "MatListener.hpp"

#include "camera/CameraManager.hpp"
#include "cinder/qtime/QuickTimeGl.h"

using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class VideoPlayerProcNode : public ProcNodeBase
		{
		public:
			VideoPlayerProcNode();
			~VideoPlayerProcNode();

			PROCNODECREATE(VideoPlayerProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onTrigger(bool event);
			ImageOutputPortRef getVideoOutPort() { return m_videoImageOutPort; };
			void seek(number playPosition);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			void loadVideo(std::string path);

			void setIsLooping(bool isLooping)	{ m_isLooping = isLooping; };
			void setIsResuming(bool isResuming) { m_isResuming = isResuming; };

		private:
			ci::SurfaceRef			m_videoSurface;
			ci::gl::Texture2dRef	m_videoTexture;

			ci::ivec2				m_videoSize;

			ImageOutputPortRef		m_videoImageOutPort;
			
			std::string				m_path;
			qtime::MovieGlRef		m_inputVideo;
			bool					m_isOpenDialog;
			bool					m_isPlaying;
			bool					m_isLooping;
			bool					m_isResuming;
		};

		using VideoPlayerProcNodeRef = std::shared_ptr<VideoPlayerProcNode>;

	}
}