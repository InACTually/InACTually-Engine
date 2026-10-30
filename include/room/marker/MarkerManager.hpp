
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"

#include "camera/CameraManager.hpp"
#include "marker/MarkerRoomNode.hpp"
#include "camera/CameraRoomNode.hpp"

#include "computing/MarkerDetector.hpp"


namespace act {
	namespace room {

		struct MarkerOccurence {
			MarkerOccurence(int id) {
				this->id = id;
			}
			int id;
			int occurence = 0;
			bool wasInLastFrame = true;
		};

		class MarkerManager : public RoomNodeManagerBase {
		public:
			MarkerManager();
			MarkerManager(CameraManagerRef cameraMgr);
			~MarkerManager();

			static	std::shared_ptr<MarkerManager> create(CameraManagerRef cameraMgr) { return std::make_shared<MarkerManager>(cameraMgr); };

			void update() override;
			void	setup() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			MarkerRoomNodeRef addMarker(int id);

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);
			act::proc::OutputPortRef<glm::vec3> getMarkerPort(int markerID);

			std::vector<int> getMarkerIDs() { return m_markerIDs; }
			std::vector<std::string> getMarkerIDsAsStrings();


		private:
			
			MarkerRoomNodeRef getMarkerByID(int id);
			int m_originMarkerID = 0;
			float m_markerSize = 0.18f;

			void checkCameras();
			CameraManagerRef m_cameraMgr;
			
			void processMarkers();
			std::vector<int> m_markerIDs;

			glm::ivec2 m_displaySize = glm::ivec2(640, 360);

			std::map<act::UID, comp::MarkerDetectorRef> m_markerDetectors;

			std::vector<cv::ogl::Texture2D> m_feedbackTextures;
		};
		using MarkerManagerRef = std::shared_ptr<MarkerManager>;
	}
}
