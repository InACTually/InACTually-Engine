/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/CameraUi.h"
#include "CinderOpenCV.h"

#include "RoomManagers.hpp"
#include "NetworkManager.hpp"
#include "ModuleBase.hpp"

#include "InteractionHelper.hpp"
#include "MouseRawListener.hpp"

#include "Stage.hpp"


namespace act {
	namespace mod {

		class RoomModule : public ModuleBase, public act::input::MouseListener
		{
		public:
			RoomModule();
			~RoomModule();

			void setup(act::room::RoomManagersRef roomMgrs, act::net::NetworkManagerRef networkMgr)	override;
			void cleanUp()	override;
			void update()	override;
			void draw()		override;

			void drawGUI()	override;

			void load(std::filesystem::path path) override;
			void save(std::filesystem::path path) override;

			void onMouseDown(ci::app::MouseEvent event);
			void onMouseMove(ci::app::MouseEvent event);
			void onMouseDrag(ci::app::MouseEvent event);
			void onMouseUp(ci::app::MouseEvent event);
			void onMouseWheel(ci::app::MouseEvent event);

			ci::Json toParams();
			void fromParams(ci::Json json);
			
			bool hasNodeWithUID(act::UID uid);
			act::room::RoomNodeBaseRef createRoomNode(ci::Json data, act::UID msgUID = "");
			act::room::RoomNodeBaseRef roomNodeFactory(std::string roomNodeName, cinder::Json params);
			bool updateRoomNode(ci::Json data, act::UID msgUID = "");
			bool deleteRoomNode(act::UID uid, act::UID msgUID = "");

			bool callRPC(act::UID uid, std::string functionName);

			ci::Json getFullDescription() override;

			std::vector<std::string> getNodeNames();

		protected:
			room::StageRef				m_stage;
			act::room::RoomManagersRef	m_roomMgrs;
			act::net::NetworkManagerRef m_networkMgr;

			ia::InteractionHelperRef	m_iaHelper;

			ci::CameraPersp				m_camera;
			ci::CameraUi				m_camUi;
			glm::vec3					m_lookAt;

			glm::vec2					m_mousePos;
			room::RoomNodeBaseRef		m_node;

			ci::Ray getMouseRay();
			void updateNodeAtMouse();

			//int getNodeIndexByUID(act::UID uid);

			ci::gl::FboRef			m_fbo;
			glm::vec2				m_fboSize;

			bool					isRoomEditorHovered;

			act::UID m_currentFollowUID;

			void setupFbo();

			void drawDevicePool();
			void drawCreateButton(std::string nodeName);
			void handleResize();

			void loadFromFile(ci::fs::path path);
			void saveToFile(ci::fs::path path);

		private:
			int m_selectedCopyPos = 0;

		};

		using RoomModuleRef = std::shared_ptr<RoomModule>;

	}
}