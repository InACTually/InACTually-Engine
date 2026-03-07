
/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
    Copyright (c) 2025 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2023-2026

    contributors:
    Lars Engeln - mail@lars-engeln.de
*/

#include "ProcNodeRegistry.hpp"

// 
#include "LinkerProcNode.hpp"

// Animation
#include "CircleMovementProcNode.hpp"
#include "PathMovementProcNode.hpp"

// Audio
#include "AFSynthProcNode.hpp"
#include "Audio3DProcNode.hpp"
#include "Audio3DPlayerProcNode.hpp"
#include "Audio3DPlayerTimestretchProcNode.hpp"
#include "AudioInProcNode.hpp"
#include "AudioPlayerProcNode.hpp"
#include "SpectrumProcNode.hpp"

// Azure Kinect
#include "HandProcNode.hpp"
#include "HeadProcNode.hpp"
#include "KinectProcNode.hpp"
#include "SkeletonFilterProcNode.hpp"

// Light
#include "DMXDimmerProcNode.hpp"
#include "MovingHeadProcNode.hpp"

// Network
#include "JsonMsgProcNode.hpp"
#include "NetworkProcNode.hpp"
#include "OSCMsgProcNode.hpp"
#include "OSCRecieverProcNode.hpp"
#include "OSCSenderProcNode.hpp"
#include "OSCSplitterProcNode.hpp"

// Object & Marker
#include "BlobDetectionProcNode.hpp"
#include "FlowDetectionProcNode.hpp"
#include "MarkerProcNode.hpp"
#include "MarkerDetectionProcNode.hpp"
#include "MovementDetectionProcNode.hpp"
#include "ObjectDetectionProcNode.hpp"

// Person
#include "BodiesFilterProcNode.hpp"
#include "BodyToSoundProcNode.hpp"
#include "BodyTrackingProcNode.hpp"
#include "BoneVectorProcNode.hpp"
#include "FaceDetectionProcNode.hpp"
#include "FaceEmotionProcNode.hpp"
#include "MultiBodyPositionsProcNode.hpp"
#include "SkeletonMovementProcNode.hpp"

// Pointcloud
#include "PointcloudProcNode.hpp"

// Timeline
#include "ClockProcNode.hpp"
#include "EasingProcNode.hpp"
#include "TriggerListProcNode.hpp"

// Utility
#include "BackgroundSubstractionProcNode.hpp"
#include "ColorProcNode.hpp"
#include "ColorMappingProcNode.hpp"
#include "DistributorProcNode.hpp"
#include "IfProcNode.hpp"
#include "ImageEnhancerProcNode.hpp"
#include "NoiseProcNode.hpp"
#include "NumberEnhancerProcNode.hpp"
#include "PositionProcNode.hpp"
#include "SpeedProcNode.hpp"

// general IO
#include "CameraProcNode.hpp"
#include "KeyInProcNode.hpp"
#include "LLMProcNode.hpp"
#include "MicrophoneProcNode.hpp"
#include "MonitorProcNode.hpp"
#include "VideoPlayerProcNode.hpp"
#include "VideoRecorderProcNode.hpp"

// literal
#include "BooleanProcNode.hpp"
#include "NumberProcNode.hpp"
#include "StringProcNode.hpp"
#include "Vector2ProcNode.hpp"
#include "Vector3ProcNode.hpp"

act::proc::ProcNodeRegistry::ProcNodeRegistry()
{

    act::proc::ProcNodeRegistry::add("", "Linker", act::proc::LinkerProcNode::create);

    act::proc::ProcNodeRegistry::add("Animation", "CircleMovement", act::proc::CircleMovementProcNode::create);
    act::proc::ProcNodeRegistry::add("Animation", "PathMovement", act::proc::PathMovementProcNode::create);

    act::proc::ProcNodeRegistry::add("Audio", "AFSynth", act::proc::AFSynthProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "Audio3D", act::proc::Audio3DProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "Audio3DPlayer", act::proc::Audio3DPlayerProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "Audio3DPlayerTimestretch", act::proc::Audio3DPlayerTimestretchProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "AudioIn", act::proc::AudioInProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "AudioPlayer", act::proc::AudioPlayerProcNode::create);
    act::proc::ProcNodeRegistry::add("Audio", "Spectrum", act::proc::SpectrumProcNode::create);

    act::proc::ProcNodeRegistry::add("Azure Kinect", "Hand", act::proc::HandProcNode::create);
    act::proc::ProcNodeRegistry::add("Azure Kinect", "Head", act::proc::HeadProcNode::create);
    act::proc::ProcNodeRegistry::add("Azure Kinect", "Kinect", act::proc::KinectProcNode::create);
    act::proc::ProcNodeRegistry::add("Azure Kinect", "SkeletonFilter", act::proc::SkeletonFilterProcNode::create);

    act::proc::ProcNodeRegistry::add("Light", "DMXDimmer", act::proc::DMXDimmerProcNode::create);
    act::proc::ProcNodeRegistry::add("Light", "MovingHead", act::proc::MovingHeadProcNode::create);

    act::proc::ProcNodeRegistry::add("Network", "JsonMsg", act::proc::JsonMsgProcNode::create);
    act::proc::ProcNodeRegistry::add("Network", "Network", act::proc::NetworkProcNode::create);
    act::proc::ProcNodeRegistry::add("Network", "OSCMsg", act::proc::OSCMsgProcNode::create);
    act::proc::ProcNodeRegistry::add("Network", "OSCReciever", act::proc::OSCRecieverProcNode::create);
    act::proc::ProcNodeRegistry::add("Network", "OSCSender", act::proc::OSCSenderProcNode::create);
    act::proc::ProcNodeRegistry::add("Network", "OSCSplitter", act::proc::OSCSplitterProcNode::create);

    act::proc::ProcNodeRegistry::add("Object & Marker", "BlobDetection", act::proc::BlobDetectionProcNode::create);
    act::proc::ProcNodeRegistry::add("Object & Marker", "FlowDetection", act::proc::FlowDetectionProcNode::create);
    act::proc::ProcNodeRegistry::add("Object & Marker", "Marker", act::proc::MarkerProcNode::create);
    act::proc::ProcNodeRegistry::add("Object & Marker", "MarkerDetection", act::proc::MarkerDetectionProcNode::create);
    act::proc::ProcNodeRegistry::add("Object & Marker", "MovementDetection", act::proc::MovementDetectionProcNode::create);
    act::proc::ProcNodeRegistry::add("Object & Marker", "ObjectDetection", act::proc::ObjectDetectionProcNode::create);

    act::proc::ProcNodeRegistry::add("Person", "BodiesFilter", act::proc::BodiesFilterProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "BodyToSound", act::proc::BodyToSoundProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "BodyTracking", act::proc::BodyTrackingProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "BoneVector", act::proc::BoneVectorProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "FaceDetection", act::proc::FaceDetectionProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "FaceEmotion", act::proc::FaceEmotionProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "MultiBodyPositions", act::proc::MultiBodyPositionsProcNode::create);
    act::proc::ProcNodeRegistry::add("Person", "SkeletonMovement", act::proc::SkeletonMovementProcNode::create);

    act::proc::ProcNodeRegistry::add("Pointcloud", "Pointcloud", act::proc::PointcloudProcNode::create);

    act::proc::ProcNodeRegistry::add("Timeline", "Clock", act::proc::ClockProcNode::create);
    act::proc::ProcNodeRegistry::add("Timeline", "Easing", act::proc::EasingProcNode::create);
    act::proc::ProcNodeRegistry::add("Timeline", "TriggerList", act::proc::TriggerListProcNode::create);

    act::proc::ProcNodeRegistry::add("Utility", "BackgroundSubstraction", act::proc::BackgroundSubstractionProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "Color", act::proc::ColorProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "ColorMapping", act::proc::ColorMappingProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "Distributor", act::proc::DistributorProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "If", act::proc::IfProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "ImageEnhancer", act::proc::ImageEnhancerProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "Noise", act::proc::NoiseProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "NumberEnhancer", act::proc::NumberEnhancerProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "Position", act::proc::PositionProcNode::create);
    act::proc::ProcNodeRegistry::add("Utility", "Speed", act::proc::SpeedProcNode::create);

    act::proc::ProcNodeRegistry::add("general IO", "Camera", act::proc::CameraProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "KeyIn", act::proc::KeyInProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "LLM", act::proc::LLMProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "Microphone", act::proc::MicrophoneProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "Monitor", act::proc::MonitorProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "VideoPlayer", act::proc::VideoPlayerProcNode::create);
    act::proc::ProcNodeRegistry::add("general IO", "VideoRecorder", act::proc::VideoRecorderProcNode::create);

    act::proc::ProcNodeRegistry::add("literal", "Boolean", act::proc::BooleanProcNode::create);
    act::proc::ProcNodeRegistry::add("literal", "Number", act::proc::NumberProcNode::create);
    act::proc::ProcNodeRegistry::add("literal", "String", act::proc::StringProcNode::create);
    act::proc::ProcNodeRegistry::add("literal", "Vector2", act::proc::Vector2ProcNode::create);
    act::proc::ProcNodeRegistry::add("literal", "Vector3", act::proc::Vector3ProcNode::create);

}

bool act::proc::ProcNodeRegistry::add(std::string group, const std::string name, nodeCreateFunc funcCreate)
{
    if (auto it = getGroups().find(group); it == getGroups().end()) {
        getGroups()[group] = std::vector<std::string>();
    }

    if (auto it = getMap().find(name); it == getMap().end()) {
        getMap()[name] = funcCreate;

        // if the entry was not in getMap, than assume that it is also not in getGroups
        getGroups()[group].push_back(name);

        return true;
    }
    return false;
}

std::shared_ptr<act::proc::ProcNodeBase> act::proc::ProcNodeRegistry::create(const std::string& name)
{
    if (auto it = getMap().find(name); it != getMap().end())
        return it->second();

    return nullptr;
}
