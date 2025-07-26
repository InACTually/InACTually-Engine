
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
#include <opencv2/dnn/dnn.hpp>

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class ObjectDetectionProcNode : public ProcNodeBase
		{
		public:
			ObjectDetectionProcNode();
			~ObjectDetectionProcNode();

			PROCNODECREATE(ObjectDetectionProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

		private:
			ci::gl::Texture2dRef				m_texture;
			float								m_displayScale;

			OutputPortRef<cv::UMat>				m_imagePort;
			OutputPortRef<cv::UMat>				m_detectionImagePort;
			OutputPortRef<featureList>			m_featureListPort;

			float								m_minConfidence;

			static bool							m_registered;

			bool								m_show;

			void								initNetwork();
			featureList							m_currentObjects;

			cv::dnn::Net						m_network;
			std::vector<std::string>			m_names;
			std::string							m_outputLayer;
			std::vector<std::string>			m_classes;

			std::thread							m_thread;
			cv::UMat							m_processedFrame;
			std::atomic<bool>					m_isProcessing;
			std::atomic<bool>					m_isProcessingDone;
			std::vector<cv::Mat>				m_detection;

			std::vector<std::string>			getOutputsNames(const cv::dnn::Net& net);
			std::vector<cv::Rect>				processDetection(cv::UMat& frame, const std::vector<cv::Mat>& outs);
			void								drawBox(std::string className, float conf, int left, int top, int right, int bottom, cv::UMat& frame);

		}; using ObjectDetectionProcNodeRef = std::shared_ptr<ObjectDetectionProcNode>;

	}
}