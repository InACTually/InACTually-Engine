
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"
#include "camera/CameraManager.hpp"

#include "Body.hpp"
#include "BodyRoomNode.hpp"


namespace act {
	namespace room {

		class BodyTrackingManager : public RoomNodeManagerBase {
		public:
			BodyTrackingManager(CameraManagerRef cameraMgr);
			~BodyTrackingManager();

			static	std::shared_ptr<BodyTrackingManager> create(CameraManagerRef cameraMgr) { return std::make_shared<BodyTrackingManager>(cameraMgr); };

			void	setup() override;
			void	update() override;
			void	draw() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			act::proc::OutputPortRef<std::vector<act::room::BodyRef>> getOutputPort() { return m_bodiesOutPort; };

		private:
			CameraManagerRef m_cameraMgr;

			std::vector<act::room::BodyRef> m_bodies;
			act::proc::OutputPortRef<std::vector<act::room::BodyRef>> m_bodiesOutPort;
			bool m_wereZeroBodiesBefore = true;

			BodyRoomNodeRef getBodyRoomNodeByBodyUID(UID uid);
			BodyRoomNodeRef addBody(BodyRef body);
		};
		using BodyTrackingManagerRef = std::shared_ptr<BodyTrackingManager>;
	}
}