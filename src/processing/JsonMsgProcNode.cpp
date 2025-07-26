
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "JsonMsgProcNode.hpp"


#include "processing/MatToBase64.hpp"

#include "MetaModel.hpp"

act::proc::JsonMsgProcNode::JsonMsgProcNode() : ProcNodeBase("JsonMsg") {

	m_drawSize = ivec2(100, 200);

	m_msgName = "value";  // TODO: maybe remove this, for not keeping JsonMsgNode in WebUIServer (ports will survive in connection-chain)

	auto number = InputPort<float>::create(PT_NUMBER, "number", [&](float n) {
		auto json = ci::Json::object();
		json["params"]["name"]		= m_msgName;
		json["params"]["type"]		= "number";
		json["params"]["value"]	= n;
		m_jsonPort->send(json);
	});

	auto text = InputPort<std::string>::create(PT_TEXT, "text", [&](std::string t) {
		auto json = ci::Json::object();
		json["params"]["name"]		= m_msgName;
		json["params"]["type"]		= "text";
		json["params"]["value"]	= t;
		m_jsonPort->send(json);
	});

	auto boolean = InputPort<bool>::create(PT_BOOL, "bool", [&](bool b) {
		auto json = ci::Json::object();
		json["params"]["name"]		= m_msgName;
		json["params"]["type"]		= "bool";
		json["params"]["value"]	= b;
		m_jsonPort->send(json);
	});

	auto vec2D = InputPort<glm::vec2>::create(PT_VEC2, "vec2", [&](glm::vec2 v) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "vec2";
		json["params"]["x"]	= v.x;
		json["params"]["y"]	= v.y;
		m_jsonPort->send(json);
	});

	auto vec3D = InputPort<glm::vec3>::create(PT_VEC3, "vec3", [&](glm::vec3 v) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "vec3";
		json["params"]["x"] = v.x;
		json["params"]["y"] = v.y;
		json["params"]["z"] = v.z;
		m_jsonPort->send(json);
	});

	auto quaternion = InputPort<glm::quat>::create(PT_QUAT, "quat", [&](glm::quat q) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "quat";
		json["params"]["w"] = q.w;
		json["params"]["x"] = q.x;
		json["params"]["y"] = q.y;
		json["params"]["z"] = q.z;
		m_jsonPort->send(json);
	});

	auto jsonMsg = InputPort<ci::Json>::create(PT_JSON, "json", [&](ci::Json j) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "json";
		json["params"]["data"] = j;
		m_jsonPort->send(json);
	});

	auto color = InputPort<ci::Color>::create(PT_COLOR, "color", [&](ci::Color c) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "color";
		json["params"]["r"]	= c.r;
		json["params"]["g"]	= c.g;
		json["params"]["b"]	= c.b;
		m_jsonPort->send(json);
	});

	auto stringfloat = InputPort<feature>::create(PT_FEATURE, "labeled number", [&](std::pair<std::string, float> sf) {
		auto json = ci::Json::object();
		json["params"]["name"]		= m_msgName;
		json["params"]["type"]		= "labelednumber";
		json["params"]["label"]	= sf.first;
		json["params"]["value"]	= sf.second;
		m_jsonPort->send(json);
	});

	auto image = InputPort<cv::UMat>::create(PT_IMAGE, "image", [&](cv::UMat uMat) {
		std::string base64 = matToBase64(uMat.getMat(cv::ACCESS_FAST), ".jpg", 85, true, 1280);
		auto json = ci::Json::object();
		json["params"]["name"]		= m_msgName;
		json["params"]["type"]		= "image";
		json["params"]["base64"]	= base64;
		m_jsonPort->send(json);
	});

	auto bodies = InputPort<std::vector<room::BodyRef>>::create(PT_BODYLIST, "bodies", [&](std::vector<room::BodyRef> bodies) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "bodies";
		ci::Json bodiesJson = ci::Json::array();
		for (auto&& body : bodies) {
			bodiesJson.push_back(body->toJson());
		}
		json["params"]["bodies"] = bodiesJson;
		m_jsonPort->send(json);
	});

	auto body = InputPort<room::BodyRef>::create(PT_BODY, "body", [&](room::BodyRef body) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "body";
		json["params"]["body"] = body->toJson();
		m_jsonPort->send(json);
	});

	auto metamodal = InputPort<MetaModelRef>::create(PT_METAMODEL, "metamodal", [&](MetaModelRef metaModal) {
		auto json = ci::Json::object();
		json["params"]["name"] = m_msgName;
		json["params"]["type"] = "metamodel";
		json["params"]["model"] = metaModal->toJson();
		m_jsonPort->send(json);
	});

	auto pointCloudInput = InputPort<act::room::Pointcloud>::create(PT_POINTCLOUD, "pointcloud", [&](act::room::Pointcloud pointcloud) {
		if (pointcloud == nullptr) return;
		
		auto json = ci::Json::object();
		auto points = *pointcloud;// ->points;

		json["name"] = m_msgName;
		json["type"] = "pointcloud";
		json["points"] = ci::Json::array();

		for (int i = 0; i < points.size(); i += 1) {
			auto pt = ci::Json::object();
			pt["x"] = points[i].x;
			pt["y"] = points[i].y;
			pt["z"] = points[i].z;
			pt["r"] = points[i].r;
			pt["g"] = points[i].g;
			pt["b"] = points[i].b;
			json["points"].push_back(pt);
		}
		m_jsonPort->send(json);
	});

	auto audioBufferInput = InputPort<ci::audio::BufferRef>::create(PT_AUDIO, "audio buffer", [&](ci::audio::BufferRef buffer) {
		if (buffer == nullptr) return;

		auto	json = ci::Json::object();
		auto	data = std::vector<float>(buffer->getData(), buffer->getData() + buffer->getSize());
		int		numChannel = (int)buffer->getNumChannels();
		int		numFrames = (int)buffer->getNumFrames();
		int		size = buffer->getSize();

		json["name"]	= m_msgName;
		json["type"]	= "audiobuffer";
		json["channel"] = numChannel;
		json["data"]	= data;
		
		m_jsonPort->send(json);
	});

	m_inputPorts.push_back(number);
	m_inputPorts.push_back(boolean);
	m_inputPorts.push_back(text);
	m_inputPorts.push_back(vec2D);
	m_inputPorts.push_back(vec3D);
	m_inputPorts.push_back(quaternion);
	m_inputPorts.push_back(color);
	m_inputPorts.push_back(stringfloat);
	m_inputPorts.push_back(image);
	m_inputPorts.push_back(metamodal);
	m_inputPorts.push_back(bodies);
	m_inputPorts.push_back(body);
	m_inputPorts.push_back(pointCloudInput);
	m_inputPorts.push_back(audioBufferInput);
	m_inputPorts.push_back(jsonMsg);

	m_allInputPorts.assign(m_inputPorts.begin(), m_inputPorts.end());

	
	m_jsonPort = OutputPort<ci::Json>::create(PT_JSON, "json");
	m_outputPorts.push_back(m_jsonPort);

}

act::proc::JsonMsgProcNode::~JsonMsgProcNode() {
}

void act::proc::JsonMsgProcNode::update() {

}

void act::proc::JsonMsgProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x);
	ImGui::InputText("msg name", &m_msgName);
	
	std::stringstream strstr;
	
	endNodeDraw();
}


ci::Json act::proc::JsonMsgProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["msgname"] = m_msgName;
	return json;
}

void act::proc::JsonMsgProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "msgname", m_msgName);
}
