
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
	Fabian T�pfer
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include <opencv2/dnn/dnn.hpp>

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class FaceEmotionProcNode : public ProcNodeBase
		{
		public:
			FaceEmotionProcNode();
			~FaceEmotionProcNode();

			PROCNODECREATE(FaceEmotionProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

		private:
			ci::gl::Texture2dRef												m_texture;
			ci::ivec2															adaptSize(ci::ivec2 size);
			float																m_displayScale;

			OutputPortRef<cv::UMat>												m_imagePort;
			OutputPortRef<std::pair<std::string, float>>						m_emotionPort;

			static bool															m_registered;

			bool																m_show = false;

			std::vector<std::string>											m_emotions;
			cv::dnn::Net														m_network;

			std::pair<std::string, float>										m_currentEmotion;
		
			cv::Mat																detectCurrentEmotions(cv::UMat frame);
			std::pair<std::string, float>										getCurrentEmotion(std::vector<float> result);

			std::vector<float>													softmax(cv::Mat* mat);

		}; using FaceEmotionProcNodeRef = std::shared_ptr<FaceEmotionProcNode>;

	}
}