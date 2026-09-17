
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

#pragma once
#include "cinder/app/App.h"

#include "UniqueIDBase.hpp"
#include "IDBase.hpp"

#include "ProcNodeBase.hpp"
#include "LinkerProcNode.hpp"

 
namespace act {
	namespace proc {
		class ContainerProcNode :  public ProcNodeBase
		{
		public:
			ContainerProcNode(int level, std::string name, std::function <void(ContainerProcNode*)> onFocusCallback);
			virtual ~ContainerProcNode();

			void setup();
			void update();

 
			void draw();
			void drawPorts(int width)	override;
 			void drawEditor();

			void addNode(ProcNodeBaseRef node);
			void setNodes(std::vector<ProcNodeBaseRef> nodes);
			bool hasNode(UID uid);
			void addContainer(std::shared_ptr<ContainerProcNode> container);
			void setLinks(std::vector<std::pair<int, int>> newLinks) {
				m_links = newLinks;
			};

			std::vector<ProcNodeBaseRef> getNodes() {
				return m_nodes;
			}

			std::vector<std::pair<int, int>> getLinks() {
				return m_links;
			}

			ci::Json toParams();
			void fromParams(ci::Json json);
  
			act::proc::PortBaseRef getOutputPortByUID(act::UID uid);
			act::proc::PortBaseRef getInputPortByUID(act::UID uid);
			act::proc::PortBaseRef getOutputPortByRuntimeID(int id);
			act::proc::PortBaseRef getInputPortByRuntimeID(int id);
			
			ProcNodeBaseRef  getNodeByPort(act::UID uid);
			ProcNodeBaseRef  getNodeByUID(act::UID uid);
			bool deleteNodeByUID(UID uid);

			bool getIsFocused()		{ return m_isFocused; };
 			std::string getName()	{ return ProcNodeBase::getName(); };
			int getLevel()			{ return m_level; };
			void setLevel(int lvl)	{ m_level = lvl; };

			bool isEnabled()		{ return m_isEnabled; };
			void enable();
			void disable();

			void LoadEditorState(const char* data, size_t size);
			
			void clear() {
				m_links.resize(0);
				m_nodes.resize(0);
 
				ImNodes::EditorContextFree(m_editorContext);
				m_editorContext = ImNodes::EditorContextCreate();
			};

			void connect(act::UID from, act::UID to);
			void connect(PortBaseRef from, PortBaseRef to);
			std::vector<int> getAllPortsConnectedTo(int runtimeID);

			void setNodePositions(std::vector<std::pair<act::UID, ImVec2>> nodePos) {
				m_nodePositions = nodePos;
				m_updateNodePosition = true;
			};

			act::proc::ProcNodeBaseRef getFocusedNode();
			void removeFocusedNode();

		protected:

		private:
 
			int		m_level;
			bool	m_isEnabled;

			bool	m_isFocused;
			bool	m_updateNodePosition;

			//nodes/links in current container
			std::vector<ProcNodeBaseRef>					m_nodes;
 			std::vector<std::pair<int, int>>				m_links;
			act::proc::PortBaseRef findPortByRuntimeID(int id);
			bool disconnectLink(std::pair<int, int> link);
  
			std::vector<std::pair<act::UID, ImVec2>>		m_nodePositions;
  

			std::function <void(ContainerProcNode*)>		m_onFocusCallback;

			ImNodesEditorContext*							m_editorContext;

			tbb::flow::function_node<act::proc::ProcNodeBaseRef, tbb::flow::continue_msg> m_functionNode {
				FlowRuntime::getGraph(), tbb::flow::unlimited, [&](act::proc::ProcNodeBaseRef node) { 
					ci::ThreadSetup threadSetup;
					node->update();
					return tbb::flow::continue_msg{};  
				}
			};

 
		}; using ContainerProcNodeRef = std::shared_ptr<ContainerProcNode>;
	}
}

