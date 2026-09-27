
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


namespace act {
	namespace proc {

		class FaceDetectionProcNode : public ProcNodeBase
		{
		public:
			FaceDetectionProcNode();
			~FaceDetectionProcNode();

			PROCNODECREATE(FaceDetectionProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			float	m_resizeScale;
			bool	m_isFixingFaceSize;
			int		m_fixedFaceSize; // in px, quadratic

			bool	m_show;
			int		m_faceAvailHeightThreshold;
			int		m_faceAvailHistoryMaxSize;
			int		m_faceAvailHistoryThreshold;
			std::deque<int> m_faceAvailHistory;

			cv::Ptr<cv::FaceDetectorYN>				m_model;
			std::vector<ci::Rectf>					m_faces;
			std::deque<std::vector<ci::Rectf>>		m_facesHistory;
			int										m_facesHistorySize;

			TextureHelperRef						m_textureHelper;

			ImageOutputPortRef	m_faceImagePort;
			OutputPortRef<bool>		m_faceAvailablePort;

		}; using FaceDetectionProcNodeRef = std::shared_ptr<FaceDetectionProcNode>;

	}
}