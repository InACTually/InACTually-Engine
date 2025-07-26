
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "WebSocketServer.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class NetworkProcNode : public ProcNodeBase
		{
		public:
			NetworkProcNode();
			~NetworkProcNode();

			PROCNODECREATE(NetworkProcNode);

			void setup(act::room::RoomManagers roomMgrs) override;
			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			void onJson(ci::Json event);

		private:

			WebSocketServer				m_server;
			unsigned int				m_port;
			std::string					m_text;

			void						sendJson(ci::Json json);
			void						recieveJson(ci::Json json);

		}; using NetworkProcNodeRef = std::shared_ptr<NetworkProcNode>;

	}
}