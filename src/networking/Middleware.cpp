
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
	Fabian Töpfer
*/

#include "Middleware.hpp"
#include "cinder/Log.h"

#include "Design.hpp"
#include "ModuleRegistry.hpp"
#include "JsonMsgProcNode.hpp"
#include "ProcNodeBase.hpp"
#include <MatToBase64.hpp>
using namespace act::proc;
//#include "MatToBase64.hpp"


act::net::Middleware::Middleware(act::room::RoomManagersRef roomMgrs)
	: m_roomMgrs(roomMgrs)
{
	for (auto&& module : reg_modules) {
		mod::RoomModuleRef mod = std::dynamic_pointer_cast<mod::RoomModule> (module);
		if (mod) {
			m_roomMod = mod;
		}
		else {
			mod::ProcessingModuleRef mod = std::dynamic_pointer_cast<mod::ProcessingModule> (module);
			if (mod) {
				m_procMod = mod;
			}
		}
	}

	m_text = "Initializing";
	createProcNodeTypeData();

	m_text = "Listening";
}

act::net::Middleware::~Middleware() {
}

void act::net::Middleware::recieveJson(ci::Json json, ConnectionProviderRef sender) {
	auto msg = Message::create(json, sender);

	if (!msg->isValid()) {
		return;
	}

	ci::Json data = msg->getData();

	switch (msg->getType()) {

	case MT_PROCNODE:

		switch (msg->getMethod()) {
		case MM_SUBSCRIBE:
			subscribeToProcNode(data["uid"], data["valueName"], sender);
			break;
		case MM_UNSUBSCRIBE:
			unsubscribeFromProcNode(data["uid"], data["valueName"]);
			break;
		case MM_CREATE:
			sender->sendMsg(createProcNode(msg->getUID(), data["nodeName"]));
			break;
		case MM_DELETE:
			sender->sendMsg(deleteProcNode(data["uid"]));
			break;
		case MM_CONNECT:
			connectProcNodes(data["fromUID"], data["outputName"], data["toUID"], data["inputName"]);
			break;
		case MM_DISCONNECT:
			sender->sendMsg(disconnectProcNodes(data["fromUID"], data["outputName"], data["toUID"], data["inputName"]));
			break;
		case MM_REQUEST:
			sender->sendMsg(getParameterOfProcNode(msg->getUID(), data["uid"]));
			break;
		case MM_UPDATE:
			setParameterOfProcNode(data["uid"], data["params"]);
			break;
		case MM_REMOTEPROCEDURECALL:
			sender->sendMsg(callRPC(msg->getUID(), data["uid"], data["functionName"]));
			break;
		default:
			m_text = "unknown method in json";
			CI_LOG_W("Got unknown method '" + (std::string)json["method"] + "' for " + (std::string)json["type"] + " from JSON Message");
		}
		break;


	case MT_ROOMNODE:

		switch(msg->getMethod()) {
		case MM_CREATE:
			createRoomNode(msg->getUID(), data);
			//sender->sendMsg(getRoomDescription());
			break;
		case MM_UPDATE:
			updateRoomNode(msg->getUID(), data);
			break;
		case MM_DELETE:
			deleteRoomNode(msg->getUID(), data["uid"]);
			//sender->sendMsg(getRoomDescription());
			break;
		case MM_REMOTEPROCEDURECALL:
			sender->sendMsg(callRPC(msg->getUID(), data["uid"], data["functionName"]));
			break;
		default:
			m_text = "unknown method in json";
			CI_LOG_W("Got unknown method '" + (std::string)json["method"] + "' for " + (std::string)json["type"] + " from JSON Message");
		}
		break;

	case MT_ASSET:
		switch (msg->getMethod()) {
		case MM_UPLOAD:
			sender->sendMsg(uploadAudio(msg->getUID(), data["uid"], data["fileName"], data["fileData"]));
			break;
		default:
			m_text = "unknown method in json";
			CI_LOG_W("Got unknown method '" + (std::string)json["method"] + "' for " + (std::string)json["type"] + " from JSON Message");
		}
		break;


	case MT_APP:

		switch (msg->getMethod()) {
		case MM_REMOTEPROCEDURECALL:
			sender->sendMsg(callRPC(msg->getUID(), data["uid"], data["functionName"]));
			break;
		default:
			m_text = "unknown method in json";
			CI_LOG_W("Got unknown method '" + (std::string)json["method"] + "' for " + (std::string)json["type"] + " from JSON Message");
		}
		break;


	case MT_DESCRIPTION:

		switch (msg->getMethod()) {
		case MM_REQUEST:
			{
				std::string name = data["name"];
				if (checkEmpty(name, "requestDescription", "Name", sender))
					break;

				if (name == "full")
					sender->sendMsg(getFullDescription(msg->getUID()));
				if (name == "room")
					sender->sendMsg(getRoomDescription(msg->getUID()));
				if (name == "proc")
					sender->sendMsg(getProcDescription(msg->getUID()));
				if (name == "procNodeTypes")
					sender->sendMsg(requestProcNodeTypes(msg->getUID()));
			}
			break;
	
		default:
			m_text = "unknown method in json";
			CI_LOG_W("Got unknown method '" + (std::string)json["method"] + "' for " + (std::string)json["type"] + " from JSON Message");
		}
		break;


	default:
		m_text = "unknown type in json";
		CI_LOG_W("Got unknown " + (std::string)json["type"] + " from JSON Message");
	}
}

void act::net::Middleware::introduceSender(ConnectionProviderRef sender)
{
	sender->sendMsg(getFullDescription(""));
}

bool act::net::Middleware::checkEmpty(std::string var, std::string where, std::string what, ConnectionProviderRef sender)
{
	if (var == "") {
		if (sender) {
			sender->sendMsg(Message().createErrorMsgJson(where, what + " was empty."));
		}
		CI_LOG_E("On '" << where << "' " << what);
		return true;
	}
	return false;
}

void act::net::Middleware::subscribeToProcNode(act::UID uid, std::string valueName, ConnectionProviderRef sender) {
	if (checkEmpty(uid,			"subscribeToProcNode", "UID", sender) ||
		checkEmpty(valueName,	"subscribeToProcNode", "Name", sender))
		return;

	auto node = m_procMod->getNodeByUID(uid);
	if (node) {
		auto outputPort = node->getOutputPortByName(valueName);
		if (outputPort) {
			auto jsonNode = proc::JsonMsgProcNode::cast(proc::JsonMsgProcNode::create());
			auto jsonInputPort = jsonNode->getInputPortByType(outputPort->getType());
			auto jsonOutputPort = jsonNode->getOutputPortByType(proc::PT_JSON);

			m_jsonNodes.push_back(jsonNode);

			act::proc::InputPortRef<ci::Json> inputPort = act::proc::InputPort<ci::Json>::create(act::proc::PT_JSON, uid, [&, valueName, sender](ci::Json inputdata, act::UID portUID) {
				
				MessageRef msg = Message::create();
				//ci::Json msg = ci::Json::object();

				msg->setType(MT_PROCNODE);
				msg->setMethod(MM_SUBSCRIBE);
				
				ci::Json data = ci::Json::object();
				data["valueName"]	= valueName;
				data["uid"]			= portUID;
				//for merging two json objects
				data.update(inputdata);

				msg->setData(data);

				sender->sendMsg(msg->toJson());
			});

			m_subscriptions[uid][valueName] = inputPort;
			outputPort->connect(jsonInputPort);
			jsonOutputPort->connect(inputPort);
		}
	}

}

void act::net::Middleware::unsubscribeFromProcNode(act::UID uid, std::string valueName) {
	if (checkEmpty(uid,			"unsubscribeFromProcNode", "UID") ||
		checkEmpty(valueName,	"unsubscribeFromProcNode", "Name"))
		return;

	auto node = m_procMod->getNodeByUID(uid);
	if (node) {
		auto outputPort = node->getOutputPortByName(valueName);
		if (outputPort) {
			outputPort->disconnect(m_subscriptions[uid][valueName]);
			m_subscriptions[uid].erase(valueName);
			if (m_subscriptions[uid].empty())
				m_subscriptions.erase(uid);
		}
	}

}

ci::Json act::net::Middleware::createProcNode(act::UID msgUID, std::string nodeName) {
	if (checkEmpty(msgUID, "createProcNode", "msgUID") ||
		checkEmpty(nodeName, "createProcNode", "Name"))
		return Message().createErrorMsgJson("createProcNode", "msgUID or Name");

	auto node = m_procMod->createNodeByName(nodeName);
	UID uid = "";
	if (node) {
		uid = node->getUID();
	}

	Message msg(msgUID, MsgType::MT_PROCNODE, MsgMethod::MM_CREATE); 

	auto data = ci::Json::object();
	data["uid"] = uid;
	msg.setData(data);

	return msg.toJson();
}

ci::Json act::net::Middleware::deleteProcNode(act::UID uid) {
	if (checkEmpty(uid,			"deleteProcNode", "UID"))
		return Message().createErrorMsgJson("deleteProcNode", "UID");

	m_procMod->deleteNodeByUID(uid); // TODO

	Message msg("", MsgType::MT_PROCNODE, MsgMethod::MM_DELETE); // TODO  has no msgUID to answer

	auto data = ci::Json::object();
	data["uid"] = uid;
	msg.setData(data);

	return msg.toJson();
}

void act::net::Middleware::connectProcNodes(act::UID fromUID, std::string outputName, act::UID toUID, std::string inputName) {
	if (checkEmpty(fromUID, "connectProcNodes", "fromUID") ||
		checkEmpty(outputName, "connectProcNodes", "outputName") ||
		checkEmpty(toUID, "connectProcNodes", "toUID") ||
		checkEmpty(inputName, "connectProcNodes", "inputName"))
		return;

	auto outputPort = m_procMod->getNodeByUID(fromUID)->getOutputPortByName(outputName);
	auto inputPort	= m_procMod->getNodeByUID(toUID)->getInputPortByName(inputName);
	m_procMod->connect(outputPort, inputPort);
}

ci::Json act::net::Middleware::disconnectProcNodes(act::UID fromUID, std::string outputName, act::UID toUID, std::string inputName) {
	if (checkEmpty(fromUID,		"disconnectProcNodes", "fromUID") ||
		checkEmpty(outputName,	"disconnectProcNodes", "outputName") ||
		checkEmpty(toUID,		"disconnectProcNodes", "toUID") ||
		checkEmpty(inputName,	"disconnectProcNodes", "inputName"))
		return Message().createErrorMsgJson("disconnectProcNodes", "fromUID, outputName, toUID or inputName");

	return Message().createErrorMsgJson("disconnectProcNodes", "Is not implemented, yet :).");
}

ci::Json act::net::Middleware::getParameterOfProcNode(act::UID msgUID, act::UID uid) {
	if (checkEmpty(msgUID,		"getParameterOfProcNode", "msgUID") ||
		checkEmpty(uid,			"getParameterOfProcNode", "UID"))
		return Message().createErrorMsgJson("getParameterOfProcNode", "msgUID or act::UID");

	Message msg(msgUID, MsgType::MT_PROCNODE, MsgMethod::MM_UPDATE);

	auto data = ci::Json::object();
	data["uid"] = uid;
	data["params"] = m_procMod->getNodeByUID(uid)->toParams();
	msg.setData(data);

	return msg.toJson();
}

void act::net::Middleware::setParameterOfProcNode(act::UID uid, ci::Json params) {
	if (checkEmpty(uid,			"setParameterOfProcNode", "UID"))
		return;

	m_procMod->getNodeByUID(uid)->fromParams(params);
}

ci::Json act::net::Middleware::requestProcNodeTypes(act::UID msgUID) {
	Message msg(msgUID, MsgType::MT_DESCRIPTION, MsgMethod::MM_UPDATE);

	auto data = ci::Json::object();
	data["name"] = "procNodeTypes";
	data["procNodeTypes"] = m_procNodeTypeData;
	msg.setData(data);

	return msg.toJson();
}

void act::net::Middleware::createProcNodeTypeData()
{
	if (m_nodeTypeNodes.empty()) {
		std::map<act::UID, act::proc::ProcNodeRegistry::nodeCreateFunc> registeredNodes = std::make_shared<act::proc::ProcNodeRegistry>()->getMap();
		for (auto&& regNode : registeredNodes) {
			try {
				m_nodeTypeNodes.push_back(regNode.second());
			}
			catch (...) {
				int panic = 1;
				continue;
			}
		}
	}
	m_procNodeTypeData = ci::Json::object();
	for (auto&& node : m_nodeTypeNodes) {
		m_procNodeTypeData.update(node->getJsonTypeDefinition());
	}
}

// TODO refactor to generic upload/transmit data/-blob function
ci::Json act::net::Middleware::uploadAudio(act::UID msgUID, act::UID uid, std::string fileName, std::string fileData) {
	const std::vector<BYTE> byteData = _BASE64_H_::base64_decode(fileData);

	if (!std::filesystem::is_directory(ci::app::getAssetPath("") / "uploaded-audio")) {
		std::filesystem::create_directory(ci::app::getAssetPath("") / "uploaded-audio");
	}

	const std::filesystem::path filePath = ci::app::getAssetPath("") / "uploaded-audio" / fileName;
	std::ofstream audioFile(filePath, std::ios::binary | std::ios::out);
	if (!audioFile) {
		CI_LOG_D(filePath.string() + " is not valid!");
		return Message().createErrorMsgJson("uploadAudio", "FilePath is not valid.");
	}
	audioFile.write(reinterpret_cast<const char*>(byteData.data()), byteData.size());
	audioFile.close();

	Message msg(msgUID, MsgType::MT_ASSET, MsgMethod::MM_UPLOAD);

	auto data = ci::Json::object();
	data["data"] = filePath.string();
	msg.setData(data);

	return msg.toJson();
}


ci::Json act::net::Middleware::createRoomNode(act::UID msgUID, ci::Json data)
{
	if (!data.contains("name"))
		return Message().createErrorMsgJson("createRoomNode", "Name");

	if (!m_roomMod->createRoomNode(data)) {
		Message msg;
		msg.setType(MsgType::MT_ROOMNODE);
		return msg.createErrorMsgJson("createRoomNode", "Could not create " + (std::string)data["name"]);
	}
	return nullptr;
}

ci::Json act::net::Middleware::updateRoomNode(act::UID msgUID, ci::Json data)
{
	if (!m_roomMod->updateRoomNode(data, msgUID)) {
		Message msg;
		msg.setType(MsgType::MT_ROOMNODE);
		return msg.createErrorMsgJson("updateRoomNode", "Could not update " + (std::string)data["uid"]);
	};

	return nullptr;
}
ci::Json act::net::Middleware::deleteRoomNode(act::UID msgUID, act::UID uid)
{
	if(!m_roomMod->deleteRoomNode(uid, msgUID)) {
		Message msg;
		msg.setType(MsgType::MT_ROOMNODE);
		return msg.createErrorMsgJson("deleteRoomNode", "Could not delete " + uid);
	};
	
	return nullptr;
}

ci::Json act::net::Middleware::getRoomDescription(act::UID msgUID) {
	auto roomDesc = m_roomMod->getFullDescription();

	Message msg(msgUID, MsgType::MT_DESCRIPTION, MsgMethod::MM_UPDATE);

	auto data = ci::Json::object();
	data["name"] = "room";
	data["roomDescription"] = roomDesc;
	msg.setData(data);

	return msg.toJson();
}
ci::Json act::net::Middleware::getProcDescription(act::UID msgUID) {
	auto procDesc = m_procMod->getFullDescription();

	Message msg(msgUID, MsgType::MT_DESCRIPTION, MsgMethod::MM_UPDATE);

	auto data = ci::Json::object();
	data["name"] = "proc";
	data["procDescription"] = procDesc;
	msg.setData(data);

	return msg.toJson();
}
ci::Json act::net::Middleware::getFullDescription(act::UID msgUID) {
	auto roomDesc = m_roomMod->getFullDescription();
	auto procDesc = m_procMod->getFullDescription();

	Message msg(msgUID, MsgType::MT_DESCRIPTION, MsgMethod::MM_UPDATE);

	auto data = ci::Json::object();
	data["name"] = "full";
	data["roomDescription"] = roomDesc;
	data["procDescription"] = procDesc;
	msg.setData(data);

	return msg.toJson();
}

ci::Json act::net::Middleware::callRPC(act::UID msgUID, act::UID uid, std::string functionName)
{
	if (checkEmpty(uid, "callRPC", "uid") ||
		checkEmpty(functionName, "callRPC", "functionName"))
		return Message().createErrorMsgJson("callRPC", "uid or functionName");
	bool success = false;

	Message msg(msgUID, MsgType::MT_APP, MsgMethod::MM_REMOTEPROCEDURECALL);

	ci::Json data = ci::Json::object();
	if (m_procMod->hasNodeWithUID(uid)) {
		success = m_procMod->callRPC(uid, functionName);
	}
	else if (m_roomMod->hasNodeWithUID(uid)) {
		success = m_roomMod->callRPC(uid, functionName);
	}
	else {
		data["what"] = "This UID has not been found.";
	}
	data["success"]	= success;
	msg.setData(data);

	return msg.toJson();
}

void act::net::Middleware::update() {
}

void act::net::Middleware::draw() {

	std::stringstream strstr;
	strstr << "Middleware: " << m_text;
	ImGui::TextUnformatted(strstr.str().c_str());

}

ci::Json act::net::Middleware::toJson() {
	return ci::Json::object();
}

void act::net::Middleware::fromJson(ci::Json json) {
}