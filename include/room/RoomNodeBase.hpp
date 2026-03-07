
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "roompch.hpp"

using namespace ci;
using namespace ci::app;

#define ROOMNODECREATE(node) CREATE(node, RoomNodeBase);

namespace act {
	namespace room {
		
		class RoomNodeBase;

		struct ScopedReplyUID {
			ScopedReplyUID(act::UID* replyUID) : replyUID(replyUID) {};
			ScopedReplyUID() {};
			~ScopedReplyUID() { *replyUID = ""; };

			act::UID* replyUID;
		};

		class RoomNodeBase : public act::proc::PortContext, public act::UniqueIDBase, public net::RPCHandler
		{
		public:
			RoomNodeBase(std::string name, ci::vec3 position = ci::vec3(0.0f, 0.0f, 0.0f), ci::vec3 rotation = ci::vec3(0.0f, 0.0f, 0.0f), float radius = 0.2f, act::UID replyUID = "");
			virtual ~RoomNodeBase();

			virtual void setup() = 0;
			virtual void update() = 0;
			virtual void draw() = 0;
			virtual void cleanUp() {};

			virtual void drawSettings();
			virtual void drawSpecificSettings() {};

			int				getMarkerID() { return m_markerID; };
			void			setMarkerID(int id) { m_markerID = id; };

			ci::vec3		getPosition() { return m_position; }
			virtual void	setPosition(ci::vec3 position, bool publish = true);
			ci::vec3		getRotation() { return m_rotation; }		// in radians !
			virtual void	setRotation(ci::vec3 orientation, bool publish = true);			// in radians !
			ci::quat		getOrientation() { return m_orientation; }	
			virtual void	setOrientation(ci::quat orientation, bool publish = true);
			float			getRadius() { return m_radius; }
			void			setRadius(float radius);
			void			setTriMesh(ci::TriMeshRef triMesh);

			virtual void	lookAt(ci::vec3 at);
			ci::vec3		getLookAt() { return m_lookAt; };
			bool			isLookingAt();
			void			isLookingAt(bool isLooking);

			void			doSmoothing(bool smooth = true) { m_isSmoothing = true; };

			bool			isFixed() { return m_isFixed; };
			void			setIsFixed(bool val) { m_isFixed = val; };

			bool			isConnected() { return m_isConnected; };

			virtual ci::AxisAlignedBox getBounds() { return m_bounds; }

			virtual bool	hit(ci::vec3 pos);
			virtual bool	hitRay(ci::Ray ray);

			virtual	void	setIsHovered(bool hovered)					{ m_isHovered = hovered; }					// used directly by interacting with the room
			virtual	void	setIsHighlighted(bool highlighted)			{ m_isHighlighted = highlighted; }			// used directly from 'outside', i.e. procNode
			virtual	void	setIsSelected(bool selected)				{ m_isSelected = selected; }				// used directly by interacting with the room
			virtual	void	setIsShowingDetails(bool showingDetails)	{ m_isShowingDetails = showingDetails; }	// used directly from 'outside', i.e. procNode
			virtual	void	setIsConnected(bool connected)				{ m_isConnected = connected; }

			bool			getIsEmphasized()	{ return m_isHovered  || m_isHighlighted; }
			bool			getIsUnfolded()		{ return m_isSelected || m_isShowingDetails; }

			std::string getName() const { return m_name; };

			std::string getCaption() const { return m_caption; };
			void setCaption(std::string caption) { m_caption = caption; };

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json, act::UID msgUID = "");

			virtual ci::Json toParams() { return ci::Json::object(); };
			virtual void fromParams(ci::Json json) {};

			//void connectPositionPort(std::shared_ptr<RoomNodeBase> node);
			//void disconnectPositionOutPort(act::proc::InputPortRef<vec3> inputPort);
			//act::proc::OutputPortRef<vec3> getOutputPort() { return m_positionOutPort; };
			//act::proc::InputPortRef<vec3>  getInputPort()  { return m_positionInPort; };
			act::UID getCopyPosUID() { return m_copyPositionUID; };
			void setCopyPosUid(act::UID uid) { m_copyPositionUID = uid; };
			//void sendCurrentPosition();

			static void setPublisher(act::net::NetworkPublisherRef publisher) { m_publisher = publisher; };
			ScopedReplyUID setReplyUID(act::UID replyUID, bool doNotScope = false);

		protected:
			std::string				m_caption;

			int						m_markerID = -1;					//-1 equals no assigned marker

			ci::vec3				m_position;
			ci::vec3				m_rotation;							// in radians !
			ci::quat				m_orientation;			
			ci::vec3				m_lookAt = vec3(0.0f);
			bool					m_isLookingAt = false;
			float					m_radius = 1.0f;
			bool					m_isFixed = false;

			ci::vec3				m_upDir = vec3(0.0f, 1.0f, 0.0f);
			ci::vec3				m_frontDir = vec3(0.0f, 0.0f, 1.0f);
			ci::vec3				m_rightDir = vec3(1.0f, 0.0f, 0.0f); // m_upDir x m_frontDir

			virtual void			onPosition(act::UID replyUID = "", bool publish = true);
			virtual void			onOrientation(act::UID replyUID = "", bool publish = true);
			ci::mat4				m_transform;
			virtual void			updateTransform();

			bool					m_isSmoothing = false;
			FilterBaseRef<glm::vec3>m_posFlt;
			FilterBaseRef<glm::vec3>m_rotFlt;
			FilterBaseRef<glm::quat>m_orientFlt;

			ci::gl::BatchRef		m_mesh;
			ci::AxisAlignedBox		m_bounds;
			
			void					publishChanges();
			void					publishChanges(std::string key, ci::Json data, net::PublishType type = act::net::PublishType::PT_ROOMNODE_UPDATE, act::UID replyUID = "");
			void					publishParam(std::string key, ci::Json value);

			void					enableStatusColor();
			//act::proc::OutputPortRef<vec3>	m_positionOutPort;
			//act::proc::InputPortRef<vec3>		m_positionInPort;
			act::UID				m_copyPositionUID = "null";

			bool					m_isConnected = true;

		private:	
			std::string				m_name;
			act::UID				m_replyUID = "";
			static act::net::NetworkPublisherRef m_publisher;

			ci::TriMeshRef			m_triMesh;

			bool					m_isHovered = false; // used directly by interacting with the roomNode
			bool					m_isHighlighted = false; // used directly from 'outside', i.e. procNode
			bool					m_isSelected = false; // used directly by interacting with the roomNode
			bool					m_isShowingDetails = false; // used directly from 'outside', i.e. procNode

		}; using RoomNodeBaseRef = std::shared_ptr<RoomNodeBase>;


		class RoomNodeBaseRegistry
		{
		public:
			using nodeCreateFunc = std::shared_ptr<RoomNodeBase>(*)();

		public:
			RoomNodeBaseRegistry() = delete;

			static bool add(const std::string name, nodeCreateFunc createFunc);

			static std::shared_ptr<RoomNodeBase> create(const std::string& name);

			static std::map<std::string, nodeCreateFunc>& getMap()
			{
				static std::map<std::string, nodeCreateFunc> nodes;
				return nodes;
			}
		};
		
	}
}