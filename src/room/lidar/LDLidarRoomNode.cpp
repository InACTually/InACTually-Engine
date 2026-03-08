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

#include "roompch.hpp"
#include "lidar/LDLidarRoomNode.hpp"

act::room::LDLidarRoomNode::LDLidarRoomNode(const std::string& portName, ci::Json description, std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID)
	: LidarRoomNodeBase(portName, description, name, position, rotation, radius, replyUID)
{
	m_driver.RegisterGetTimestampFunctional([]() -> uint64_t {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		});

	int baudrate = 230400;
	if (!m_driver.Start(ldlidar::LDType::LD_19, m_portName, baudrate)) {
		CI_LOG_E("LDLidarRoomNode: failed to start driver on " << m_portName);
		return;
	}

	if (!m_driver.WaitLidarCommConnect(3000)) {
		CI_LOG_W("LDLidarRoomNode: lidar comm connect timeout on " << m_portName);
	}
}

act::room::LDLidarRoomNode::~LDLidarRoomNode()
{
	cleanUp();
}

void act::room::LDLidarRoomNode::setup()
{
}

void act::room::LDLidarRoomNode::update()
{
	LidarRoomNodeBase::update();
	getData();
}

std::vector<ci::vec2>& act::room::LDLidarRoomNode::getData()
{
	ldlidar::Points2D points;
	auto status = m_driver.GetLaserScanData(points, 100);

	

	if (status != ldlidar::LidarStatus::NORMAL)
		return m_lidarData;

	m_lidarData.clear();
	m_lidarData.reserve(points.size());

	for (const auto& p : points) {
		float rad = static_cast<float>(p.angle * M_PI / 180.0f);
		float dist = static_cast<float>(p.distance) / 1000.0f; // mm -> m
		m_lidarData.emplace_back(std::cos(rad) * dist, std::sin(rad) * dist);
	}

	return m_lidarData;
}

void act::room::LDLidarRoomNode::cleanUp()
{
	m_driver.Stop();
	LidarRoomNodeBase::cleanUp();
}
