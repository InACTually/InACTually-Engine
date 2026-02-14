
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

#include "procpch.hpp"
#include "NetworkProcNode.hpp"

act::proc::NetworkProcNode::NetworkProcNode() : ProcNodeBase("Network") {

	auto image = createJsonInput("json", [&](ci::Json json) { this->onJson(json); });

	//m_imagePort = createImageOutput("pass-through image");

	m_port = 9002;
	
	m_text = "Initializing";

	m_server = std::make_shared<WsServer>();
	m_server->config.port = m_port;
	m_server->config.header.insert({ "Access-Control-Allow-Origin", "*" });

	auto& endpoint = m_server->endpoint["^/?$"];

	endpoint.on_close = [&](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/) {
		if (m_server->get_connections().size() == 0) {
			m_isConnected = false;
		}

		m_text = "Connection closed";
		CI_LOG_I(m_text);
	};

	endpoint.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec) {
		std::stringstream strstr;
		strstr << "Error in connection " << connection.get() << ". " << "Error: " << ec << ", error message: " << ec.message();
		m_text = strstr.str();
		CI_LOG_I(m_text);
	};

	endpoint.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
		auto out_message = in_message->string();

		if (!out_message.empty()) {

			try {
				this->recieveJson(ci::Json(out_message));
				CI_LOG_I("JSON Message Received");
			}
			catch (const std::exception&)
			{
				m_text = out_message + " (string)";
				CI_LOG_I("Could not read JSON Message");
			}
		}
	};

	endpoint.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
		m_isConnected = true;
		m_text = "Connected";
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

act::proc::NetworkProcNode::~NetworkProcNode() {
}

void act::proc::NetworkProcNode::setup(act::room::RoomManagers roomMgrs) {
	
}

void act::proc::NetworkProcNode::sendJson(ci::Json json) {
	if (!m_isConnected | !json.is_null())
		return;

	auto str = json.dump();
	for (auto& a_connection : m_server->get_connections())
		a_connection->send(str);
}

void act::proc::NetworkProcNode::recieveJson(ci::Json json) {

}

void act::proc::NetworkProcNode::update() {
}

void act::proc::NetworkProcNode::draw() {
	beginNodeDraw();

	std::stringstream strstr;
	strstr << m_text << " on " << m_port;
	ImGui::TextUnformatted(strstr.str().c_str());
	
	endNodeDraw();
}

void act::proc::NetworkProcNode::onJson(ci::Json json) {
	sendJson(json);
}

ci::Json act::proc::NetworkProcNode::toParams() {
	return ci::Json::object();
}

void act::proc::NetworkProcNode::fromParams(ci::Json json) {
}
