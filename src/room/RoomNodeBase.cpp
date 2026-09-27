
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

#include "roompch.hpp"
#include "RoomNodeBase.hpp"


// unsigned int act::proc::IDBase::nextID = 1;

act::net::NetworkPublisherRef act::room::RoomNodeBase::m_publisher = nullptr;

act::room::RoomNodeBase::RoomNodeBase(std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: m_name(name), m_deviceName(name), m_caption(name)
{
	/*m_positionInPort = act::proc::InputPort<glm::vec3>::create(act::proc::PT_VEC3, "posIn", [&](glm::vec3 position) {
		setPosition(position);
		});

	m_positionOutPort = act::proc::OutputPort<glm::vec3>::create(act::proc::PT_VEC3, "posOut");
	*/
	
	//auto scoped = setReplyUID(replyUID);
	publishChanges("name", m_name, act::net::PT_ROOMNODE_CREATE, replyUID);

	setPosition(position);
	setRotation(rotation);
	setRadius(radius);

	m_posFlt = OneEuroFilterV3::create(3.0f, 0.006f, 3.0f, 100.0f);
	m_rotFlt = OneEuroFilterV3::create(3.0f, 0.006f, 3.0f, 100.0f);
	m_orientFlt = OneEuroFilterQ::create(3.0f, 0.006f, 3.0f, 100.0f);
	
	m_posFlt->insertValue(getPosition());
	m_rotFlt->insertValue(getRotation());	
	m_orientFlt->insertValue(getOrientation());
}

act::room::RoomNodeBase::~RoomNodeBase()
{
	publishChanges("name", m_name, act::net::PT_ROOMNODE_DELETE);
}

void act::room::RoomNodeBase::drawSettings()
{
	ImGui::InputText("name", &m_caption);
	//ImGui::InputText("UID", &m_uid, ImGuiInputTextFlags_ReadOnly);

	ImGui::Checkbox("fixed", &m_isFixed);
	ImGui::SameLine();
	ImGui::Checkbox("is smoothing", &m_isSmoothing);

	if (ImGui::DragFloat3("position", &m_position, 0.01f)) {
		setPosition(m_position);
	}
	glm::vec3 rot = glm::degrees(getRotation());
	if (ImGui::DragFloat3("rotation", &rot, 1.0f, -0.1f, 360.1f)) {
		if (rot.x > 360.0f)
			rot.x = 0.0f;
		else if (rot.x < 0.0f)
			rot.x = 360.0f;
		if (rot.y > 360.0f)
			rot.y = 0.0f;
		else if (rot.y < 0.0f)
			rot.y = 360.0f;
		if (rot.z > 360.0f)
			rot.z = 0.0f;
		else if (rot.z < 0.0f)
			rot.z = 360.0f;
		setRotation(glm::radians(rot));
	}
	if (ImGui::Checkbox("isLookingAt", &m_isLookingAt)) {
		isLookingAt(m_isLookingAt);
	}
	ImGui::InputInt("Marker ID", &m_markerID);
	
	ImGui::Separator();
	drawSpecificSettings();
}

void act::room::RoomNodeBase::setPosition(glm::vec3 position, bool publish)
{
	if (m_isFixed)
		return;

	if (m_isSmoothing) {
		m_posFlt->insertValue(position);
		m_position = m_posFlt->getValue();
	}
	else {
		m_position = position;
	}

	if (isLookingAt())
		lookAt(m_lookAt);

	updateTransform();

	onPosition("", publish);
	//m_positionOutPort->send(position);
}

void act::room::RoomNodeBase::setRotation(glm::vec3 rotation, bool publish)
{
	if (m_isFixed)
		return;

	rotation.x = fmodf(rotation.x, glm::two_pi<float>());
	rotation.y = fmodf(rotation.y, glm::two_pi<float>());
	rotation.z = fmodf(rotation.z, glm::two_pi<float>());

	
	if (m_isSmoothing) {
		m_rotFlt->insertValue(rotation);
		m_rotation = m_rotFlt->getValue();
		m_orientFlt->insertValue(glm::normalize(glm::quat(m_rotation)));
		m_orientation = glm::normalize(m_orientFlt->getValue());
	}
	else {
		m_rotation = rotation;
		m_orientation = glm::normalize(glm::quat(m_rotation));
	}

	updateTransform();

	onOrientation("", publish);
}

void act::room::RoomNodeBase::setOrientation(glm::quat orientation, bool publish)
{
	if (m_isFixed)
		return;

	if (m_isSmoothing) {
		m_orientFlt->insertValue(glm::normalize(orientation));
		m_orientation = glm::normalize(m_orientFlt->getValue());
		m_rotFlt->insertValue(glm::eulerAngles(m_orientation));
		m_rotation = m_rotFlt->getValue();
	}
	else {
		m_orientation = glm::normalize(orientation);
		m_rotation = glm::eulerAngles(m_orientation);
	}

	updateTransform();

	onOrientation("", publish);
}

void act::room::RoomNodeBase::setRadius(float radius)
{
	if (m_isFixed)
		return;

	m_radius = radius;
	updateTransform();
}

void act::room::RoomNodeBase::setTriMesh(ci::TriMeshRef triMesh)
{
	m_triMesh = triMesh;
	auto lambertShader = ci::gl::getStockShader(ci::gl::ShaderDef().color().lambert());
	auto colorShader = ci::gl::getStockShader(ci::gl::ShaderDef().color());

	m_mesh = ci::gl::Batch::create(*m_triMesh, colorShader);

	m_bounds = m_triMesh->calcBoundingBox();
}

void act::room::RoomNodeBase::lookAt(glm::vec3 at)
{
	m_isLookingAt = true;
	m_lookAt = at;
	glm::vec3 gaze = normalize(at - m_position);
	setOrientation(glm::rotation(glm::vec3(0.0f), gaze));
}

bool act::room::RoomNodeBase::isLookingAt()
{
	return m_isLookingAt;
}

void act::room::RoomNodeBase::isLookingAt(bool isLooking)
{
	m_isLookingAt = isLooking;
	if (m_isLookingAt)
		lookAt(m_lookAt);
}

bool act::room::RoomNodeBase::hit(glm::vec3 pos)
{
	if (glm::length(getPosition() - pos) < getRadius()) {
		m_isHovered = true;
		return true;
	}
	return false;
}

bool act::room::RoomNodeBase::hitRay(ci::Ray ray)
{
	if(!m_triMesh)
		setTriMesh(ci::TriMesh::create(ci::geom::Cube()));

	ci::AxisAlignedBox worldBoundsApprox = m_bounds.transformed(m_transform); // fast

	if (!worldBoundsApprox.intersects(ray))
		return false;

	// Set initial distance to something far, far away.
	float result = FLT_MAX;

	// Traverse triangle list and find the closest intersecting triangle.
	const size_t polycount = m_triMesh->getNumTriangles();

	float distance = 0.0f;
	for (size_t i = 0; i < polycount; ++i) {
		// Get a single triangle from the mesh.
		glm::vec3 v0, v1, v2;
		m_triMesh->getTriangleVertices(i, &v0, &v1, &v2);

		// Transform triangle to world space.
		v0 = glm::vec3(m_transform * ci::vec4(v0, 1.0));
		v1 = glm::vec3(m_transform * ci::vec4(v1, 1.0));
		v2 = glm::vec3(m_transform * ci::vec4(v2, 1.0));

		// Test to see if the ray intersects this triangle.
		if (ray.calcTriangleIntersection(v0, v1, v2, &distance)) {
			// Keep the result if it's closer than any intersection we've had so far.
			if (distance < result) {
				result = distance;

				// Assuming this is the closest triangle, we'll calculate our normal
				// while we've got all the points handy.
				//*pickedNormal = normalize(cross(v1 - v0, v2 - v0));
			}
		}
	}

	// Did we have a hit?
	if (distance > 0) {
		// Calculate the exact position of the hit.
		//*pickedPoint = ray.calcPosition(result);

		return true;
	}
	else
		return false;
}
/*
void act::room::RoomNodeBase::connectPositionPort(std::shared_ptr<RoomNodeBase> node)
{
	auto port = node->getOutputPort();
	port->connect(m_positionInPort);
	m_copyPositionUID = node->getUID();
}

void act::room::RoomNodeBase::disconnectPositionOutPort(act::proc::InputPortRef<glm::vec3> inputPort)
{
	m_positionOutPort->disconnect(inputPort);
}

void act::room::RoomNodeBase::sendCurrentPosition()
{
	m_positionOutPort->send(m_position);
}
*/
void act::room::RoomNodeBase::onPosition(act::UID replyUID, bool publish)
{
	if(publish)
		publishChanges("position", util::valueToJson(m_position), net::PT_ROOMNODE_UPDATE, replyUID);
}
void act::room::RoomNodeBase::onOrientation(act::UID replyUID, bool publish)
{
	if (publish)
		publishChanges("orientation", util::valueToJson(m_orientation), net::PT_ROOMNODE_UPDATE, replyUID);
}

void act::room::RoomNodeBase::updateTransform() {
	m_transform = ci::translate(getPosition()) * glm::toMat4(m_orientation) * ci::scale(glm::vec3(getRadius()));
}

void act::room::RoomNodeBase::publishChanges()
{
	if (m_publisher)
		m_publisher->publishChanges(getUID(), toJson());
}

void act::room::RoomNodeBase::publishChanges(std::string key, ci::Json data, net::PublishType type, act::UID replyUID)
{
	if (replyUID.empty())
		replyUID = m_replyUID;
	if (m_publisher)
		m_publisher->publishChanges(getUID(), key, data, type, replyUID);
}

void act::room::RoomNodeBase::publishParam(std::string key, ci::Json value)
{
	ci::Json param = ci::Json::object();
	param[key] = value;
	if (m_publisher)
		m_publisher->publishChanges(getUID(), "params", param);
}

ci::Json act::room::RoomNodeBase::toJson()
{
	auto json = ci::Json::object();
	json["uid"]			= getUID();
	json["name"]		= m_name;
	json["deviceName"]	= m_deviceName;
	json["caption"]		= getCaption();
	json["isFixed"]		= m_isFixed;
	json["isSmoothing"]	= m_isSmoothing;
	json["position"]	= util::valueToJson(getPosition());
	json["orientation"] = util::valueToJson(getOrientation());
	//json["rotation"]	= util::valueToJson(getRotation());
	json["radius"]		= getRadius();
	json["lookAt"]		= util::valueToJson(getLookAt());
	json["isLookingAt"] = isLookingAt();
	json["markerID"]	= getMarkerID();
	json["params"]		= toParams();

	return json;
}

void act::room::RoomNodeBase::fromJson(ci::Json json, act::UID replyUID)
{
	if (json.contains("uid") && json["uid"] != "") {
		setUID(json["uid"]);
	}
	util::setValueFromJson(json, "name", m_name);
	util::setValueFromJson(json, "deviceName", m_deviceName);
	util::setValueFromJson(json, "isSmoothing", m_isSmoothing);

	if (json.contains("caption")) {
		setCaption(json["caption"]);
	}
	glm::vec3 position = glm::vec3(0.0f);
	if (util::setValueFromJson(json, "position", position)) {
		setPosition(position, false);
	}
	glm::quat orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	if (util::setValueFromJson(json, "orientation", orientation)) {
		setOrientation(orientation, false);
	}

	// after positioning
	util::setValueFromJson(json, "isFixed", m_isFixed);

	if (json.contains("radius")) {
		setRadius(json["radius"]);
	}

	if (json.contains("params")) {
		fromParams(json["params"]);
	}

	if (m_publisher)
		m_publisher->publishChanges(replyUID, json);
}

act::room::ScopedReplyUID act::room::RoomNodeBase::setReplyUID(UID replyUID, bool doNotScope)
{
	m_replyUID = replyUID;

	if (doNotScope)
		return act::room::ScopedReplyUID();

	return act::room::ScopedReplyUID(&m_replyUID);
}

void act::room::RoomNodeBase::enableStatusColor()
{
	ci::gl::color(ci::ColorA(ci::Color::white(), 0.6f));
	if (getIsEmphasized())
		ci::gl::color(ci::ColorA(ci::Color::white(), 0.85f));
	if (getIsUnfolded())
		ci::gl::color(util::Design::highlightColor(0.85f));
	if(!m_isConnected)
		ci::gl::color(util::Design::errorColor(0.85f));
}

bool act::room::RoomNodeBaseRegistry::add(const std::string name, nodeCreateFunc funcCreate)
{
    if (auto it = getMap().find(name); it == getMap().end()) {
        getMap()[name] = funcCreate;
        return true;
    }
    return false;
}

std::shared_ptr<act::room::RoomNodeBase> act::room::RoomNodeBaseRegistry::create(const std::string& name)
{
    if (auto it = getMap().find(name); it != getMap().end())
        return it->second();

    return nullptr;
}