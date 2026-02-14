
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

#include "WebUIServer.hpp"
#include "cinder/Log.h"

#include "Design.hpp"
#include "ModuleRegistry.hpp"
#include "Port.hpp"
#include "JsonMsgProcNode.hpp"
#include "ProcNodeBase.hpp"
#include <MatToBase64.hpp>
using namespace act::proc;
//#include "MatToBase64.hpp"


act::net::WebUIServer::WebUIServer(MsgRecieverRef reciever)
 : m_reciever(reciever)
{
	m_port = 9002;
	m_isConnected = false;
	m_text = "Initializing";

	m_server = std::make_shared<WsServer>();
	m_server->config.port = m_port;
	m_server->config.header.insert({ "Access-Control-Allow-Origin", "*" });

	auto& endpoint = m_server->endpoint["^/?$"];

	endpoint.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
		auto out_message = in_message->string();

		if (!out_message.empty()) {

			try {
				this->recieveJson(ci::Json::parse(out_message));
				// CI_LOG_I("JSON Message Received");
			}
			catch (const std::exception& exc)
			{
				m_text = "[cannot interprete msg] " + out_message + " (string) - " + exc.what();
				CI_LOG_I(m_text);
			}
		}

		// reply?

		};

	endpoint.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
		m_isConnected = true;
		m_reciever->onConnect(getUID());
		m_text = "A WebUI is connected";
		CI_LOG_I(m_text);
		};

	// See RFC 6455 7.4.1. for status codes
	endpoint.on_close = [&](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/) {
		if (m_server->get_connections().size() == 0) {
			m_reciever->onDisconnect(getUID());
			m_isConnected = false;			
		}
		m_text = "Connection closed";
		CI_LOG_I(m_text);
		};

	endpoint.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap& response_header) {
		//response_header.insert({ "Access-Control-Allow-Origin", "*" });
		return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
		};

	endpoint.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec) {
		std::stringstream strstr;
		strstr << "Error in connection " << connection.get() << ". " << "Error: " << ec << ", error message: " << ec.message();
		m_text = strstr.str();
		CI_LOG_I(m_text);
		};

	std::shared_ptr<std::promise<unsigned short>> server_port = std::make_shared<std::promise<unsigned short>>();
	auto server_port_future = server_port->get_future();
	m_serverThread = std::thread([server_port, this]() {
		try {
			m_server->start([server_port](unsigned short port) {
				try {
					server_port->set_value(port);
				}
				catch (...) {
				}
				});
		}
		catch (...) {
			try {
				server_port->set_exception(std::current_exception());
			}
			catch (...) {
			}
		}
		});
	// wait briefly for server to report bound port, avoid blocking indefinitely
	if (server_port_future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
		try {
			m_port = server_port_future.get();
		}
		catch (const std::exception& e) {
			CI_LOG_I(std::string("Failed to get server port: ") + e.what());
		}
	}
	else {
		CI_LOG_I("Server start timed out; continuing with configured port");
	}

	m_text = "Listening";
}

act::net::WebUIServer::~WebUIServer() {
	m_serverThread.join();
}

void act::net::WebUIServer::sendMsg(ci::Json msg) {
	if (!m_isConnected | !msg.is_null())
		return;

	auto str = msg.dump();
	for (auto& a_connection : m_server->get_connections())
		a_connection->send(str);
}

void act::net::WebUIServer::recieveJson(ci::Json json) {
	m_reciever->onMsg(json, getUID());
}


void act::net::WebUIServer::update() {
}

void act::net::WebUIServer::draw() {

	std::stringstream strstr;
	strstr << "WebUI: " << m_text;
	ImGui::TextUnformatted(strstr.str().c_str());

}

ci::Json act::net::WebUIServer::toJson() {
	return ci::Json::object();
}

void act::net::WebUIServer::fromJson(ci::Json json) {
}
