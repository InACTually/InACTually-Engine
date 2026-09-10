
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 20234

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include <memory>
#include <vector>
#include "cinder/Json.h"

#include "RoomManagers.hpp"
//#include "Middleware.hpp"
//#include "WebUIServer.hpp"
#include "Connection.hpp"
#include "NetworkPublisher.hpp"


namespace act {
	namespace net {

		class Middleware;
		class WebUIServer;
		class WebUISecureServer;

		class NetworkManager : public act::net::MsgReciever, public act::net::NetworkPublisher, public std::enable_shared_from_this<NetworkManager> {
		public:
			NetworkManager(act::room::RoomManagersRef roomMgrs);
			~NetworkManager();

			static	std::shared_ptr<NetworkManager> create(act::room::RoomManagersRef roomMgrs) { return std::make_shared<NetworkManager>(roomMgrs); };

			void	setup();
			void	update();
			void	drawGUI();
			void	drawStatusBar();

			void	publishChanges(act::UID uid, ci::Json params, PublishType type = PT_ROOMNODE_UPDATE);

			void	getFullDescription();

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			virtual void onMsg(ci::Json json, act::UID uid) override;
			virtual void onConnect(act::UID uid) override;
			virtual void onDisconnect(act::UID uid) override;

		private:

			std::shared_ptr<act::net::Middleware>		m_middleware;
			std::shared_ptr<act::net::WebUIServer>		m_webUI;
			std::shared_ptr<act::net::WebUISecureServer> m_secureWebUI;
			std::map<act::UID, act::net::ConnectionProviderRef>	m_connections;

			proc::InputPortRef<room::BodyRefList>		m_bodiesInPort;

			double m_lastT;
			
			act::net::ConnectionProviderRef getConnectionByUID(act::UID uid);

		};
		using NetworkManagerRef = std::shared_ptr<NetworkManager>;
	}
}