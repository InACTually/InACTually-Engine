
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

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class MovementDetectionProcNode : public ProcNodeBase
		{
		public:
			MovementDetectionProcNode();
			~MovementDetectionProcNode();

			PROCNODECREATE(MovementDetectionProcNode);

			void init();
			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			ci::gl::Texture2dRef	m_texture;
			float	m_displayScale;
			float   m_resizeScale;
			bool	m_visualize;

			float m_movementThreshold;


			cv::Ptr<cv::BackgroundSubtractor> m_bgSub;
			bool	m_useMOG2;
			bool	m_useKNN;
			float	m_movementValue;

			cv::UMat	m_previous;

			OutputPortRef<cv::UMat>	m_imagePort;
			OutputPortRef<float>	m_movementPort;
			OutputPortRef<bool>		m_isMovingPort;

		}; using MovementDetectionProcNodeRef = std::shared_ptr<MovementDetectionProcNode>;

	}
}