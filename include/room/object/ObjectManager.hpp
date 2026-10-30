
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
#include "object/ObjectRoomNode.hpp"
#include "camera/CameraRoomNode.hpp"

#include "computing/ObjectDetector.hpp"


namespace act {
	namespace room {


		class ObjectManager : public RoomNodeManagerBase {
		public:
			ObjectManager();
			ObjectManager(CameraManagerRef cameraMgr);
			~ObjectManager();

			static	std::shared_ptr<ObjectManager> create(CameraManagerRef cameraMgr) { return std::make_shared<ObjectManager>(cameraMgr); };

			void update() override;
			void	setup() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			ObjectRoomNodeRef addObject(act::comp::ObjectCandidate candidate);

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);


		private:
			

			void checkCameras();
			CameraManagerRef m_cameraMgr;
			
			void processObjects();

			glm::ivec2 m_displaySize = glm::ivec2(640, 360);

			std::map<act::UID, comp::ObjectDetectorRef> m_objectDetectors;

			std::vector<cv::ogl::Texture2D> m_feedbackTextures;
		};
		using ObjectManagerRef = std::shared_ptr<ObjectManager>;
	}
}
