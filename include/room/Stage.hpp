/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeBase.hpp"
#include "ModuleBase.hpp"
#include "RoomManagers.hpp"


namespace act {
	namespace room {

		class Stage : public RoomNodeBase {
		public:
			Stage();
			~Stage();

			static	std::shared_ptr<Stage> create() { return std::make_shared<Stage>(); };

			void	setup() override {};
			void	setup(act::room::RoomManagersRef roomMgrs);
			void	update() override;
			void	draw() override;

			void				addNode(RoomNodeBaseRef node);
			RoomNodeBaseRef	getNodeByUID(act::UID uid);

			bool				hit(glm::vec3 pos) override;
			bool				hitRay(ci::Ray ray) override;
			RoomNodeBaseRef	getNodeAtPos(glm::vec3 pos);
			RoomNodeBaseRef	getNodeOnRay(ci::Ray ray);

			bool	removeNode(act::UID uid);
			void	clear();

			const std::vector<RoomNodeBaseRef> getNodes() { return m_nodes; };
			std::vector<RoomNodeBaseRef> getAllNodes();
			const RoomNodeBaseRef getSelectedNode() { return m_selectedNode; };
			void setSelectedNode(RoomNodeBaseRef node);

			ci::Json toJson() override;
			void fromJson(ci::Json json, act::UID replyUID = "") override;

		private:

			std::vector<RoomNodeBaseRef>	m_nodes;
			act::room::RoomManagersRef		m_roomMgrs;

			act::room::RoomNodeBaseRef		m_selectedNode;

			ci::gl::BatchRef				m_wireRoom;
			ci::gl::BatchRef				m_wirePlane;
			glm::vec3						m_size;
		};
		using StageRef = std::shared_ptr<Stage>;
	}
}