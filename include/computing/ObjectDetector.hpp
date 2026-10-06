
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"

#include "DetectorBase.hpp"
#include "camera/CameraRoomNode.hpp"
#if __has_include(<opencv2/aruco.hpp>)
#define WITHARUCO
#endif

#ifdef WITHARUCO
#include <opencv2/aruco.hpp>
#endif

//#include "ByteTrack/BYTETracker.h"


namespace act {
	namespace comp {

		struct ObjectOccurence {
			ObjectOccurence(int id) {
				this->id = id;
			}
			static std::shared_ptr<ObjectOccurence> create(int id) { return std::make_shared<ObjectOccurence>(id); }
			int id;
			int occurence = 0;
			bool wasInLastFrame = true;
		};

		struct ObjectCandidate {
			int id;
			float confidence;
			std::string className;
			int classID;
			ci::Rectf box;
		};

		class ObjectDetector : public DetectorBase<ObjectCandidate> {
		public:
			ObjectDetector();
			ObjectDetector(room::CameraRoomNodeRef camera);
			~ObjectDetector();

			static	std::shared_ptr<ObjectDetector> create(room::CameraRoomNodeRef camera) { return std::make_shared<ObjectDetector>(camera); };

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

		private:
			cv::Mat m_objPoints;
			void refreshObjPoints();
			
			
			cv::dnn::Net						m_network;
			std::vector<std::string>			m_names;
			std::string							m_outputLayer;
			std::vector<std::string>			m_classes;
			bool								m_isUsingTiny = false;
			void initNetwork();

			bool m_detecting = true;
			std::vector<room::CameraRoomNodeRef> m_cameras;

			float								m_minConfidence = 0.4f;
			std::vector<cv::Mat>				m_detection;
			cv::Size							m_blobSize;
			cv::UMat							m_blob;
			std::vector<int>					m_strides;
			cv::Mat								m_expandedStrides;
			cv::Mat								m_grids;

			float								m_nmsThreshold;
			float								m_objThreshold;

			void								generateAnchors();

			cv::UMat							preprocess(cv::UMat frame, cv::Size targetSize, float ratio);
			void								detect() override;
			cv::Mat								postprocess(cv::Mat	outputs);
			std::vector<std::string>			getOutputsNames(const cv::dnn::Net& net);
			void								processDetection(cv::UMat& frame, const cv::Mat& outs, float ratio);
			void								drawBox(std::string className, float conf, int left, int top, int right, int bottom, cv::UMat& frame);


			void checkOccurency(int id);


			//std::shared_ptr<byte_track::BYTETracker> m_tracker;
			void								trackObjects(std::vector<ObjectCandidate>& candidates);

			glm::ivec2 m_displaySize = glm::ivec2(640, 360);

			std::map<int, bool> m_validObjectIDs;
			std::map<int, std::shared_ptr<ObjectOccurence>> m_newObjectOccurence;

		};
		using ObjectDetectorRef = std::shared_ptr<ObjectDetector>;
	}
}
