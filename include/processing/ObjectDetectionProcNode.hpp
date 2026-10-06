
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
#include <opencv2/dnn/dnn.hpp>


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
			TextureHelperRef					m_textureHelper;
			float								m_displayScale;

			ImageOutputPortRef					m_imagePort;
			ImageOutputPortRef					m_detectionImagePort;
			OutputPortRef<featureList>			m_featureListPort;

			float								m_minConfidence;

			static bool							m_registered;

			bool								m_show;

			void								initNetwork();
			featureList							m_currentObjects;

			cv::Size							m_inputSize;
			cv::dnn::Net						m_network;
			std::vector<std::string>			m_classes;
			std::vector<int>					m_strides;
			cv::Mat								m_expandedStrides;
			cv::Mat								m_grids;

			float								m_nmsThreshold;
			float								m_objThreshold;

			void								generateAnchors();

			cv::UMat							preprocess(cv::UMat frame, cv::Size targetSize, float ratio);
			cv::Mat								detect(cv::UMat blob);
			cv::Mat								postprocess(cv::Mat	outputs);
			std::vector<cv::Rect>				processDetection(cv::UMat frame, const cv::Mat& outputs, float scaleFactor);
			void								drawBox(cv::UMat& frame, cv::Rect box, std::string className, float confidence);

		}; using ObjectDetectionProcNodeRef = std::shared_ptr<ObjectDetectionProcNode>;

	}
}