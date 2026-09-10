
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/
#pragma once

#include "ProcNodeBase.hpp"
#include "OSCServer.hpp"


namespace act {
	namespace proc {

		class OSCSenderProcNode : public ProcNodeBase
		{
		public:
			OSCSenderProcNode();
			~OSCSenderProcNode();

			PROCNODECREATE(OSCSenderProcNode);

			void setup(act::room::RoomManagersRef roomMgrs) override;
			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			void onOSC(ci::osc::Message msg);

		private:

			act::net::OSCServerRef	    m_server;
			asio::ip::address			m_address;
			std::string					m_addressString;
			int							m_port;
			std::string					m_text;
			bool						m_isRunning;

			void						initialize();

		}; 
		using OSCSenderProcNodeRef = std::shared_ptr<OSCSenderProcNode>;
	}
}