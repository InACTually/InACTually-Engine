
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "server_ws.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {
		using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
		using WsServerRef = std::shared_ptr<WsServer>;

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

			WsServerRef				m_server;
			std::thread				m_serverThread;
			unsigned int			m_port;
			bool					m_isConnected;

			std::string				m_text;

			void					sendJson(ci::Json json);
			void					recieveJson(ci::Json json);

		}; using NetworkProcNodeRef = std::shared_ptr<NetworkProcNode>;

	}
}