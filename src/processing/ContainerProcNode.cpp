
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Fabian Töpfer
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "ContainerProcNode.hpp"

act::proc::ContainerProcNode::ContainerProcNode(int level, std::string name, std::function <void(ContainerProcNode*)> onFocusCallback):ProcNodeBase(name, act::proc::NT_CONTAINER) {
	m_drawSize = glm::ivec2(300, 500);
	
	m_level = level;
	m_isFocused = false;

	m_onFocusCallback = onFocusCallback;
	m_editorContext = ImNodes::EditorContextCreate(); 

	m_updateNodePosition = false;
 
	CI_LOG_I("[ContainerProcNode] created with UID : " << getUID());
};

act::proc::ContainerProcNode::~ContainerProcNode() {
	ImNodes::EditorContextFree(m_editorContext);
};

void act::proc::ContainerProcNode::setup() {};

void act::proc::ContainerProcNode::update() {
	// for (auto&& node : m_nodes) {
	for(int i = 0; i < m_nodes.size();) {
		if (!m_nodes[i]) {
			CI_LOG_W("[ContainerProcNode - " << getTitle() << "] a node is empty");
			m_nodes.erase(m_nodes.begin() + i);
			continue;
		}
		if (m_nodes[i]->isEnabled()) {
			m_functionNode.try_put(m_nodes[i]);
		}

		i++;
	}
};

void act::proc::ContainerProcNode::draw() { // "GroupNode", Node in higher level Container
	beginNodeDraw();

	endNodeDraw();
};

void act::proc::ContainerProcNode::drawPorts(int width) {
	for (auto&& node : m_nodes) {
		auto linker = LinkerNodeRef(dynamic_pointer_cast<LinkerProcNode>(node));
		if (linker != nullptr) {
			if (linker->getIsInput()) {
				linker->getExternalInputPort()->draw(width);
			}
			else {
				linker->getExternalOutputPort()->draw(width);
			}
		}
	}
};

void act::proc::ContainerProcNode::drawEditor() {
	auto darkPrimaryColor	= IM_COL32(util::Design::darkPrimaryColor().r * 255, util::Design::darkPrimaryColor().g * 255, util::Design::darkPrimaryColor().b * 255, 255);
	auto primaryColor		= IM_COL32(util::Design::primaryColor().r * 255, util::Design::primaryColor().g * 255, util::Design::primaryColor().b * 255, 255);
	auto highlightColor		= IM_COL32(util::Design::highlightColor().r * 255, util::Design::highlightColor().g * 255, util::Design::highlightColor().b * 255, 255);
	auto errorColor			= IM_COL32(util::Design::errorColor().r * 255, util::Design::errorColor().g * 255, util::Design::errorColor().b * 255, 255);
 
	
	ImNodes::EditorContextSet(m_editorContext);

	ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"));

	if (isEnabled())
		ImGui::Begin(getTitle().c_str());
	else
		ImGui::Begin((getTitle() + " [disabled]").c_str());
	

	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
		if (!m_isFocused) {
			m_onFocusCallback(this);
			m_isFocused = true;
		}
		
		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) { // (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl))) {
			removeFocusedNode();
		}
	}
	else {
		m_isFocused = false;
	}

	ImNodes::BeginNodeEditor();	

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, darkPrimaryColor);
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, primaryColor);
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, primaryColor);
		ImNodes::PushColorStyle(ImNodesCol_BoxSelector, darkPrimaryColor);
		ImNodes::PushColorStyle(ImNodesCol_Pin, darkPrimaryColor);
		ImNodes::PushColorStyle(ImNodesCol_PinHovered, highlightColor);
		ImNodes::PushColorStyle(ImNodesCol_Link, darkPrimaryColor);
		ImNodes::PushColorStyle(ImNodesCol_LinkHovered, highlightColor);
		ImNodes::PushColorStyle(ImNodesCol_LinkSelected, highlightColor);
		ImNodes::PushColorStyle(ImNodesCol_MiniMapLink, darkPrimaryColor);
		ImNodes::PushColorStyle(ImNodesCol_MiniMapLinkSelected, primaryColor);
		ImNodes::PushColorStyle(ImNodesCol_BoxSelector, primaryColor);
			
		for (auto&& node : m_nodes) {
			node->draw();
		}

		if (m_updateNodePosition) {
			m_updateNodePosition = false;

			for (auto&& n : m_nodePositions) {
				auto node = getNodeByUID(n.first);
				if(node)
					ImNodes::SetNodeEditorSpacePos(node->getID(), n.second);
			}
		}
				
		for (int i = 0; i < m_links.size(); ++i)
		{
			const std::pair<int, int> p = m_links[i];
			// in this case, we just use the array index of the link
			// as the unique identifier
				ImNodes::Link(i, p.first, p.second);
		}

		ImNodes::MiniMap(0.15f, ImNodesMiniMapLocation_BottomRight);
		
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	ImNodes::EndNodeEditor();

	for (auto&& node : m_nodes) {
		int id = -1;
		if (ImNodes::IsNodeHovered(&id) && id == node->getID()) {
			node->setIsHovered(true);
		}
		else {
			node->setIsHovered(false);
		}

		if (ImNodes::IsNodeSelected(node->getID())) {
			node->setIsSelected(true);
		}
		else {
			node->setIsSelected(false);
		}
	}
	

	ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
 
	int start_attr, end_attr;
	if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
	{
		auto out = getOutputPortByRuntimeID(start_attr);
		auto in = getInputPortByRuntimeID(end_attr);

		connect(out, in);
	}

	ImGui::End();
};

void  act::proc::ContainerProcNode::addContainer(std::shared_ptr<ContainerProcNode> container) {
	//get current selected nodes
	//add nodes to container
	//delete old links and create new ones 

	const int num_selected_nodes = ImNodes::NumSelectedNodes();
	std::vector<std::pair<std::string, ImVec2>>	nodePositions;

	if (num_selected_nodes > 0)
	{
		std::vector<int> selected_nodes;
		selected_nodes.resize(num_selected_nodes);
		ImNodes::GetSelectedNodes(selected_nodes.data());

		std::vector<ProcNodeBaseRef> newNodes;
		std::vector<ProcNodeBaseRef> remainingNodes;

		for (auto&& node : m_nodes) {
			bool hit = false;

			for (int i = 0; i < num_selected_nodes; i++) {
				if (node->getID() == selected_nodes[i]) {
					newNodes.push_back(node);
					nodePositions.push_back(std::pair<std::string, ImVec2>{ node->getUID(), node->getPosition() });
					hit = true;
				}
			}
			if (!hit)
				remainingNodes.push_back(node);
		};
		m_nodes = remainingNodes;
		container->setNodes(newNodes);
		container->setNodePositions(nodePositions);
	}
	//TODO set links and delete old ones
	const int num_selected_links = ImNodes::NumSelectedLinks();
	if (num_selected_links > 0)
	{
		std::vector<int> selectedLinks;
		selectedLinks.resize(num_selected_links);
		ImNodes::GetSelectedLinks(selectedLinks.data());

		std::vector<std::pair<int, int>> newLinksForNewContainer;
		std::vector<std::pair<int, int>> newLinks;

		for (int i = 0; i < m_links.size(); i++) {
			if (std::find(selectedLinks.begin(), selectedLinks.end(), i) != selectedLinks.end()) {
				newLinksForNewContainer.push_back(m_links[i]);
			}
			else {
				newLinks.push_back(m_links[i]);
			}

		}
		m_links = newLinks;
		container->setLinks(newLinksForNewContainer);

	}
	m_nodes.push_back(container);

	CI_LOG_V("[ContainerProcNode] adds new Container to " << getTitle() << " on level " << m_level);
};

act::proc::PortBaseRef act::proc::ContainerProcNode::getOutputPortByUID(act::UID uid) {
	
	for (auto&& node : m_nodes) {
		PortBaseRef port = nullptr;

		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container) {
			port = container->getOutputPortByUID(uid);
		}
		else {
			port = node->getOutputPortByUID(uid);
		}

		if (port)
			return port;
	}
	return nullptr;
};

act::proc::PortBaseRef act::proc::ContainerProcNode::getInputPortByUID(act::UID uid) {
	
	for (auto&& node : m_nodes) {
		PortBaseRef port = nullptr;

		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container) {
			port = container->getInputPortByUID(uid);
		}
		else {
			port = node->getInputPortByUID(uid);
		}

		if (port)
			return port;
	}
	return nullptr;
}
act::proc::PortBaseRef act::proc::ContainerProcNode::getOutputPortByRuntimeID(int id) {
	for (auto&& node : m_nodes) {
		PortBaseRef port = nullptr;

		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container) {
			port = container->getOutputPortByRuntimeID(id);
		}
		else {
			port = node->getOutputPortByRuntimeID(id);

		}

		if (port)
			return port;
	}
	return nullptr;
};

act::proc::PortBaseRef act::proc::ContainerProcNode::getInputPortByRuntimeID(int id) {
	
	for (auto&& node : m_nodes) {	
		PortBaseRef port = nullptr; 

		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container != nullptr) {
			port = container->getInputPortByRuntimeID(id);
		}
		else {
			port = node->getInputPortByRuntimeID(id);

		}
		
		if (port != nullptr)
			return port;
	}
	return nullptr;
}

act::proc::ProcNodeBaseRef act::proc::ContainerProcNode::getNodeByPort(act::UID uid) {
	for (auto&& node : m_nodes) {
			auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
			if (container) {
				if (container->getInputPortByUID(uid) != nullptr) {
					for (auto&& n : container->getNodes()) {
						if (n->getInputPortByUID(uid) != nullptr) {
							return n;
						}
					}
				}
				if (container->getOutputPortByUID(uid) != nullptr) {
					for (auto&& n : container->getNodes()) {
						if (n->getOutputPortByUID(uid) != nullptr) {
							return n;
						}
					}
				}
			}
 			if (node->getInputPortByUID(uid) != nullptr) {
				return node;
			}

			if (node->getOutputPortByUID(uid) != nullptr) {
				return node;
			}
	}
	return nullptr;
};

act::proc::ProcNodeBaseRef act::proc::ContainerProcNode::getNodeByUID(act::UID uid) {
	for (auto&& node : m_nodes) {
		if (node->getUID() == uid) {
			return node;
		}
		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container) {
			auto n = container->getNodeByUID(uid);
			if (n) {
				return n;
			}
		}
	}
	return nullptr;
}

bool act::proc::ContainerProcNode::deleteNodeByUID(UID uid)
{
	std::vector<std::pair<int, int>> newLinks = m_links;

	for (int i = 0; i <= m_nodes.size(); i++) {
		if (m_nodes[i]->getUID() != uid)
			continue;

		std::vector<int> rids = m_nodes[i]->getRuntimeIDs();
		for (int i = 0; i < m_links.size(); i++)
		{
			for (auto rid : rids)
			{
				if (m_links[i].first == rid || m_links[i].second == rid)
				{
					disconnectLink(newLinks[i]);
					newLinks.erase(newLinks.begin() + i);
				}
			}
		}
		m_links = newLinks;
		m_nodes.erase(m_nodes.begin() + i);

		return true;
	}
	 
	return false;
};

void act::proc::ContainerProcNode::connect(act::UID from, act::UID to) {
	auto fromPort = getOutputPortByUID(from);
	auto toPort = getInputPortByUID(to);

	connect(fromPort, toPort);
};

std::vector<int> act::proc::ContainerProcNode::getAllPortsConnectedTo(int runtimeID) {
	std::vector<int> ids;
	for (auto&& l : m_links) {
		if (l.first == runtimeID) {
			ids.push_back(l.first);
		};
		if (l.second == runtimeID) {
			ids.push_back(l.second);
		};
	};

	for (auto&& node : m_nodes) {
		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));
		if (container != nullptr) {
			auto v = container->getAllPortsConnectedTo(runtimeID);
			ids.insert(ids.end(), v.begin(), v.end());
		}

	};

	return ids;
};

void act::proc::ContainerProcNode::connect(proc::PortBaseRef from, proc::PortBaseRef to) {

	if (from && to)
	{
		if (from->getType() == PT_GENERIC && to->getType() == PT_GENERIC) {
			CI_LOG_W("You cannot connect 2 generic Ports");
			return;
		}
		auto fromNode = getNodeByPort(from->getUID());
		auto fromLinker = LinkerNodeRef(dynamic_pointer_cast<LinkerProcNode>(fromNode));

		if (fromLinker != nullptr) {
			from = fromLinker->connectToPort(to);
		}

		auto toNode = getNodeByPort(to->getUID());
		auto toLinker = LinkerNodeRef(dynamic_pointer_cast<LinkerProcNode>(toNode));

		if (toLinker != nullptr) {
			to = toLinker->connectFromPort(from);
		}

		if (from->connect(to)) {
			m_links.push_back(std::make_pair(from->getRuntimeID(), to->getRuntimeID()));
		}
	}
};

void act::proc::ContainerProcNode::enable()
{
	for (auto node : m_nodes)
		node->enable();
	m_isEnabled = true;
};

void act::proc::ContainerProcNode::disable()
{
	for (auto node : m_nodes)
		node->disable();
	m_isEnabled = false;
};

void act::proc::ContainerProcNode::LoadEditorState(const char* data, size_t size) {
	//ImNodes::LoadEditorStateFromIniString(m_editorContext, data, size);
};

void act::proc::ContainerProcNode::addNode(ProcNodeBaseRef node) {
	m_nodes.push_back(node);
};
 
void act::proc::ContainerProcNode::setNodes(std::vector<ProcNodeBaseRef> nodes) {
	m_nodes = nodes;
}
bool act::proc::ContainerProcNode::hasNode(UID uid)
{
	for (auto&& node : m_nodes) {
		if (node->getUID() == uid)
			return true;
	}
	return false;
};

void act::proc::ContainerProcNode::fromParams(ci::Json json) {
	std::string panning = json["panning"];
	std::string delimiter = ";";
	int panningX = stoi(panning.substr(0, panning.find(delimiter)));
	int panningY = stoi(panning.erase(0, panning.find(delimiter) + delimiter.length()));
	
	ImNodes::EditorContextResetPanning(glm::vec2(panningX, panningY));

	m_nodePositions.resize(0);

	for (auto&& node : json["nodes"]) {
		int x = node["x_pos"];
		int y = node["y_pos"];
		act::UID uid = node["uid"];

		m_nodePositions.push_back(std::pair<act::UID, ImVec2>(uid, ImVec2(x, y)));
	}
	for (auto&& node : json["containers"]) {
		int x = node["x_pos"];
		int y = node["y_pos"];
		act::UID uid = node["uid"];

		m_nodePositions.push_back(std::pair<act::UID, ImVec2>(uid, ImVec2(x, y)));
	}


	m_updateNodePosition = true;
};

ci::Json act::proc::ContainerProcNode::toParams() {

	std::string editorState = "";
	if (ImNodes::GetCurrentContext())
		editorState = std::string(ImNodes::SaveCurrentEditorStateToIniString());

	ci::Json nodeConfiguration = ci::Json::object();

	glm::vec2 panning = ImNodes::EditorContextGetPanning();
	std::string panningString = std::to_string(panning.x).append(";").append(std::to_string(panning.y));

	nodeConfiguration["panning"] = panningString;


	auto nodes = ci::Json::array();
	auto containers = ci::Json::array();
	for (auto&& node : m_nodes) {
		auto nodeJson = ci::Json::object();
		nodeJson["uid"] = node->getUID();
		nodeJson["name"] = node->getName();
		nodeJson["title"] = node->getTitle();
		nodeJson["x_pos"] = node->getPosition().x;
		nodeJson["y_pos"] = node->getPosition().y;

		auto container = ContainerProcNodeRef(dynamic_pointer_cast<ContainerProcNode>(node));

		if (container == nullptr) {	// is node
			nodeJson["params"] = node->toParams();
			nodes.push_back(nodeJson);

		}
		else { // is container
			containers.push_back(nodeJson);
		}
	}

	auto links = ci::Json::array();
	for (auto&& link : m_links) {
		auto linkJson = ci::Json::object();
		auto out = getOutputPortByRuntimeID(link.first);
		auto in = getInputPortByRuntimeID(link.second);

		auto outNode = getNodeByPort(out->getUID());
		auto inNode = getNodeByPort(in->getUID());

		linkJson["from"] = outNode->getUID().append("\n").append(out->getName());
		linkJson["to"] = inNode->getUID().append("\n").append(in->getName());
		links.push_back(linkJson);
	}

	nodeConfiguration["nodes"] = nodes;
	nodeConfiguration["links"] = links;
	nodeConfiguration["containers"] = containers;

	return nodeConfiguration;
};

act::proc::ProcNodeBaseRef act::proc::ContainerProcNode::getFocusedNode()
{
	ImNodes::EditorContextSet(m_editorContext);
	for (auto&& node : m_nodes) {
		if (ImNodes::IsNodeSelected(node->getID()))	{
			return node;
		}
	}
	return nullptr;
}

void act::proc::ContainerProcNode::removeFocusedNode()
{
	act::proc::ProcNodeBaseRef node = getFocusedNode();

	if(node && deleteNodeByUID(node->getUID()))
		return;

	std::vector<std::pair<int, int>> newLinks = m_links;
	for (int i = 0; i < m_links.size(); i++) {
		if (ImNodes::IsLinkSelected(i))	{
			disconnectLink(newLinks[i]);
			newLinks.erase(newLinks.begin() + i);
			m_links = newLinks;
			return;
		}
	}
};

act::proc::PortBaseRef act::proc::ContainerProcNode::findPortByRuntimeID(int id)
{
	for (auto node : m_nodes) {
		auto port = node->getPortByRuntimeID(id);
		if (port != nullptr) {
			return port;
		}
	}
	return nullptr;
};

bool act::proc::ContainerProcNode::disconnectLink(std::pair<int, int> link)
{
	auto p1 = findPortByRuntimeID(link.first);
	auto p2 = findPortByRuntimeID(link.second);
	if (p1 != nullptr && p2 != nullptr)	{
		return p1->disconnect(p2);
	}
	return false;
};