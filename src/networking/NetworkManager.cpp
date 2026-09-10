
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "NetworkManager.hpp"
#include "cinder\gl\gl.h"
#include "cinder\app\App.h"
#include "cinder/CinderImGui.h"

#include "RoomManagers.hpp"
#include "Middleware.hpp"
#include "WebUIServer.hpp"
#include "WebUISecureServer.hpp"
#include "TCPSocket.hpp"

#include <algorithm>

act::net::NetworkManager::NetworkManager(act::room::RoomManagersRef roomMgrs)
{
	m_middleware = net::Middleware::create(roomMgrs);

	m_lastT = 0.0;
}

act::net::NetworkManager::~NetworkManager()
{
}

void act::net::NetworkManager::setup()
{
	m_webUI = net::WebUIServer::create(std::dynamic_pointer_cast<MsgReciever>(shared_from_this()));
	m_connections[m_webUI->getUID()] = m_webUI;

	m_secureWebUI = net::WebUISecureServer::create(std::dynamic_pointer_cast<MsgReciever>(shared_from_this()));
	m_connections[m_secureWebUI->getUID()] = m_secureWebUI;

	auto tcpSocket = TCPSocket::create(std::dynamic_pointer_cast<MsgReciever>(shared_from_this()));
	m_connections[tcpSocket->getUID()] = tcpSocket;
}

void act::net::NetworkManager::drawStatusBar()
{
	m_secureWebUI->draw();
}

void act::net::NetworkManager::publishChanges(act::UID msgUID, ci::Json data, PublishType publishtype)
{
	MsgType		type	= MsgType::MT_ROOMNODE;
	MsgMethod	method	= MsgMethod::MM_UNKNOWN;
	ci::Json	json;

	switch (publishtype) {
	case PT_ROOMNODE_CREATE:
		method = MsgMethod::MM_CREATE;
		break;
	case PT_ROOMNODE_UPDATE:
		method = MsgMethod::MM_UPDATE;
		break;
	case PT_ROOMNODE_DELETE:
		method = MsgMethod::MM_DELETE;
		break;
	default:
		method = MsgMethod::MM_ERROR;
	}

	Message	msg = Message(msgUID, type, method);

	if (method == MsgMethod::MM_ERROR) {
		json = msg.createErrorMsgJson("publishChanges", "Unknown publish type.");
	}
	else {
		msg.setData(data);
		json = msg.toJson();
	}

	for (auto& [key, connection] : m_connections) {
		connection->sendMsg(json);
	}
}

void act::net::NetworkManager::update()
{
	m_webUI->update();
	m_secureWebUI->update();
}

void act::net::NetworkManager::getFullDescription() {
	m_lastT = ci::app::getElapsedSeconds();

	if (m_webUI->isConnected())
 		m_webUI->sendMsg(m_middleware->getFullDescription());

	if (m_secureWebUI->isConnected())
		m_secureWebUI->sendMsg(m_middleware->getFullDescription());
}

void act::net::NetworkManager::drawGUI()
{
	for (auto& [key, connection] : m_connections) {
		std::string status = connection->getCurrentStatus();
		ImGui::TextUnformatted(status.c_str());
	}
}

ci::Json act::net::NetworkManager::toJson()
{
	auto json = ci::Json::object();

	ci::Json devices = ci::Json::array();
	
	json["devices"] = devices;

	return json;
	
}

void act::net::NetworkManager::fromJson(ci::Json json)
{
	if (json.contains("devices")) {
		auto devicesJson = json["devices"];
		for (auto&& device : devicesJson) {

			
		}
	}
}

void act::net::NetworkManager::onMsg(ci::Json json, act::UID uid)
{
	m_middleware->recieveJson(json, getConnectionByUID(uid));
}

act::net::ConnectionProviderRef act::net::NetworkManager::getConnectionByUID(act::UID uid)
{
	if(m_connections.find(uid) != m_connections.end())
		return m_connections[uid];

	return nullptr;
}

void act::net::NetworkManager::onConnect(act::UID uid)
{
	getConnectionByUID(uid)->sendMsg(m_middleware->getFullDescription());
}

void act::net::NetworkManager::onDisconnect(act::UID uid)
{
}
