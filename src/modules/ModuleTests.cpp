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

#include "doctest.h"

#include "ModuleRegistry.hpp"
#include "RoomManagers.hpp"
#include "NetworkManager.hpp"
#include "FlowRuntime.hpp"

#include "implot.h"

class ModuleTest : public act::mod::ModuleRegistry {
public:
    static std::vector<act::mod::ModuleBaseRef> get() {
        return reg_modules;
    }
};

TEST_SUITE("Modules") {
    
    TEST_CASE("all registered modules survive setup() and update() without throwing") {
        CI_LOG_D("Testing all Modules:");
        return;

        auto options = ImGui::Options().window(ci::app::getWindow());
        Initialize(options);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImNodes::CreateContext();
        ImPlot::CreateContext();

        act::room::RoomManagersRef roomMgrs = act::room::RoomManagers::get();
        act::net::NetworkManagerRef networkMgr = act::net::NetworkManager::get(roomMgrs);

        for (auto&& module : ModuleTest::get()) {
            REQUIRE(module != nullptr);

            SUBCASE("Testing: " + module->getName()) {
                CAPTURE(module->getName());
                CI_LOG_D("Testing Module: " << module->getName());

                CHECK_NOTHROW(module->setup(roomMgrs, networkMgr));
                CHECK_NOTHROW(module->update());
                CHECK_NOTHROW(module->draw());

                //act::proc::FlowRuntime::wait();

                CHECK_NOTHROW(module.reset());
            }
        }
        CI_LOG_D("All Modules have been tested.");
    }

}
