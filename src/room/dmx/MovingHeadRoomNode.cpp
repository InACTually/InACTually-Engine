
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "dmx/MovingHeadRoomNode.hpp"

#include "RGBAWHelper.h"

act::room::MovingHeadRoomNode::MovingHeadRoomNode(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID)
	: DMXRoomNodeBase(dmxInterface, description, startAddress), RoomNodeBase("movinghead", position, rotation, radius, replyUID)
{
	util::setValueFromJson(description, "panRange",			m_panRange);
	util::setValueFromJson(description, "tiltRange",		m_tiltRange);
	util::setValueFromJson(description, "panCenterOffset",	m_panCenterOffset);
	util::setValueFromJson(description, "tiltOffset",		m_tiltOffset);
	util::setValueFromJson(description, "beamAngle",		m_beamAngle);
	util::setValueFromJson(description, "strobeSpeed",		m_strobeSpeed);

	m_hasFineAdjust = m_channelMapping.find("finePan")	!= m_channelMapping.end() && m_channelMapping.find("fineTilt") != m_channelMapping.end();
	m_hasWhite		= m_channelMapping.find("W")		!= m_channelMapping.end();
	m_hasAmber		= m_channelMapping.find("A")		!= m_channelMapping.end();
	m_hasUV			= m_channelMapping.find("UV")		!= m_channelMapping.end();
	m_hasColorWheel = m_channelMapping.find("color")	!= m_channelMapping.end();
	m_hasStrobe		= m_channelMapping.find("strobe")	!= m_channelMapping.end();
	m_hasGobo		= m_channelMapping.find("gobo")		!= m_channelMapping.end();
	m_hasZoom		= m_channelMapping.find("zoom")		!= m_channelMapping.end();

	m_yaw		= 0.0f; // radians
	m_pitch		= 0.0f; // radians
	m_phi		= 0.0f;	// radians
	m_theta		= 0.0f;	// radians
	m_pan		= util::MinMaxValue<float>(0.0f, m_panRange);   // degree
	m_tilt		= util::MinMaxValue<float>(0.0f, m_tiltRange);  // degree
	m_dimmer	= util::MinMaxValue<float>(0.0f, 1.0f);
	m_dimmerMul	= util::MinMaxValue<float>(0.0f, 1.0f);
	m_dimmerMul.setValue(1.0f);
	m_dimmerPreHighlight = util::MinMaxValue<float>(0.0f, 1.0f);
	m_dimmerPreHighlight.value = 0;
	m_speed		= util::MinMaxValue<float>(0.0f, 1.0f);
	m_zoom		= util::MinMaxValue<float>(0.0f, 1.0f);
	m_strobe	= util::MinMaxValue<float>(0.0f, 1.0f);
	m_UV		= util::MinMaxValue<float>(0.0f, 1.0f);

	m_panCenterOffset = (m_panRange - 360.0f) * 0.5f; // half leftover after "full" pan, so full circle
	m_tiltCenterOffset = (m_tiltRange - 180.0f) * 0.5f; // half leftover after "full" tilt, so half circle

	m_beamAngleMin = m_beamAngle;
	m_beamAngleMax = m_beamAngle;
	if (m_hasZoom) {
		util::setValueFromJson(description, "beamAngleMin", m_beamAngleMin);
		util::setValueFromJson(description, "beamAngleMax", m_beamAngleMax);
	}
	
	if (m_hasGobo) {
		auto goboMap = description["goboMap"];
		int amount = 0;
		util::setValueFromJson(goboMap, "amount", amount);
		m_gobo = util::MinMaxValue<int>(0, amount);
		m_goboShake = util::MinMaxValue<float>(0.0f, 1.0f);
	}

	if (m_hasColorWheel) {
		createColorWheelLookUp(description["colorMap"]);
	}

	m_color = ci::Color::white();
	m_colorPreHighlight = ci::Color::white();

	m_gaze = glm::quat(m_upDir);
	m_previousGaze = glm::quat(m_upDir);

	m_lookAtFlt = OneEuroFilterV3::create(3.0f, 0.006f, 3.0f, 100.0f);

	//m_triMesh = ci::TriMesh::create(ci::geom::Cone()); // m_triMesh is for intersection

	m_cameraPersp = ci::CameraPersp(64, 64, m_beamAngle, 0.1f, 5.0f);
	m_cameraPersp.setEyePoint(vec3(0.0f));
	setPosition(position);
	setRotation(rotation);
	lookAt(position + (m_orientation * m_upDir));

	setSpeed(0.042f);
}

act::room::MovingHeadRoomNode::~MovingHeadRoomNode()
{
}

void act::room::MovingHeadRoomNode::setup()
{
	
}

void act::room::MovingHeadRoomNode::update()
{
	
}

void act::room::MovingHeadRoomNode::draw()
{
	gl::ScopedColor color;

	// 
	// gl::drawVector(m_position, m_position + glm::normalize(m_orientation * m_frontDir)); // sanity-check
	
	// drawVector only if debugging
	/*
	gl::color(ci::Color(0, 0, 255));
	gl::drawVector(m_position, m_position + glm::normalize(glm::normalize(m_orientation * m_gaze) * m_frontDir)); // sanity-check
	gl::color(ci::Color(255, 0, 0));
	gl::drawVector(m_position, m_position + glm::normalize(m_lookAt - m_position));
	*/
	gl::pushMatrices();
		gl::translate(m_position);
		gl::rotate(m_orientation);

		// drawVector only if debugging
	/*
		gl::color(ci::Color(0, 255, 0));
		gl::drawVector(ci::vec3(0.0f), glm::normalize(m_gaze * m_frontDir)); // sanity-check

		gl::color(Color::white());
		 
		gl::drawVector(ci::vec3(0.0f), m_upDir * vec3(0.5));
		gl::drawVector(ci::vec3(0.0f), m_frontDir * vec3(0.5));		

		gl::color(Color(1.0f, 1.0f, 0.0f));
		*/

		//vec3 v = m_cameraPersp.getViewDirection();
		//gl::drawVector(ci::vec3(0.0f), normalize(vec3(v.x, 0.0f, v.z)));
		//gl::drawVector(ci::vec3(0.0f), normalize(vec3(0.0f, v.y, v.z)));

		enableStatusColor();

		gl::drawCube(ci::vec3(0.0f,-0.15f,0.0f), ci::vec3(0.2f, 0.05f, 0.2f));
		gl::rotate(m_phi, m_upDir);

		gl::drawCube(ci::vec3(-0.075f, -0.075f, 0.0f), ci::vec3(0.05f, 0.15f, 0.05f));
		gl::drawCube(ci::vec3(0.075f, -0.075f, 0.0f), ci::vec3(0.05f, 0.15f, 0.05f));

		gl::pushMatrices();
			gl::rotate(m_theta - glm::half_pi<float>(), m_rightDir);
			gl::drawCube(ci::vec3(0.0f), ci::vec3(0.1f, 0.125f, 0.1f));
			
			// drawVector only if debugging
			/*
			gl::drawVector(ci::vec3(0.0f), m_frontDir);
			*/

			//gl::drawCoordinateFrame();

		gl::popMatrices();
	gl::popMatrices();

	if (m_isLookingAt) {
		//glEnable(GL_BLEND); 
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//TODO: change to project color
		
		gl::color(m_color);
		gl::drawSphere(m_lookAt, 0.25f, 8);
	}

		
	gl::color(ColorA(m_color, 0.6f));
	gl::drawFrustum(m_cameraPersp);
	/*
	gl::pushMatrices();
		gl::translate(m_cameraPersp.getEyePoint()); 
		gl::drawVector(ci::vec3(0.0f), m_cameraPersp.getViewDirection());
	
		gl::color(Color(0.0f, 1.0f, 1.0f));
		gl::pushMatrices();
			gl::rotate(glm::eulerAngles(m_cameraPersp.getOrientation()));
			gl::drawVector(ci::vec3(0.0f), vec3(3.0f));
		gl::popMatrices();
		gl::rotate(glm::eulerAngles(glm::rotation(m_frontDir, normalize(m_cameraPersp.getViewDirection()))));
		gl::drawVector(ci::vec3(0.0f), vec3(5.0f));
	gl::popMatrices();
	*/
}

void act::room::MovingHeadRoomNode::cleanUp()
{
	home();
}

void act::room::MovingHeadRoomNode::drawSpecificSettings()
{
	ImGui::SetNextItemWidth(150);
	int startAddress = m_startAddress;
	if (ImGui::DragInt("address", &startAddress))
		setStartAddress(startAddress);

	if (ImGui::Button("flip upside down")) {
		m_upDir.y = -m_upDir.y;
		setRotation(getRotation() + vec3(0, 0, glm::pi<float>()));
		if (m_isLookingAt) // refresh
			lookAt(m_lookAt);
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("pan is flipped", &m_isPanFlipped)) {
		//m_upDir.y = -m_upDir.y;
		if (m_isLookingAt) // refresh
			lookAt(m_lookAt);
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("tilt is flipped", &m_isTiltFlipped)) {
		//m_upDir.y = -m_upDir.y;
		if (m_isLookingAt) // refresh
			lookAt(m_lookAt);
	}

	ImGui::SetNextItemWidth(150);
	float dimmer = m_dimmer.getValue();
	if (ImGui::DragFloat("dimmer", &dimmer, 0.01f, 0.0f, 1.0f))
		setDimmer(dimmer);
	
	if (hasZoom()) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150);
		float zoom = m_zoom.getValue();
		if (ImGui::DragFloat("zoom", &zoom, 0.01f, 0.0f, 1.0f))
			setZoom(zoom);
	}

	ImGui::SetNextItemWidth(150);
	float speed = m_speed.getValue();
	if (ImGui::DragFloat("speed", &speed, 0.01f, 0.0f, 1.0f))
		setSpeed(speed);

	if (hasUV()) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150);
		float uv = m_UV.getValue();
		if (ImGui::DragFloat("UV", &uv, 0.01f, 0.0f, 1.0f))
			setUV(uv);
	}

	ImGui::SetNextItemWidth(200);
	vec3 lAt = getLookAt();
	if (ImGui::DragFloat3("lookAt", &lAt, 0.01f))
		lookAt(lAt);

	ImGui::SetNextItemWidth(200);
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_DisplayHSV);
	ci::Color color = m_color;
	if (ImGui::ColorPicker3("Color", color.ptr())) {
		setColor(color);
	}

	ImGui::NewLine();
	ImGui::BeginDisabled();
	ImGui::SetNextItemWidth(120);
	float yaw = toDegrees(m_yaw);
	ImGui::DragFloat("yaw", &yaw);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	float pitch = toDegrees(m_pitch);
	ImGui::DragFloat("pitch", &pitch);

	ImGui::SetNextItemWidth(120);
	float pan = m_pan.getValue();
	ImGui::DragFloat("pan", &pan);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	float tilt = m_tilt.getValue();
	ImGui::DragFloat("tilt", &tilt);
	ImGui::EndDisabled();
}

ci::Json act::room::MovingHeadRoomNode::toParams()
{
	ci::Json json = ci::Json::object();

	json["pan"]				= m_pan.getValue();
	json["tilt"]			= m_tilt.getValue();
	json["dimmer"]			= m_dimmer.getValue();
	json["dimmerMul"]		= m_dimmerMul.getValue();
	json["color"]			= util::valueToJson(m_color);
	json["speed"]			= m_speed.getValue();
	json["zoom"]			= m_zoom.getValue();
	json["strobe"]			= m_strobe.getValue();
	json["uv"]				= m_UV.getValue();
	json["isPanFlipped"]	= m_isPanFlipped;
	json["isTiltFlipped"]	= m_isTiltFlipped;
	json["startAddress"]    = getStartAddress();
	json["fixtureName"]		= getFixtureName();
	json["lookAt"]			= util::valueToJson(m_lookAt);

	if (m_hasGobo) {
		json["gobo"]		= m_gobo.getValue();
		json["goboShake"]	= m_goboShake.getValue();
	}
	return json;
}

void act::room::MovingHeadRoomNode::fromParams(ci::Json json)
{
	util::setValueFromJson(json, "pan",				m_pan.value);  
	util::setValueFromJson(json, "tilt",			m_tilt.value);
	util::setValueFromJson(json, "dimmer",			m_dimmer.value);
	util::setValueFromJson(json, "dimmerMul",		m_dimmerMul.value);
	util::setValueFromJson(json, "color",			m_color);
	util::setValueFromJson(json, "speed",			m_speed.value);
	util::setValueFromJson(json, "zoom",			m_zoom.value);
	util::setValueFromJson(json, "strobe",			m_strobe.value);
	util::setValueFromJson(json, "uv",				m_UV.value);
	util::setValueFromJson(json, "isPanFlipped",	m_isPanFlipped);
	util::setValueFromJson(json, "isTiltFlipped",	m_isTiltFlipped);
	
	int startAdress = 0; 
	if (util::setValueFromJson(json, "startAddress", startAdress)) {
		setStartAddress(startAdress);
	}

	ci::vec3 lookAtVec;
	if (util::setValueFromJson(json, "lookAt", lookAtVec)) {
			lookAt(lookAtVec);
	}

	/*
	std::string fixtureName;
	util::setValueFromJson(json, "fixtureName", fixtureName);
	if (m_fixtureName != fixtureName)
		CI_LOG_D("setting new fixture by fixtureName is not implemented");
	*/

	setDimmer(m_dimmer.getValue(), false);
	setZoom(m_zoom.getValue(), false);

	setPanTilt(m_pan.getValue(), m_tilt.getValue());

	setColor(m_color, false);

	if (m_hasGobo) {
		util::setValueFromJson(json, "gobo", m_gobo.value);
		util::setValueFromJson(json, "goboShake", m_goboShake.value);
	}
}

void act::room::MovingHeadRoomNode::setColor(ci::Color color, bool publish)
{
	m_color = color;
	RGBAWColor col;
	if (m_hasAmber) {
		col = RGBAWHelper::RGBtoRGBAW(color);
	}
	else {
		col = RGBAWHelper::RGBtoRGBW(color);
	}
	
	if (m_hasColorWheel) {
		vec3 hsv = ci::rgbToHsv(m_color);
		int wheelValue = 0;
		if (m_hasWhiteColorWheel && hsv.y <= 0.1f)
			wheelValue = m_whiteColorWheelValue;
		else
			wheelValue = m_colorWheelLookUp[hsv.x * 255];

		setValue("color", wheelValue);
		m_dimmerMul.setValue(hsv.z);
		setDimmer(m_dimmer.value);
	}
	else {
		setValue("R", col.r);
		setValue("G", col.g);
		setValue("B", col.b);
	}
	if (m_hasWhite)
		setValue("W", col.w);
	if (m_hasAmber)
		setValue("A", col.a);

	if(publish)
		publishParam("color", util::valueToJson(m_color));
}

glm::vec3 toPitchYawRoll(glm::quat data)
{
	glm::vec3 ans;

	double q2sqr = data.y * data.y;
	double t0 = -2.0 * (q2sqr + data.z * data.z) + 1.0;
	double t1 = +2.0 * (data.x * data.y + data.w * data.z);
	double t2 = -2.0 * (data.x * data.z - data.w * data.y);
	double t3 = +2.0 * (data.y * data.z + data.w * data.x);
	double t4 = -2.0 * (data.x * data.x + q2sqr) + 1.0;

	t2 = t2 > 1.0 ? 1.0 : t2;
	t2 = t2 < -1.0 ? -1.0 : t2;

	ans.y = asin(t2);
	ans.x = atan2(t3, t4);
	ans.z = atan2(t1, t0);

	return ans;
}

void act::room::MovingHeadRoomNode::lookAt(ci::vec3 at)
{
	m_isLookingAt = true;

	if (m_isSmoothing) {
		m_lookAtFlt->insertValue(at);
		m_lookAt = m_lookAtFlt->getValue();
	}
	else {
		m_lookAt = at;
	}

	// m_position + glm::normalize(m_orientation * m_frontDir))

	vec3 atVec = glm::normalize(m_lookAt - m_position);
	m_gaze = util::rotationBetween(m_frontDir, atVec);
	if (isnan(m_gaze.w)) {
		m_gaze = m_previousGaze;
	}

	vec3 right = glm::cross(atVec, m_upDir);
	vec3 up = glm::cross(right, atVec);
	if (length(up) > 0.0f) { // otherwise rotationBetween will become nan
		vec3 newUp = m_gaze * m_upDir;
		quat upCorrection = util::rotationBetween(newUp, up);
		m_gaze = upCorrection * m_gaze;
	}
	m_gaze = glm::inverse(m_orientation) * m_gaze;

	vec3 gazeFront = glm::normalize(m_gaze * m_frontDir);
	m_phi		= atan2(gazeFront.x, gazeFront.z);
	m_theta		= acos(gazeFront.y / length(gazeFront));
	
	vec3 gaze = glm::eulerAngles(util::rotationBetween(m_frontDir, glm::normalize(m_gaze * m_frontDir)));
	m_pitch = gaze.x;
	m_yaw = gaze.y;
	// float roll	= gaze.z;
 

	if(!polarToPanTilt()) { // this sets the dmx-data
	}

	m_cameraPersp.lookAt(m_lookAt);
	publishChanges("lookAt", util::valueToJson(m_lookAt));
}

void act::room::MovingHeadRoomNode::setPan(float pan)
{
	panTo(pan);
	//panTiltToYawPitch();
}

void act::room::MovingHeadRoomNode::setTilt(float tilt)
{
	tiltTo(tilt);
	//panTiltToYawPitch();
}

void act::room::MovingHeadRoomNode::setPanTilt(float pan, float tilt)
{
	panTo(pan);
	tiltTo(tilt);

	//panTiltToYawPitch();
}

void act::room::MovingHeadRoomNode::setDimmer(float dim, bool publish)
{
	m_dimmer.setValue(dim);
	setValue("dimmer", (m_dimmer.getValue() * m_dimmerMul.getValue()) * 255);
}

void act::room::MovingHeadRoomNode::setSpeed(float speed)
{
	m_speed.setValue(speed);
	setValue("speed", m_speed.getValue() * 255);
}

void act::room::MovingHeadRoomNode::setZoom(float zoom, bool publish)
{
	if (!m_hasZoom)
		return; 
	m_zoom.setValue(zoom);
	m_beamAngle = m_beamAngleMin + (zoom * (m_beamAngleMax - m_beamAngleMin));
	m_cameraPersp.setFov(m_beamAngle);
	setValue("zoom", m_zoom.getValue() * 255);
}

void act::room::MovingHeadRoomNode::setUV(float uv, bool publish)
{
	if (!m_hasUV)
		return;
	m_UV.setValue(uv);
	setValue("uv", m_UV.getValue() * 255);
}

void act::room::MovingHeadRoomNode::setStrobe(float strobe, bool publish)
{
	if (!m_hasStrobe)
		return;
	m_strobe.setValue(strobe);
	setValue("strobe", m_strobe.getValue() * 255);
}

void act::room::MovingHeadRoomNode::home()
{
	setDimmer(0.0f);
	setPanTilt(m_panCenterOffset, 90);
	setZoom(0.0f);
}

void act::room::MovingHeadRoomNode::createColorWheelLookUp(ci::Json colorMap)
{
	//if (!colorMap)
	//	return;

	std::map<int, int> hueToVal;
	m_hasWhiteColorWheel = false;
	m_whiteColorWheelValue = -1;

	for (auto color : colorMap["colors"]) {
		if (color.find("value") == color.end())
			continue;
		int value = color["value"];

		ci::Color rgb = ci::Color(color["R"], color["G"], color["B"]);
		vec3 hsv = ci::rgbToHsv(rgb);

		if (hsv.y <= 0.05f) {
			m_hasWhiteColorWheel = true;
			m_whiteColorWheelValue = value;
			continue;
		}

		hueToVal[hsv.x * 255] = value;
	}
	int mapHue = 0;
	for (auto entry = hueToVal.begin(); entry != hueToVal.end(); ++entry) {
		auto current	= *entry;
		auto nextEntry	= ++entry;
		if(nextEntry == hueToVal.end())
			nextEntry	= hueToVal.begin();
		auto next		= *nextEntry;

		int currentHue	= current.first;
		int nextHue		= next.first;
		int currentVal  = current.second;
		int nextVal		= next.second;
		
		if (nextHue < currentHue) { // assume that next is the first entry again
			nextHue += 255;
		}

		int halfHue = (currentHue + nextHue) * 0.5f;

		for (mapHue; mapHue < halfHue && mapHue <= 255; mapHue++) {
			m_colorWheelLookUp[mapHue] = currentVal;
		}
		for (mapHue; mapHue < nextHue && mapHue <= 255; mapHue++) {
			m_colorWheelLookUp[mapHue] = nextVal;
		}

		if (entry == hueToVal.end())
			break;
	}
}

bool act::room::MovingHeadRoomNode::polarToPanTilt()
{
	float tilt	= toDegrees(m_theta)	+ 90	;	// shift to middle position and add offset
	float pan	= toDegrees(m_phi)		+ 180	+ m_panCenterOffset;	// shift to middle position and add offset

	if (pan > (m_panCenterOffset + 360.0f))
		pan -= (m_panRange - m_panCenterOffset);

	if (tilt >= 360.0f - m_tiltCenterOffset)
		tilt = m_theta - (360.0f - m_tiltCenterOffset);


	setPanTilt(pan, tilt);

	if (pan >= 0.0f && tilt >= 0.0f && pan <= m_panRange && tilt <= m_tiltRange)
		return true;
	else {
		//panTiltToYawPitch();
		return false;
	}
}

void act::room::MovingHeadRoomNode::panTiltToPolar()
{
	m_theta		= toRadians(m_tilt.getValue() - 90);
	m_phi		= toRadians(m_pan.getValue()  - 180);

	m_yaw		= m_phi;
	m_pitch		= glm::two_pi<float>() - (m_theta);
}

void act::room::MovingHeadRoomNode::panTo(float pan)
{
	m_pan.setValue(fmodf(pan, (float)m_panRange));
	
	double panRough = (((double)m_pan.getValue() - (m_panCenterOffset + 0.5)) / (double)m_panRange);

	if (m_isPanFlipped) {
		panRough = 1.0f - panRough;
	}
	panRough *= 255;
	setValue("pan", (int)floor(panRough));

	if (m_hasFineAdjust) {
		double panFine = (panRough - floor(panRough)) * 255.0;
		setValue("finePan", (int)floor(panFine));
	}
}

void act::room::MovingHeadRoomNode::tiltTo(float tilt)
{
	m_tilt.setValue(clamp(tilt, 0.0f, (float)(m_tiltRange)));

	double tiltRough = ((double)(m_tilt.getValue() - m_tiltOffset) / (double)m_tiltRange);
	
	if (m_isTiltFlipped) {
		tiltRough = 1.0f - tiltRough;
	}

	tiltRough *= 255;
	setValue("tilt", (int)floor(tiltRough));

	if (m_hasFineAdjust) {
		double tiltFine = (tiltRough - floor(tiltRough)) * 255.0;
		setValue("fineTilt", (int)floor(tiltFine));
	}
}

void act::room::MovingHeadRoomNode::onPosition(act::UID replyUID, bool publish)
{
	m_cameraPersp.setEyePoint(m_position);

	if (publish)
		publishChanges("position", util::valueToJson(m_position), net::PT_ROOMNODE_UPDATE, replyUID);
}

/*highlights the MH 
	(Dimmer: 100%,
	Color: white)
	*/
void act::room::MovingHeadRoomNode::highlight() {
	
	m_colorPreHighlight = m_color;
	m_dimmerPreHighlight = m_dimmer.getValue();
	this->setColor(ci::Color::white());
	this->setDimmer(100.0);
}

/*releases the highlight
-> all changed values are set to the values pre-highlight*/
void act::room::MovingHeadRoomNode::release() {
	this->setColor(m_colorPreHighlight);
	this->setDimmer(m_dimmerPreHighlight.getValue());
}