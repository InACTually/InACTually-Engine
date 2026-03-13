
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


#include "RoomNodeBase.hpp"
#include "dmx/DMXRoomNodeBase.hpp"

#include "dmx/DMXPro.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace room {

		class MovingHeadRoomNode : public RoomNodeBase, public DMXRoomNodeBase
		{
		public:
			MovingHeadRoomNode(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID = "");
			virtual ~MovingHeadRoomNode();

			static std::shared_ptr<MovingHeadRoomNode> create(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, ci::vec3 position = ci::vec3(0.0f, 1.0f, 0.0f), ci::vec3 rotation = ci::vec3(0.0f, 0.0f, 0.0f), float radius = 0.5f, act::UID replyUID = "") { return std::make_shared<MovingHeadRoomNode>(dmxInterface, description, name, startAddress, position, rotation, radius, replyUID); };

			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;
			virtual void cleanUp()  override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

			void setColor(ci::Color color, bool publish = true);
			void lookAt(ci::vec3 at);

			void setPan(float pan);
			void setTilt(float tilt);
			void setPanTilt(float pan, float tilt);
			void setDimmer(float dim, bool publish = true);
			void setSpeed(float speed);
			void setZoom(float zoom, bool publish = true);
			void setUV(float uv, bool publish = true);
			void setStrobe(float strobe, bool publish = true);

			void home();

			inline bool hasZoom()	{ return m_hasZoom; };
			inline bool hasUV()		{ return m_hasUV; };
			inline bool hasAmber()	{ return m_hasAmber; };
			inline bool hasStrobe() { return m_hasStrobe; };
			void highlight();
			void release();

		private:
			ci::CameraPersp			m_cameraPersp;

			bool m_hasFineAdjust	= true;
			bool m_hasWhite			= false;
			bool m_hasAmber			= false;
			bool m_hasUV			= false;
			bool m_hasZoom			= false;
			bool m_hasColorWheel	= false;
			bool m_hasStrobe		= true;
			bool m_hasGobo			= false;

			int m_panRange			= 360;
			int m_tiltRange			= 180;
			int m_tiltOffset		= 0;
			int m_panCenterOffset	= 180;
			int m_tiltCenterOffset	= 90;
			int m_beamAngle			= 10; 
			int m_beamAngleMin		= 10;
			int m_beamAngleMax		= 10;
			int m_strobeSpeed		= 25;

			int m_panShift			= 0;
			int m_tiltShift			= 0;


			FilterBaseRef<glm::vec3>	m_lookAtFlt;

			ci::quat					m_gaze;
			ci::quat					m_previousGaze;

			float						m_yaw;		// radians
			float						m_pitch;	// radians
			float						m_phi;		// radians
			float						m_theta;	// radians

			util::MinMaxValue<float>	m_pan;		// degree
			util::MinMaxValue<float>	m_tilt;		// degree
			util::MinMaxValue<float>	m_dimmer;
			util::MinMaxValue<float>	m_dimmerMul;
			util::MinMaxValue<float>	m_dimmerPreHighlight;
			util::MinMaxValue<float>	m_speed;
			util::MinMaxValue<float>	m_strobe;
			util::MinMaxValue<float>	m_zoom;
			util::MinMaxValue<float>	m_UV;
			util::MinMaxValue<int>		m_gobo;
			util::MinMaxValue<float>	m_goboShake;
			ci::Color					m_color;
			ci::Color					m_colorPreHighlight;

			bool						m_isPanFlipped	= false;
			bool						m_isTiltFlipped = false;

			std::map<int, int>			m_colorWheelLookUp; // hue to chnVal
			void createColorWheelLookUp(ci::Json colorMap);
			bool						m_hasWhiteColorWheel;
			int							m_whiteColorWheelValue;

			bool polarToPanTilt();
			void panTiltToPolar();

			void panTo(float pan);
			void tiltTo(float tilt);

			virtual void onPosition(act::UID replyUID = "", bool publish = true) override;

		}; using MovingHeadRoomNodeRef = std::shared_ptr<MovingHeadRoomNode>;
		
	}
}