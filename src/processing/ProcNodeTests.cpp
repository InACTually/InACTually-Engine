/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2026 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2026

    contributors:
    Lars Engeln - mail@lars-engeln.de
*/

// Tests for act::proc::ProcNodeRegistry:
// every registered ProcNode is created, set up with a fully populated
// act::room::RoomManagersRef and then update()d once. The test only verifies
// that no exception escapes setup()/update() (i.e. no crash - noexcept behavior).

#include "doctest.h"

#include "ProcNodeRegistry.hpp"
#include "RoomManagers.hpp"
#include "FlowRuntime.hpp"

#include "implot.h"


void testPort(act::proc::PortBaseRef port) {
    CAPTURE(port->getName());

    auto boolPort = std::dynamic_pointer_cast<act::proc::InputPort<bool>>(port);

    switch (port->getType()) {
        case act::proc::PortType::PT_JSON: break;
        case act::proc::PortType::PT_OSC: break;
        case act::proc::PortType::PT_BOOL:
            
            boolPort->recieve(false);
            boolPort->recieve(true);
            // boolPort->recieve(true);
            // boolPort->recieve(false);
            // boolPort->recieve(false);
            // boolPort->recieve(true);
        case act::proc::PortType::PT_NUMBER: break;
        case act::proc::PortType::PT_NUMBERLIST: break;
        case act::proc::PortType::PT_VEC2: break;
        case act::proc::PortType::PT_VEC2LIST: break;
        case act::proc::PortType::PT_VEC3: break;
        case act::proc::PortType::PT_VEC3LIST: break;
        case act::proc::PortType::PT_QUAT: break;
        case act::proc::PortType::PT_COLOR: break;
        case act::proc::PortType::PT_COLORLIST: break;
        case act::proc::PortType::PT_TEXT:

        break;
    default:
        break;
    }
}

TEST_SUITE("ProcNodeRegistry") {
    
    TEST_CASE("all registered nodes survive setup() and update() without throwing") {
        CI_LOG_D("Testing all ProcNodes:");

        auto options = ImGui::Options().window(ci::app::getWindow());
        Initialize(options);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImNodes::CreateContext();
        ImPlot::CreateContext();

        act::proc::ProcNodeRegistry registry;
        act::room::RoomManagersRef roomMgrs = act::room::RoomManagers::get();

        for (auto&& entry : registry.getMap()) {
            auto node = registry.create(entry.first);
            REQUIRE(node != nullptr);

            //SUBCASE("Testing: " + entry.first) {
                CAPTURE(entry.first);
                CI_LOG_D("Testing ProcNode: " << entry.first);

                CHECK_NOTHROW(node->setup(roomMgrs));
                CHECK_NOTHROW(node->update());

                for (auto&& port : node->getAllInputPorts()) {
                    testPort(port);
                }

                act::proc::FlowRuntime::wait();

                CHECK_NOTHROW(node.reset());
            //}
        }
        CI_LOG_D("All ProcNodes have been tested..");
    }

}
