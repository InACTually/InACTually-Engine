
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/
#pragma once

#include "ProcNodeBase.hpp"
#include "OSCReciever.hpp"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class OSCRecieverProcNode : public ProcNodeBase
		{
		public:
			OSCRecieverProcNode();
			~OSCRecieverProcNode();

			PROCNODECREATE(OSCRecieverProcNode);

			void setup(act::room::RoomManagers roomMgrs) override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:

			act::net::OSCRecieverRef	m_reciever;
			int							m_port;
			std::string					m_text;
			bool						m_isRunning;

			std::vector<std::string>	m_endpointNames;
			void			addEndpoint(std::string name);
			void			removeEndpoint(int index);
			void			setEndpointLabel(int index, std::string name);
			std::string					m_endpointName = "";

			void						initialize();

		}; 
		using OSCRecieverProcNodeRef = std::shared_ptr<OSCRecieverProcNode>;
	}
}