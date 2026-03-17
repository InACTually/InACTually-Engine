
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "WebUISecureServer.hpp"
#include "cinder/Log.h"

#include "Design.hpp"
#include "ModuleRegistry.hpp"
#include "Port.hpp"
#include "JsonMsgProcNode.hpp"
#include "ProcNodeBase.hpp"
#include <MatToBase64.hpp>
using namespace act::proc;
//#include "MatToBase64.hpp"


act::net::WebUISecureServer::WebUISecureServer(MsgRecieverRef reciever)
 : m_reciever(reciever)
{
	m_port = 9001;
	m_isConnected = false;
	m_text = "Initializing";

	m_server = std::make_shared<WssServer>(ci::app::getAssetPath("certs/inactually.crt").string(), ci::app::getAssetPath("certs/inactuallyCA.key").string());
	m_server->config.port = m_port;
	m_server->config.header.insert({ "Access-Control-Allow-Origin", "*" });

	auto& endpoint = m_server->endpoint["^/?$"];

	endpoint.on_message = [&](std::shared_ptr<WssServer::Connection> connection, std::shared_ptr<WssServer::InMessage> in_message) {
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

	endpoint.on_open = [&](std::shared_ptr<WssServer::Connection> connection) {
		m_isConnected = true;
		m_reciever->onConnect(getUID());
		m_text = "A WebUI is connected";
		CI_LOG_I(m_text);
	};

	// See RFC 6455 7.4.1. for status codes
	endpoint.on_close = [&](std::shared_ptr<WssServer::Connection> connection, int status, const std::string& /*reason*/) {
		std::stringstream strstr;
		strstr << "Closed connection " << connection->remote_endpoint().address().to_string() << ":" << connection->remote_endpoint().port() << ". ";
		CI_LOG_I(strstr.str());
		m_text = "Connection closed";
		if (m_server->get_connections().size() == 0) {
			m_reciever->onDisconnect(getUID());
			m_isConnected = false;
			strstr.clear();
			strstr.str("");
			strstr << "Listening on " << m_port;
			m_text = strstr.str();
		}
	};

	endpoint.on_handshake = [&](std::shared_ptr<WssServer::Connection> connection, SimpleWeb::CaseInsensitiveMultimap& response_header) {
		//response_header.insert({ "Access-Control-Allow-Origin", "*" });
		std::stringstream strstr;
		strstr << "Shaking hands with " << connection->remote_endpoint().address().to_string() << ":" << connection->remote_endpoint().port() << ". ";
		CI_LOG_I(strstr.str());
		return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
	};

	endpoint.on_error = [&](std::shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code& ec) {
		std::stringstream strstr;
		strstr << "Error in connection " << connection->remote_endpoint().address().to_string() << ":" << connection->remote_endpoint().port() << ". " << "Error: " << ec << ", error message: " << ec.message();
		m_text = strstr.str();
		CI_LOG_E(m_text);
	};

	auto server_port = std::make_shared<std::promise<unsigned short>>();
	auto server_port_future = server_port->get_future();
	m_serverThread = std::thread([server_port, this]() {
		try {
			m_server->start([server_port](unsigned short port) {
				try { server_port->set_value(port); }
				catch (...) {}
				});
		}
		catch (const std::exception& e) {
			CI_LOG_E(std::string("WssServer start failed: ") + e.what());
			try { server_port->set_exception(std::current_exception()); }
			catch (...) {}
		}
	});
	// wait briefly for server to report bound port
	if (server_port_future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
		try { 
			m_port = server_port_future.get(); 
			std::stringstream strstr;
			strstr << "Listening on " << m_port;
			m_text = strstr.str();
			CI_LOG_I("WebUI " + strstr.str());
		}
		catch (const std::exception& e) { CI_LOG_E(std::string("Failed to get server port: ") + e.what()); }
	}
	else {
		CI_LOG_E("WebUISecureServer start timed out.");
	}
}

act::net::WebUISecureServer::~WebUISecureServer() {
	m_server->stop();
	m_serverThread.join();
}

void act::net::WebUISecureServer::sendMsg(ci::Json msg) {
	if (!m_isConnected || msg.is_null())
		return;

	auto str = msg.dump();
	for (auto& a_connection : m_server->get_connections())
		a_connection->send(str);
}

void act::net::WebUISecureServer::recieveJson(ci::Json json) {
	m_reciever->onMsg(json, getUID());
}


void act::net::WebUISecureServer::update() {

}

void act::net::WebUISecureServer::draw() {

	std::stringstream strstr;
	strstr << "wss-WebUI: " << m_text;
	ImGui::TextUnformatted(strstr.str().c_str());

}

ci::Json act::net::WebUISecureServer::toJson() {
	return ci::Json::object();
}

void act::net::WebUISecureServer::fromJson(ci::Json json) {
}
