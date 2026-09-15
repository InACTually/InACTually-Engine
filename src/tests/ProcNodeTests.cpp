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

    auto jsonPort       = act::proc::toJsonInputPort(port);
	auto oscPort        = act::proc::toOscInputPort(port);
	auto boolPort       = act::proc::toBoolInputPort(port);
	auto numberPort     = act::proc::toNumberInputPort(port);
	auto numberListPort = act::proc::toNumberListInputPort(port);
	auto vec2Port       = act::proc::toVec2InputPort(port);
	auto vec2ListPort   = act::proc::toVec2ListInputPort(port);
	auto vec3Port       = act::proc::toVec3InputPort(port);
	auto vec3ListPort   = act::proc::toVec3ListInputPort(port);
	auto quatPort       = act::proc::toQuatInputPort(port);
	auto colorPort      = act::proc::toColorInputPort(port);
	auto colorListPort  = act::proc::toColorListInputPort(port);
	auto textPort       = act::proc::toTextInputPort(port);
	auto imagePort      = act::proc::toImageInputPort(port);
	auto audioPort      = act::proc::toAudioInputPort(port);
	auto audioNodePort  = act::proc::toAudioNodeInputPort(port);
	//auto pointcloudPort = act::proc::toPointcloudInputPort(port);
	auto featurePort    = act::proc::toFeatureInputPort(port);
	auto featureListPort = act::proc::toFeatureListInputPort(port);
	auto bodyPort       = act::proc::toBodyInputPort(port);
	auto bodyListPort   = act::proc::toBodyListInputPort(port);

    ci::Json testJson = ci::app::getAssetPath("bodies.json");

    ci::osc::Message testMsg("/test");

	auto testBools = std::vector<bool>{ true, true, false, true, false, false, true, false };
    for (int i = 0; i < 10; i++)
		testBools.push_back(i % 2 == 0);
	for (int i = 0; i < 100; i++)
		testBools.push_back(i > 50);

    auto testNumbers = act::proc::numberList{ 0.0f, -1.0f, 1.0f, FLT_MAX, FLT_MIN, 42 };
    for (float f = -1.2f; f <= 1.2f; f += 0.01f)
        testNumbers.push_back(f);
    for (float f = -100.0f; f <= 100.0f; f += 1.0f)
        testNumbers.push_back(f);

    auto testVec2s = act::proc::vec2List{ glm::vec2(0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(1.0f, -1.0f), glm::vec2(42.0f, -42.0f), glm::vec2(FLT_MAX, FLT_MAX), glm::vec2(FLT_MAX, FLT_MIN), glm::vec2(FLT_MIN, FLT_MAX), glm::vec2(FLT_MIN, FLT_MIN) };
	auto testVec3s = act::proc::vec3List{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(42.0f, -42.0f, 42.0f), glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX), glm::vec3(FLT_MAX, FLT_MIN, FLT_MAX), glm::vec3(FLT_MIN, FLT_MAX, FLT_MIN), glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN) };
	auto testQuats = std::vector<glm::quat>{ glm::quat(0.0f, 0.0f, 0.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(42.0f, 1.0f, -21.0f, 1.1f), glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f), glm::quat(FLT_MIN, FLT_MIN, FLT_MIN, -1.0f) };
	
    auto testColors = std::vector<ci::Color>{ ci::Color::black(), ci::Color::white(), ci::Color::hex(0xFF0000) };
    for(float f = 0.0f; f <= 1.0f; f += 0.01f)
		testColors.push_back(ci::Color::gray(f));

	auto testTexts = std::vector<std::string>{ "", "test", "The quick brown fox jumps over the lazy dog.", "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };

	auto testImages = std::vector<act::proc::image>{ cv::UMat() };
    testImages.push_back(cv::UMat(0, 0, CV_8UC3, cv::Scalar(0, 0, 0)));
    testImages.push_back(cv::UMat(1, 1, CV_8UC3, cv::Scalar(0, 0, 0)));
    testImages.push_back(cv::UMat(100, 100, CV_8UC3, cv::Scalar(255, 255, 255)));
    testImages.push_back(cv::UMat(5000, 5000, CV_8UC3, cv::Scalar(128, 128, 128)));
    testImages.push_back(toOcv(ci::loadImage(ci::app::getAssetPath("design/splash.png"))).getUMat(cv::ACCESS_FAST));
    testImages.push_back(toOcv(ci::loadImage(ci::app::getAssetPath("design/icon.png"))).getUMat(cv::ACCESS_FAST));

	auto testAudioBuffers = std::vector<ci::audio::BufferRef>{ nullptr };
    testAudioBuffers.push_back(std::make_shared<ci::audio::Buffer>(1, 44100));
    testAudioBuffers.push_back(std::make_shared<ci::audio::Buffer>(2, 48000));
    testAudioBuffers.push_back(std::make_shared<ci::audio::Buffer>(4, 96000));
    auto testAudioBuffer = ci::audio::load(ci::loadFile(ci::app::getAssetPath("sounds/soundscape/soundscape_LarsEngeln (1).wav")), ci::audio::Context::master()->getSampleRate())->loadBuffer();
    testAudioBuffers.push_back(testAudioBuffer);

	auto testAudioNodes = std::vector<ci::audio::NodeRef>{ nullptr };
	testAudioNodes.push_back(ci::audio::Context::master()->makeNode<ci::audio::GainNode>());
	testAudioNodes.push_back(ci::audio::Context::master()->makeNode<ci::audio::BufferPlayerNode>(testAudioBuffer));

	auto testFeatures = std::vector<act::proc::feature>{{"", 0.0f}};
	for (act::proc::number n : testNumbers)
		testFeatures.push_back({ "feature" + std::to_string(n), n });

	auto testBodies = std::vector<act::room::BodyRef>{ nullptr };
    auto testBody = act::room::Body::create();
    if(testJson.contains("body"))
        testBody->fromJson(testJson["body"]);
	testBodies.push_back(testBody);
    for (int i = 0; i < 15; i++) {
		testBody = act::room::Body::create();
		testBody->setUID("testBody" + std::to_string(i));
        //testBodies.push_back(testBody);
    }

    switch (port->getType()) {
    case act::proc::PortType::PT_JSON:
        jsonPort->recieve(ci::Json::object()); act::proc::FlowRuntime::wait();
        jsonPort->recieve(nullptr); act::proc::FlowRuntime::wait();
        jsonPort->recieve(ci::Json::array()); act::proc::FlowRuntime::wait();
        jsonPort->recieve(ci::Json::parse("{}")); act::proc::FlowRuntime::wait();
        jsonPort->recieve(ci::Json::parse(R"({ "value": 42 })")); act::proc::FlowRuntime::wait();
        jsonPort->recieve(testJson);
        break;
    case act::proc::PortType::PT_OSC:
        oscPort->recieve(ci::osc::Message()); act::proc::FlowRuntime::wait();
        testMsg.append(42);
        oscPort->recieve(testMsg); act::proc::FlowRuntime::wait();
        testMsg.append("42");
        oscPort->recieve(testMsg);
        break;
    case act::proc::PortType::PT_BOOL:
        for (auto b : testBools) {
            boolPort->recieve(b);
        }
        break;
    case act::proc::PortType::PT_NUMBER:
        for (act::proc::number n : testNumbers) {
            numberPort->recieve(n); act::proc::FlowRuntime::wait();
        }
    case act::proc::PortType::PT_NUMBERLIST: break;
        numberListPort->recieve(act::proc::numberList{}); act::proc::FlowRuntime::wait();
        numberListPort->recieve(testNumbers);
    case act::proc::PortType::PT_VEC2:
        for (auto&& v : testVec2s) {
            vec2Port->recieve(v); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_VEC2LIST:
        vec2ListPort->recieve(act::proc::vec2List{}); act::proc::FlowRuntime::wait();
        vec2ListPort->recieve(testVec2s);
        break;
    case act::proc::PortType::PT_VEC3:
        for (auto&& v : testVec3s) {
            vec3Port->recieve(v); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_VEC3LIST:
        vec3ListPort->recieve(act::proc::vec3List{}); act::proc::FlowRuntime::wait();
        vec3ListPort->recieve(testVec3s);
        break;
    case act::proc::PortType::PT_QUAT:
        for (auto&& q : testQuats) {
            quatPort->recieve(q); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_COLOR:
        for (auto&& c : testColors) {
            colorPort->recieve(c); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_COLORLIST:
        colorListPort->recieve(std::vector<ci::Color>{}); act::proc::FlowRuntime::wait();
        colorListPort->recieve(testColors);
        break;
    case act::proc::PortType::PT_TEXT:
        for (auto&& t : testTexts) {
            textPort->recieve(t); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_IMAGE:
        for (auto&& img : testImages) {
            imagePort->recieve(img); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_AUDIO:
        for (auto&& audio : testAudioBuffers) {
            audioPort->recieve(audio); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_AUDIONODE:
        for (auto&& audioNode : testAudioNodes) {
            audioNodePort->recieve(audioNode); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_FEATURE:
        for (auto&& feature : testFeatures) {
            featurePort->recieve(feature); act::proc::FlowRuntime::wait();
        }
        break;
    case act::proc::PortType::PT_FEATURELIST:
        featureListPort->recieve(std::vector<act::proc::feature>{}); act::proc::FlowRuntime::wait();
        featureListPort->recieve(testFeatures);
        break;
    case act::proc::PortType::PT_BODY:
        for (auto&& body : testBodies) {
            bodyPort->recieve(body); act::proc::FlowRuntime::wait();
        }
            break;
		case act::proc::PortType::PT_BODYLIST:
			bodyListPort->recieve(std::vector<act::room::BodyRef>{}); act::proc::FlowRuntime::wait();
			bodyListPort->recieve(testBodies);
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

            CAPTURE(entry.first);
            CI_LOG_D("Testing ProcNode: " << entry.first);

            CHECK_NOTHROW(node->setup(roomMgrs));
            CHECK_NOTHROW(node->update());

            for (auto&& port : node->getAllInputPorts()) {
                CHECK_NOTHROW(testPort(port));
                act::proc::FlowRuntime::wait();
            }

            act::proc::FlowRuntime::wait();

            CHECK_NOTHROW(node.reset());
        }
        CI_LOG_D("All ProcNodes have been tested.");
    }

}
