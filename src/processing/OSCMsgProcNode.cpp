
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "OSCMsgProcNode.hpp"


#include "processing/MatToBase64.hpp"

#include "MetaModel.hpp"

act::proc::OSCMsgProcNode::OSCMsgProcNode() : ProcNodeBase("OSCMsg") {

	m_msgName = "/value";  // TODO: maybe remove this, for not keeping OSCMsgNode in WebUIServer (ports will survive in connection-chain)
	
	auto number = InputPort<float>::create(PT_NUMBER, "number", [&](float n) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(n);
		m_oscPort->send(osc);
	});

	auto text = InputPort<std::string>::create(PT_TEXT, "text", [&](std::string t) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(t);
		m_oscPort->send(osc);
	});

	auto boolean = InputPort<bool>::create(PT_BOOL, "bool", [&](bool b) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(b);
		m_oscPort->send(osc);
	});

	auto vec2D = InputPort<glm::vec2>::create(PT_VEC2, "vec2", [&](glm::vec2 v) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(v.x);
		osc.append(v.y);
		m_oscPort->send(osc);
	});

	auto vec3D = InputPort<glm::vec3>::create(PT_VEC3, "vec3", [&](glm::vec3 v) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(v.x);
		osc.append(v.y);
		osc.append(v.z);
		m_oscPort->send(osc);
	});

	auto quaternion = InputPort<glm::quat>::create(PT_QUAT, "quat", [&](glm::quat q) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(q.w);
		osc.append(q.x);
		osc.append(q.y);
		osc.append(q.z);
		m_oscPort->send(osc);
	});

	auto json = InputPort<ci::Json>::create(PT_JSON, "json", [&](ci::Json j) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(j.dump());
		m_oscPort->send(osc);
	});

	auto color = InputPort<ci::Color>::create(PT_COLOR, "color", [&](ci::Color c) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(c.r);
		osc.append(c.g);
		osc.append(c.b);
		m_oscPort->send(osc);
	});

	auto stringfloat = InputPort<feature>::create(PT_FEATURE, "labeled number", [&](std::pair<std::string, float> sf) {
		auto osc = ci::osc::Message(m_msgName);
		osc.append(sf.first);
		osc.append(sf.second);
		m_oscPort->send(osc);
	});

	auto image = InputPort<cv::UMat>::create(PT_IMAGE, "image", [&](cv::UMat uMat) {
		std::string base64 = matToBase64(uMat.getMat(cv::ACCESS_FAST), ".jpg", 89, false, 1280);

		int chunkSize = 2048;
		std::vector<std::string> chunks;
		for (size_t i = 0; i < base64.size(); i += chunkSize) {
			chunks.push_back(base64.substr(i, chunkSize));
		}

		UID uid = UniqueIDBase().getUID();

		for (int i = 0; i < chunks.size(); i++) {
			auto osc = ci::osc::Message(m_msgName);
			osc.append(uid);
			osc.append((int)chunks.size());
			osc.append(i);
			osc.append(chunks[i]);
			m_oscPort->send(osc);
		}
		/*
		ci::Json json = ci::Json::object();
		json["uid"] = uid;
		json["data"] = base64;

		auto path = app::getAssetPath("").string() + "image.json";
		ci::writeJson(path, json);
	*/
	});

	auto bodies = InputPort<room::BodyRefList>::create(PT_BODYLIST, "bodies", [&](room::BodyRefList bodies) {
		UID uid = UniqueIDBase().getUID();

		int i = 0;
		for (auto&& body : bodies) {
			auto osc = ci::osc::Message(m_msgName);
			ci::Json json = ci::Json::object();
			json["uid"] = uid;
			json["index"] = i;
			json["amount"] = bodies.size();
			json["body"] = body->toJson();
			osc.append(json.dump());
			m_oscPort->send(osc);

			auto path = app::getAssetPath("").string() + "bodies.json";
			ci::writeJson(path, json);

			i++;
		}
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
	m_inputPorts.push_back(json);
	m_inputPorts.push_back(bodies);

	m_allInputPorts.assign(m_inputPorts.begin(), m_inputPorts.end());

	
	m_oscPort = OutputPort<ci::osc::Message>::create(PT_OSC, "osc");
	m_outputPorts.push_back(m_oscPort);

}

act::proc::OSCMsgProcNode::~OSCMsgProcNode() {
}

void act::proc::OSCMsgProcNode::update() {

}

void act::proc::OSCMsgProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x);
	ImGui::InputText("msg name", &m_msgName);
	
	endNodeDraw();
}


ci::Json act::proc::OSCMsgProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["msgname"] = m_msgName;
	return json;
}

void act::proc::OSCMsgProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "msgname", m_msgName);
}
