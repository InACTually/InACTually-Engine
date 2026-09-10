
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
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "Design.hpp"

//#include "modules/ModuleManager.hpp"
//#include "input/InteractionManager.hpp"
#include "modules/ModuleBase.hpp"

#include "input/InputManager.hpp"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "cinder/CinderImGui.h"

#include "cinder/Json.h"

#include <algorithm>
#include <functional>

#include "ModuleRegistry.hpp"

#include "NetworkManager.hpp"


/**
* @brief main namespace of act - everything from act is in this namespace
*
*/
namespace act {
	/**
	* @brief InACTually is the main-class of act and the entrance for the cinder app
	*/
	class InACTually : 
		public mod::ModuleRegistry {
	public:
		/**
		* @brief InACTually is the main-class
		*/
		InACTually();
		~InACTually();

		/**
		* @brief callback on main-window close
		* 
		*/
		void onClose();
		/**
		* @brief prepares for shutdown, is called after all windows are closed and right before the app is closed
		*
		*/
		void cleanup();
		/**
		* @brief update call in main-thread 
		*
		*/
		void update();
		/**
		* @brief main draw call
		*
		*/
		void draw();
		void drawMinimalGUI();
		void drawFullGUI();
		void drawShowGUIMenuEntry();
		void fileDrop(ci::app::FileDropEvent event);
		void resize()  const;

		/**
		* @brief passes a listener for Mouse-Events, without using signals directly
		*
		*/
		void setRawListener(std::shared_ptr<input::MouseRawListener> listener) { m_mouseRawListener = listener; }
		/**
		* @brief passes a listener for Touch-Events, without using signals directly
		*
		*/
		void setRawListener(std::shared_ptr<input::TouchRawListener> listener) { m_touchRawListener = listener; }
		/**
		* @brief passes a listener for Key-Events, without using signals directly
		*
		*/
		void setRawListener(std::shared_ptr<input::KeyRawListener> listener) { m_keyRawListener = listener; }

		/**
		* @brief if the setup in init() is finished onInitialised is called before update/draw cycle begins
		*
		*/
		void onInitialised(std::function<void(void)> callback) { m_initCallback = callback; }


	private:
		ci::app::AppBase* m_app;
		UID m_mainWindowUID;

		std::shared_ptr<input::MouseRawListener> m_mouseRawListener;
		std::shared_ptr<input::TouchRawListener> m_touchRawListener;
		std::shared_ptr<input::KeyRawListener>   m_keyRawListener;

		/**
		* @brief initializes (setup) act at startUp
		*
		*/
		void init();
		void initStyle();

		std::function<void(void)>			m_initCallback = []() {};

		//shared_ptr<ia::InteractionManager>	m_interactionMgr;
		//shared_ptr<input::InputManager>		m_inputMgr;
		//shared_ptr<mod::ModuleManager>		m_moduleMgr;

		act::net::NetworkManagerRef			m_networkMgr;

		act::room::RoomManagersRef			m_roomMgrs;

		bool								m_drawGUI;
		bool								m_prevDrawGUI;

		bool								m_drawDebug;

		ci::gl::TextureRef					m_splashScreenTex;

		ImFont*								m_font;

		std::function<void(std::filesystem::path)> m_getPathCallback;
		bool								m_isGettingOpenPath = false;
		bool								m_isGettingSavePath = false;
	};

}