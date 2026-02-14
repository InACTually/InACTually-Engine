
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/
#define IM_ASSERT(_EXPR) do { if (!(_EXPR)) __debugbreak(); } while (0)
#include "ProcessingModule.hpp"
#include "cinder/CinderImGui.h"
#include "imnodes.h"
#include "Design.hpp"

#include "imnodes_internal.h"

act::mod::ProcessingModule::ProcessingModule() {
	setName("Processing");

	m_focusedContainerNode = nullptr;
	m_rootContainerNode = nullptr;

	m_onFocusCallback = [this](proc::ContainerProcNode* focusedContainerNode) {
		m_focusedContainerNode = focusedContainerNode;
		CI_LOG_V("New Focused Container " << m_focusedContainerNode->getName());
	};

	m_nodeRegistry = std::make_shared<act::proc::ProcNodeRegistry>();
};

act::mod::ProcessingModule::~ProcessingModule() {
}

void act::mod::ProcessingModule::setup(act::room::RoomManagers roomMgrs, act::net::NetworkManagerRef networkMgr) {
	m_roomMgrs = roomMgrs;
	m_networkMgr = networkMgr;

	fs::path path = app::getAssetPath("recentProcessing.json");
	
	if (path.empty()) {
		path = app::getAssetPath("").string() + "recentProcessing.json";
		ci::writeJson(path,""); // touch
		saveToFile(path);
	}

	loadFromFile(path);

	if (getContainerByName("Root") == nullptr)
	{
		m_rootContainerNode = std::make_shared<proc::ContainerProcNode>(0, "Root", m_onFocusCallback);
		m_containers.push_back(m_rootContainerNode);
	}
}

void act::mod::ProcessingModule::cleanUp() {
	saveToFile(app::getAssetPath("recentProcessing.json"));
}

void act::mod::ProcessingModule::update() {
	if(m_rootContainerNode != nullptr)
		m_rootContainerNode->update();
}

void act::mod::ProcessingModule::draw() {
	
}

void act::mod::ProcessingModule::drawGUI() {	

	for (auto&& container : m_containers) {
		container->drawEditor();
	};

	drawNodePool();
}

act::proc::ContainerProcNodeRef act::mod::ProcessingModule::getContainerByUID(act::UID uid){
	for (auto container : m_containers) {
		if (uid == container->getUID())
			return container;
	}
 	return nullptr;
};

act::proc::ContainerProcNodeRef act::mod::ProcessingModule::getContainerByName(std::string name) {
	for (auto container : m_containers) {
		if (name == container->getName())
			return container;
	}
	return nullptr;
}

bool act::mod::ProcessingModule::callRPC(act::UID uid, std::string functionName)
{
	auto node = m_rootContainerNode->getNodeByUID(uid);
	if (node) {
		return node->call(functionName);
	}
	return false;
}

act::proc::ContainerProcNodeRef act::mod::ProcessingModule::createContainerByName(std::string name) {
 
	if (getContainerByName(name) != nullptr) {
		CI_LOG_W("[ProcessingModule] can't create container => choose a different name");
		return nullptr;
	}
	
	if (m_focusedContainerNode == nullptr) {
		return nullptr;
	}

	proc::ContainerProcNodeRef container = std::make_shared<proc::ContainerProcNode>(m_focusedContainerNode->getLevel() + 1, name, m_onFocusCallback);

	m_focusedContainerNode->addContainer(container);
	m_containers.push_back(container);
	return container;
};

void act::mod::ProcessingModule::deleteContainerByUID(act::UID uid) {
	for (int i = 0; i < m_containers.size(); i++) {
		if (m_containers[i]->getUID() == uid) {
			m_containers.erase(m_containers.begin() + i);
			break;
		}
	}
	for (auto&& container : m_containers) {
		container->deleteNodeByUID(uid);
	}
}

void act::mod::ProcessingModule::connect(int from, int to) {
	m_rootContainerNode->connect(m_rootContainerNode->getOutputPortByRuntimeID(from), m_rootContainerNode->getInputPortByRuntimeID(to));
}

void act::mod::ProcessingModule::connect(proc::PortBaseRef from, proc::PortBaseRef to)
{
	m_rootContainerNode->connect(from, to);
}

bool act::mod::ProcessingModule::hasNodeWithUID(act::UID uid)
{
	return m_rootContainerNode->hasNode(uid);
}

act::proc::ProcNodeBaseRef act::mod::ProcessingModule::getNodeByUID(act::UID uid) {
	return m_rootContainerNode->getNodeByUID(uid);
}

act::proc::ContainerProcNodeRef act::mod::ProcessingModule::getContainerByContainingNode(act::UID uid) {

	for (auto&& container : m_containers) {
		for (auto&& node : container->getNodes()) {
			if (node->getUID() == uid) {
				return container;
			}
		}
	}
	return nullptr;
}

void act::mod::ProcessingModule::drawNodePool() {

	ImGui::Begin("ProcessingNode Pool");
		
		if(ImGui::Button("clear editor")) {
			
			if (m_focusedContainerNode != nullptr) {
				//proc::IDBase::resetNextID();
				m_focusedContainerNode->clear();
			}
			
		}
		ImGui::NewLine();

		ImGui::InputText("Name", &m_newGroupName);
		//ImGui::SameLine();

		if (ImGui::Button("new group")) {
			createContainerByName(m_newGroupName);
		}

		if (m_focusedContainerNode != nullptr) {
			if (m_focusedContainerNode->getLevel() > 0) {
				ImGui::NewLine();
				if (m_focusedContainerNode->isEnabled()) {
					if (ImGui::Button("disable Group")) {
						m_focusedContainerNode->disable();
					}
				}
				else {
					if (ImGui::Button("enable Group")) {
						m_focusedContainerNode->enable();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("delete Group")) {
					deleteContainerByUID(m_focusedContainerNode->getUID());
				}

				ImGui::NewLine();
				drawCreateButton("Linker");
				ImGui::NewLine();
			}
		}

		//ImGui::NewLine();
		/*
		if (ImGui::Button("delete selected") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
			deleteSelectedNode();
		}

		if (ImGui::Button("delete link")) {
			deleteSelectedLink();
		}*/


		ImGui::NewLine();
		
		if (ImGui::Button("delete focused Node/Connection"))
		{
			m_focusedContainerNode->removeFocusedNode();
		}

		ImGui::NewLine();

		if (ImGui::CollapsingHeader("Groups")) {
			for (auto&& container : m_containers) {
				if (container->getLevel() == 0 || m_focusedContainerNode->hasNode(container->getUID()))
					continue;
				if (ImGui::Button(container->getTitle().c_str())) {
					m_focusedContainerNode->addNode(container);
				}
			}
		}

		for (auto group : m_nodeRegistry->getGroups()) {
			if (group.first != "" && !group.second.empty() && ImGui::CollapsingHeader(group.first.c_str())) {
				for (auto nodeName : group.second) {
					drawCreateButton(nodeName);
				}
				ImGui::NewLine();
			}
		}

	ImGui::End();
}

void act::mod::ProcessingModule::drawCreateButton(std::string nodeName) {
	ImGui::PushID(nodeName.c_str());
	if (ImGui::Button(nodeName.c_str())) {

		if (m_focusedContainerNode != nullptr) {

			auto node = m_nodeRegistry->create(nodeName);
			if (node) {
				node->setup(m_roomMgrs);
				m_focusedContainerNode->addNode(node);

				CI_LOG_D("[ProcessingModule] add Node to Container : " << m_focusedContainerNode->getName());
			}
			else {
				CI_LOG_W("Node '" << nodeName << "' was not registered correctly!");
			}
		}
	}
	ImGui::PopID();
};

void act::mod::ProcessingModule::saveToFile(fs::path path) {
	ci::writeJson(path, getFullDescription());
}

ci::Json act::mod::ProcessingModule::getFullDescription()
{
	ci::Json nodeConfiguration = ci::Json::object();
	
	auto containers = ci::Json::array();

	for (auto&& container : m_containers) {
		auto containerJson = ci::Json::object();
		containerJson["uid"] = container->getUID();
		containerJson["name"] = container->getName();
		containerJson["title"] = container->getTitle();
		containerJson["level"] = container->getLevel();
		containerJson["params"] = container->toParams();

		containers.push_back(containerJson);
	}

	nodeConfiguration["containers"] = containers;
	return nodeConfiguration;
}

void act::mod::ProcessingModule::load(std::filesystem::path path)
{
	if (m_focusedContainerNode != nullptr) {
		m_focusedContainerNode->clear();
	}
	for (auto&& container : m_containers)
		container->clear();
	m_containers.resize(0);

	proc::IDBase::resetNextID();

	loadFromFile(path);
}

void act::mod::ProcessingModule::save(std::filesystem::path path)
{
	saveToFile(path);
}

act::proc::ProcNodeBaseRef act::mod::ProcessingModule::createNodeByName(std::string nodeName) {
	auto node = m_nodeRegistry->create(nodeName);
	if (node) {
		node->setup(m_roomMgrs);
		//m_nodes.push_back(node);
		m_rootContainerNode->addNode(node);
		return node;
	}

	CI_LOG_W("Node '" << nodeName << "' was not registered correctly!");
	return nullptr;
}

void act::mod::ProcessingModule::deleteNodeByUID(act::UID uid) {
	for (auto&& container : m_containers) {
		//container->
	}
}

//TODO Refactor. put that stuff to container node from JSON
void act::mod::ProcessingModule::loadFromFile(fs::path path) { 
	
	ci::Json nodeConfiguration = ci::loadJson(loadFile(path));
	std::vector<std::pair<proc::ContainerProcNodeRef, act::UID>> containerToContainerMap;
	std::vector<std::pair<proc::ContainerProcNodeRef, std::pair<std::string, std::string>>> containerToLinkMap;
	std::vector<std::pair<proc::ContainerProcNodeRef, ci::Json>> containerToParamsMap;

	for (auto&& container : nodeConfiguration["containers"]) {
		act::UID containerUid = container["uid"];
		std::string name = container["name"];

		int level = container["level"];	
		
		proc::ContainerProcNodeRef  c;
		if (level == 0) {
			c = std::make_shared<proc::ContainerProcNode>(0, "Root", m_onFocusCallback);
			c->setUID(containerUid);

			m_rootContainerNode = c;
		}
		else {
			c = std::make_shared<proc::ContainerProcNode>(level, name, m_onFocusCallback);
			c->setUID(containerUid);
		}
		
		for (auto&& n : container["params"]["nodes"]) {
			act::UID uid = n["uid"];
			std::string name = n["name"];
			std::string title = name;
			if(n["title"].is_string())
				title = n["title"];

			vec2 pos(0, 0);
			if (n.contains("x_pos")) {
				pos = vec2(n["x_pos"], n["y_pos"]);
			}

			auto node = m_nodeRegistry->create(name);
			if (node) {
				node->setUID(uid);
				node->setTitle(title);
				node->setup(m_roomMgrs);

				if (n.contains("params"))
					node->fromParams(n["params"]);

				c->addNode(node);
				node->setPosition(pos);
			}
		}

		for (auto&& nestedContainer : container["params"]["containers"]) {
			act::UID uid = nestedContainer["uid"];
			containerToContainerMap.push_back({c, uid});
		}

		for (auto&& l : container["params"]["links"]) {
			std::string from = l["from"];
			std::string to = l["to"];
			containerToLinkMap.push_back({ c, std::pair<std::string, std::string>(from, to) });
		}
		
		containerToParamsMap.push_back({ c, container["params"] });
		
		m_containers.push_back(c);
	}
	
	for (auto&& p : containerToContainerMap) {
		proc::ContainerProcNodeRef nestedContainer;
		for (auto&& c : m_containers) {
			if (c->getUID() == p.second) {
				nestedContainer = c;
				break;
			}
		}
		p.first->addContainer(nestedContainer);
	}

	for (auto&& value : containerToLinkMap) {
		std::pair<act::UID, std::string> fromInfo = getNodeUIDAndPortName(value.second.first);
		std::pair<act::UID, std::string> toInfo = getNodeUIDAndPortName(value.second.second);
		
		auto fromNode = value.first->getNodeByUID(fromInfo.first);
		auto toNode = value.first->getNodeByUID(toInfo.first);

		if (!fromNode || !toNode) {
			CI_LOG_W("cannot load from file, as cannot find node");
			continue;
		}

		auto out = fromNode->getOutputPortByName(fromInfo.second);
		auto in = toNode->getInputPortByName(toInfo.second);

		value.first->connect(out, in);
	}

	for (auto&& ctp : containerToParamsMap) {
		ctp.first->fromParams(ctp.second);
	}
}
/*
void act::mod::ProcessingModule::deleteSelectedLink() {
	//TODO: delete multiple links at once
	int linkID;
	ImNodes::GetSelectedLinks(&linkID);

	m_links.erase(m_links.begin() + linkID);
}

void act::mod::ProcessingModule::deleteSelectedNode() {

	//TODO: delete multiple nodes at once
	int nodeID;
	ImNodes::GetSelectedNodes(&nodeID);

	int it_nodes = 0, it_links = 0;

	for (auto&& node : m_nodes) {
		int currID = node->getID();
		if (currID == nodeID) {
			//delete Node with currID and all active links
			vector<act::proc::PortBaseRef> inPorts = node->getAllInputPorts();
			vector<act::proc::PortBaseRef> outPorts = node->getAllOutputPorts();

			for (int i = 0; i < inPorts.size(); i++) {
				int inID = inPorts[i]->getID();
				for (int k = 0; k < m_links.size(); k++) {
					int firstID = m_links[k].first;
					int secondID = m_links[k].second;

					if (firstID == inID){
						m_links.erase(m_links.begin() + k);

					}
					else if (secondID == inID){
						m_links.erase(m_links.begin() + k);
					}

					//if ((firstID == inID) || (secondID == inID)) {
					//	//TODO: inPorts[i]->disconnect
					//	m_links.erase(m_links.begin() + k);
					//}
				}
			}

			for (int j = 0; j < outPorts.size(); j++) {
				int outID = outPorts[j]->getID();
				for (int k = 0; k < m_links.size(); k++) {
					if ((m_links[k].first == outID) || (m_links[k].second == outID)) {
						//TODO: outPorts[i]->disconnect
						m_links.erase(m_links.begin() + k);
					}
				}
			}

			//TODO: node-> delete(); //individual delete function per node
			m_nodes.erase(m_nodes.begin() + it_nodes);

			break;
		}
		it_nodes++;
	}

	for (auto&& c : containerToParamsMap) {
		c.first->fromParams(c.second);
	}
	
}*/

std::pair<act::UID, std::string> act::mod::ProcessingModule::getNodeUIDAndPortName(std::string str) {

	std::string delimiter = "\n";
	act::UID nodeUID = str.substr(0, str.find(delimiter));
	std::string portName = str.erase(0, str.find(delimiter) + delimiter.length());

	return std::pair<act::UID, std::string>(nodeUID, portName);
}
