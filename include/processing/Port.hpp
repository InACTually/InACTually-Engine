
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

#pragma once

//#include "input/ListenerBase.hpp"

#include "IDBase.hpp"
#include "UniqueIDBase.hpp"

#include "PortType.hpp"

#include "imnodes.h"

#include <functional>

namespace act {
	namespace proc {

		
		class PortContext {
		public:
			PortContext() {};
			~PortContext() {};
		};
		using PortContextRef = std::shared_ptr<PortContext>; // std::shared_ptr<void>;


		//### PortBase

		class PortBase : public UniqueIDBase, private IDBase {
		public:
			PortBase(PortType type, std::string name) : m_type(type), m_name(name), m_caption(name) {
				m_runtimeID = m_id;
			};
			~PortBase() {};

			PortType getType() { return m_type; };

			std::string getName() { return m_name; };
			std::string getCaption() { return m_caption; };

			int getRuntimeID() { return m_runtimeID; };

			void setName(std::string name) { m_name = name; };
			void setCaption(std::string caption) { m_caption = caption; };

			virtual void draw(int width, bool noCaption = false) = 0;
			
			virtual bool connect(std::shared_ptr<PortBase> port) = 0;
			virtual bool disconnect(std::shared_ptr<PortBase> port) = 0;

			inline bool isEnabled()	{ return m_isEnabled; };
			void enable()		{ m_isEnabled = true; };
			void disable()		{ m_isEnabled = false; };

		protected:
			PortType m_type; 
			std::string m_name;
			std::string m_caption;
			int	m_runtimeID;
			bool m_isEnabled = true;

		private:
		
		}; using PortBaseRef = std::shared_ptr<PortBase>;

		//### Port

		template <class T, class K = PortContextRef>
		class Port : public PortBase {
		public:
			Port(PortType type, std::string name) : PortBase(type, name) {};
			~Port() {};

			void setConnectionCB(std::function<void()> cb) { m_onConnectionCB = cb; };
			void setDisconnectionCB(std::function<void()> cb) { m_onDisconnectionCB = cb; };

			bool connect(PortBaseRef port) override {
				if (!port)
					return false;
				if (port->getType() != getType())
					return false;

				bool connected = addListener(port);
				if (connected && listenerCount() == 1)
					m_onConnectionCB();
				return connected;
			};

			bool disconnect(PortBaseRef port) override {
				if (!port)
					return false;
				if (port->getType() != getType())
					return false;

				bool disconnected = removeListener(port);
				if (disconnected && listenerCount() == 0)
					m_onDisconnectionCB();
				return disconnected;
			}

			virtual void send(T data, K context = nullptr) {
				if (!isEnabled())
					return;

				m_cachedData = data;
				m_cachedDataContext = context;
				m_hasCachedData = true;

				for (auto&& port : m_ports) {
					port->recieve(data, context);
				}
			}

			virtual void recieve(T data, K context = nullptr) {
				m_cachedData = data;
				m_cachedDataContext = context;
				m_hasCachedData = true;
			}

		protected:
			std::vector<std::shared_ptr<Port<T, PortContextRef>>> m_ports;
			std::function<void()> m_onConnectionCB = []() {};
			std::function<void()> m_onDisconnectionCB = []() {};

			bool	m_hasCachedData = false;
			T		m_cachedData;
			K		m_cachedDataContext;

			int listenerCount() {
				return (int)(m_ports.size());
			}

			bool addListener(PortBaseRef port) {
				if (port->getType() == getType()) {
					std::shared_ptr<Port<T, PortContextRef>> p = std::dynamic_pointer_cast<Port<T, PortContextRef>>(port);
					
					if (!p) {
						return false;
					} 

					if(m_hasCachedData)
						p->recieve(m_cachedData, m_cachedDataContext);
					m_ports.push_back(p);

					return true;
				}
				return false;
			}

			bool removeListener(PortBaseRef port) {
				if (std::find_if(m_ports.begin(), m_ports.end(), [&](std::shared_ptr<Port<T, PortContextRef>> pt) { return pt->getRuntimeID() == port->getRuntimeID(); }) == m_ports.end()) {
					return false;
				}
				if (port->getType() == getType()) {
					std::shared_ptr<Port<T, PortContextRef>> p = std::static_pointer_cast<Port<T, PortContextRef>>(port);
					if (!p)
						return false;

					m_ports.erase(std::remove(m_ports.begin(), m_ports.end(), p), m_ports.end());
					return true;
				}
				return false;
			}
		};

		//### OutputPort

		template <class T, class K = PortContextRef>
		class OutputPort : public Port<T, K> {
		public:
			OutputPort(PortType type, std::string name) 
				: Port<T, K>(type, name) {};

			static std::shared_ptr<OutputPort<T, K>> create(PortType type, std::string name) 
			{ 
				return std::make_shared<act::proc::OutputPort<T, K>>(type, name); 
			};
			
			void draw(int width, bool noCaption = false) override {
				//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (act::Settings::get().fontSize * 0.5f));
				ImNodes::BeginOutputAttribute(PortBase::getRuntimeID());
				if (!noCaption)
				{
					ImGui::Indent(width - ImGui::CalcTextSize(PortBase::getCaption().c_str()).x);
					ImGui::Text(PortBase::getCaption().c_str());
				}
				else {
					ImGui::Indent(width);
				}
				ImNodes::EndOutputAttribute();
				//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (act::Settings::get().fontSize * 0.5f));
			}
			int getListenerCount()
			{
				return this->listenerCount();
			}
		};
		template <class T, class K = PortContextRef>
		using OutputPortRef = std::shared_ptr<OutputPort<T, K>>;

		//### InputPort

		template <class T, class K = PortContextRef>
		class InputPort : public Port<T, K> {

		public:
			InputPort(PortType type, std::string name, std::function<void(T)> recieveCallback) 
				: Port<T, K>(type, name), m_recieveFunc(recieveCallback) {};
			InputPort(PortType type, std::string name, std::function<void(T, std::string)> recieveCallback) 
				: Port<T, K>(type, name), m_namedRecieveFunc(recieveCallback), m_sendName(true) {};
			InputPort(PortType type, std::string name, std::function<void(T, std::string)> recieveCallback, bool sendUID) 
				: Port<T, K>(type, name), m_namedRecieveFunc(recieveCallback), m_sendUID(sendUID), m_sendName(!sendUID) {};

			static std::shared_ptr<InputPort<T, K>> create(PortType type, std::string name, std::function<void(T)> recieveCallback)
			{
				return std::make_shared<act::proc::InputPort<T, K>>(type, name, recieveCallback);
			};

			static std::shared_ptr<InputPort<T, K>> create(PortType type, std::string name, std::function<void(T, std::string)> recieveCallback)
			{
				return std::make_shared<act::proc::InputPort<T, K>>(type, name, recieveCallback);
			};

			InputPort(PortType type, std::string name, std::function<void(T, K)> recieveCallback)
				: Port<T, K>(type, name), m_recieveCtxFunc(recieveCallback) { m_wantsContext = true; };
			InputPort(PortType type, std::string name, std::function<void(T, std::string, K)> recieveCallback)
				: Port<T, K>(type, name), m_namedRecieveCtxFunc(recieveCallback), m_sendName(true) { m_wantsContext = true; };
			InputPort(PortType type, std::string name, std::function<void(T, std::string, K)> recieveCallback, bool sendUID)
				: Port<T, K>(type, name), m_namedRecieveCtxFunc(recieveCallback), m_sendUID(sendUID), m_sendName(!sendUID) { m_wantsContext = true; };

			static std::shared_ptr<InputPort<T, K>> create(PortType type, std::string name, std::function<void(T, K)> recieveCallback) 
			{ 
				return std::make_shared<act::proc::InputPort<T, K>>(type, name, recieveCallback); 
			};

			static std::shared_ptr<InputPort<T, K>> create(PortType type, std::string name, std::function<void(T, std::string, K)> recieveCallback) 
			{ 
				return std::make_shared<act::proc::InputPort<T, K>>(type, name, recieveCallback); 
			};
			
			void draw(int width, bool noCaption = false) override {
				//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (act::Settings::get().fontSize * 0.5f));
				ImNodes::BeginInputAttribute(PortBase::getRuntimeID());
				ImGui::Indent(0);
				if (!noCaption)
					ImGui::Text(PortBase::getCaption().c_str());
				ImNodes::EndInputAttribute();
				//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (act::Settings::get().fontSize * 0.5f));
			}

			void recieve(T data, K context = nullptr) override {
				if (!PortBase::isEnabled())
					return;
				

				if (m_wantsContext) {
					if (m_sendUID)
						m_namedRecieveCtxFunc(data, PortBase::getUID(), context);
					else if (m_sendName)
						m_namedRecieveCtxFunc(data, PortBase::getName(), context);
					else
						m_recieveCtxFunc(data, context);
				}
				if (!m_wantsContext) {
					if (m_sendUID)
						m_namedRecieveFunc(data, PortBase::getUID());
					else if (m_sendName)
						m_namedRecieveFunc(data, PortBase::getName());
					else
						m_recieveFunc(data);
				}
			};

		protected:
			std::function<void(T)> m_recieveFunc;
			std::function<void(T, std::string)> m_namedRecieveFunc;

			std::function<void(T, K)> m_recieveCtxFunc;
			std::function<void(T, std::string, K)> m_namedRecieveCtxFunc;

			bool m_sendName		= false;
			bool m_sendUID		= false;
			bool m_wantsContext = false;

		};
		template <class T, class K = PortContextRef>
		using InputPortRef = std::shared_ptr<InputPort<T, K>>;
	}
}