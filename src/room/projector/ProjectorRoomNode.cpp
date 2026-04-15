
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
	Anton Hörig - dev@antonhoerig.de
*/

#include "roompch.hpp"
#include "projector/ProjectorRoomNode.hpp"
#include "WindowData.hpp"
#include "CallbackDrawable.hpp"


act::room::ProjectorRoomNode::ProjectorRoomNode(std::string name, ci::vec3 position, ci::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase("projector", position, rotation, radius, replyUID)
{	
	setResolution(ci::ivec2(1920, 1080));
	setFocalLengthPixel(ci::vec2(800, 800));
	setSkew(0);
	setPrincipalPoint(ci::vec2(0,0));
	setIsCalibrating(false);

	updateCameraPersp();
	calculateProjectionMatrix();

	auto colorShader = ci::gl::getStockShader(ci::gl::ShaderDef().color());
	m_wirePlane = ci::gl::Batch::create(ci::geom::WirePlane().size(ci::vec2(20)).subdivisions(ci::ivec2(100)), colorShader);

	m_imageInputPort = proc::InputPort<proc::image>::create(proc::PT_IMAGE, "image", [this](proc::image img) {
		m_currentImageTex = ci::gl::Texture2d::create(fromOcv(img));
	});
}

act::room::ProjectorRoomNode::~ProjectorRoomNode()
{
	if (m_window) // if the window is still open, close it
		m_window->close();
}

void act::room::ProjectorRoomNode::setup()
{
}

void act::room::ProjectorRoomNode::update()
{
}

void act::room::ProjectorRoomNode::draw()
{
	gl::ScopedColor color;
	enableStatusColor(); 

	gl::pushMatrices();
	gl::translate(m_position);
	gl::rotate(m_rotation);
	gl::drawSphere(ci::Sphere(ci::vec3(0.0f), 0.05f));
	gl::drawCube(ci::vec3(0.1f, 0.0f, 0.15f), ci::vec3(0.4f, 0.15f, 0.3f));

	util::drawCoords();

	if (getIsUnfolded())
		gl::color(util::Design::highlightColor(0.85f));
	else
		gl::color(ColorA(1.0f, 1.0f, 1.0f, 0.6f));
	gl::scale(vec3(0.1, 0.1, 0.1));
	gl::drawFrustum(m_cameraPersp);
	gl::popMatrices();
}

void act::room::ProjectorRoomNode::drawSpecificSettings()
{
	//Parameters
	if (ImGui::CollapsingHeader("Parameters")) {
		if (ImGui::DragInt2("Resolution", &m_resolution, 1.0, 0.0, 10000, "%i"))
		{
			setResolution(m_resolution);
		}

		if (ImGui::DragFloat2("Focal Length", &m_focalLenghtPixel))
		{
			setFocalLengthPixel(m_focalLenghtPixel);
		}

		if (ImGui::DragFloat("Skew", &m_skew))
		{
			setSkew(m_skew);
		}

		if (ImGui::DragFloat2("Principle Point", &m_principalPoint))
		{
			setPrincipalPoint(m_principalPoint);
		}
	}

	//Window
	if (ImGui::CollapsingHeader("Window")) {
		if(ImGui::Button("Open Projector Window"))
		{
			createWindowOnDisplay();
		}

		if (ImGui::InputInt("Display", &m_DisplayNumber)) {
			createWindowOnDisplay(true);
		}
	}

	//Rendering
	if (ImGui::CollapsingHeader("Drawing")) {
		//ImGui::Checkbox("Use cameraPersp", &m_useCameraPersp);
		ImGui::Checkbox("Show Debug Grid", &m_showDebugGrid);
		ImGui::Checkbox("Show Debug Grid CV", &m_showDebugGridCV);
		ImGui::Checkbox("Show Window Borders", &m_showWindowBorders);
	}

	//Calibration
	if (ImGui::CollapsingHeader("Calibration")) {
		if (ImGui::InputInt("Total Points", &m_totalPoints)) {
			if (m_totalPoints < 6) m_totalPoints = 6;
		}
		if (ImGui::InputInt("Total Rays", &m_totalCalibrationRays)) {
			if (m_totalCalibrationRays < 4) m_totalCalibrationRays = 4;
			if (m_totalCalibrationRays > 10) m_totalCalibrationRays = 10;
		}
		if (ImGui::Checkbox("Calibrate with HMD", &m_isCalibrating))
		{
			setIsCalibrating(m_isCalibrating);
		}
		ImGui::Text("Calibration refinement is only possible in the session the calibration process was started!");
		if (ImGui::Button("Reset Correspondences"))
		{
			resetCorrespondences();
		}

		ImGui::Separator();

		if (ImGui::Button("Calibrate with Test Pairs"))
		{
			calibrateDLT(true);
		}
	}

	if (ImGui::CollapsingHeader("Evaluate Calibration")) {

		ImGui::Checkbox("Serialize Errors", &m_serializeErrors);
		ImGui::Separator();
		ImGui::Text("Calibration Error Metrics:");
		ImGui::LabelText("DLT Total Error", "%f", m_totalError);
		ImGui::LabelText("DLT Total Square Error", "%f", m_totalSpuareError);
		ImGui::LabelText("DLT Min Error", "%f", m_minError);
		ImGui::LabelText("DLT Max Error", "%f", m_maxError);

		ImGui::Separator();
		ImGui::LabelText("DLT Mean Error", "%f", m_meanError);
		ImGui::LabelText("DLT GL Mean Error", "%f", m_glMeanError);
		ImGui::LabelText("DLT RMS Error", "%f", m_rmsError);

		ImGui::Separator();
		ImGui::LabelText("Ground Truth Total Error", "%f", m_trueTotalError);
		ImGui::LabelText("Ground Truth Square Error", "%f", m_trueTotalSpuareError);
		ImGui::LabelText("Ground Truth Min Error", "%f", m_trueMinError);
		ImGui::LabelText("Ground Truth Max Error", "%f", m_trueMaxError);

		ImGui::Separator();
		ImGui::Checkbox("Show Evaluation Dot Pattern", &m_showDotPattern);
		if (ImGui::Button("Restart Dot Measurement"))
		{
			m_evaluateDots = true;
			m_currentDot = 0;

			m_trueMaxError = 0.0f;
			m_trueMinError = std::numeric_limits<double>::max();
			m_trueTotalError = 0.0f;
			m_trueTotalSpuareError = 0.0f;
		}
	}
}

ci::Json act::room::ProjectorRoomNode::toParams()
{
	ci::Json json = ci::Json::object();

	//Parameters
	json["resolution"] = util::valueToJson(getResolution());
	json["focalLengthPixel"] = util::valueToJson(getFocalLengthPixel());
	json["skew"] = getSkew();
	json["principalPoint"] = util::valueToJson(getPrincipalPoint());

	//Calibration
	json["isCalibrating"] = getIsCalibrating();

	// Error metrics
	if (m_serializeErrors)
	{
		json["meanError"] = m_meanError;
		json["glMeanError"] = m_glMeanError;
		json["rmsError"] = m_rmsError;

		json["totalError"] = m_totalError;
		json["totalSpuareError"] = m_totalSpuareError;
		json["minError"] = m_minError;
		json["maxError"] = m_maxError;

		json["trueTotalError"] = m_trueTotalError;
		json["trueTotalSpuareError"] = m_trueTotalSpuareError;
		json["trueMinError"] = m_trueMinError;
		json["trueMaxError"] = m_trueMaxError;
	}

	return json;
}

void act::room::ProjectorRoomNode::fromParams(ci::Json json)
{
	ci::ivec2 resolution = ci::ivec2(0);
	if (util::setValueFromJson(json, "resolution", resolution)) {
		setResolution(resolution, false);
	}

	ci::vec2 focalLengthPixel = ci::vec2(0.0f);
	if (util::setValueFromJson(json, "focalLengthPixel", focalLengthPixel)) {
		setFocalLengthPixel(focalLengthPixel, false);
	}

	float skew = 0;
	if (util::setValueFromJson(json, "skew", skew)) {
		setSkew(skew, false);
	}

	ci::vec2 principalPoint = ci::vec2(0.0f);
	if (util::setValueFromJson(json, "principalPoint", principalPoint)) {
		setPrincipalPoint(principalPoint, false);
	}

	//communication with frontend (not serialized)
	bool isCalibrating;
	if (util::setValueFromJson(json, "isCalibrating", isCalibrating)) {
		setIsCalibrating(isCalibrating, false);
	}

	bool tmp;
	if (util::setValueFromJson(json, "resetCorrespondences",  tmp)) {
		resetCorrespondences();
	}

	//correspondence from frontend (not serialized)
	ci::vec3 objectPoint;
	if (util::setValueFromJson(json, "objectPoint", objectPoint)) {
		addCorrespondence(cv::Point3f(objectPoint.x, objectPoint.y, objectPoint.z));
	}

	//Error metrics
	float totalError = 0.0f;
	if (util::setValueFromJson(json, "totalError", totalError)) {
		m_totalError = totalError;
	}

	float minError = 0.0f;
	if (util::setValueFromJson(json, "minError", minError)) {
		m_minError = minError;
	}

	float maxError = 0.0f;
	if (util::setValueFromJson(json, "maxError", maxError)) {
		m_maxError = maxError;
	}

	float meanError = 0.0f;
	if (util::setValueFromJson(json, "meanError", meanError)) {
		m_meanError = meanError;
	}

	float glMeanError = 0.0f;
	if (util::setValueFromJson(json, "glMeanError", glMeanError)) {
		m_glMeanError = glMeanError;
	}

	float rmsError = 0.0f;
	if (util::setValueFromJson(json, "rmsError", rmsError)) {
		m_rmsError = rmsError;
	}

	float trueTotalError = 0.0f;
	if (util::setValueFromJson(json, "trueTotalError", trueTotalError)) {
		m_trueTotalError = trueTotalError;
	}

	float trueTotalSpuareError = 0.0f;
	if (util::setValueFromJson(json, "trueTotalSpuareError", trueTotalSpuareError)) {
		m_trueTotalSpuareError = trueTotalSpuareError;
	}

	float trueMinError = 0.0f;
	if (util::setValueFromJson(json, "trueMinError", trueMinError)) {
		m_trueMinError = trueMinError;
	}

	float trueMaxError = 0.0f;
	if (util::setValueFromJson(json, "trueMaxError", trueMaxError)) {
		m_trueMaxError = trueMaxError;
	}
}

void act::room::ProjectorRoomNode::setResolution(ci::ivec2 resolution, bool publish)
{
	m_resolution = resolution;
	if (publish)
	{
		publishParam("resolution", util::valueToJson(m_resolution));
	}
}

void act::room::ProjectorRoomNode::setIsCalibrating(bool isCalibrating, bool publish)
{
	m_isCalibrating = isCalibrating;
	if (publish)
	{
		publishParam("isCalibrating", m_isCalibrating);
	}
}

void act::room::ProjectorRoomNode::setFocalLengthPixel(ci::vec2 focalLengthPixel, bool publish, bool updateCam)
{
	m_focalLenghtPixel = focalLengthPixel;
	if (publish)
	{
		publishParam("focalLengthPixel", util::valueToJson(m_focalLenghtPixel));
	}
	if (updateCam)
	{
		updateCameraPersp();
		calculateProjectionMatrix();
	}
}

void act::room::ProjectorRoomNode::setSkew(float skew, bool publish, bool updateCam)
{
	m_skew = skew;
	if (publish)
	{
		publishParam("skew", m_skew);
	}
	if (updateCam)
	{
		updateCameraPersp();
		calculateProjectionMatrix();
	}
}

void act::room::ProjectorRoomNode::setPrincipalPoint(ci::vec2 principalPoint, bool publish, bool updateCam)
{
	m_principalPoint = principalPoint;
	if (publish)
	{
		publishParam("principalPoint", util::valueToJson(m_principalPoint));
	}
	if (updateCam)
	{
		updateCameraPersp();
		calculateProjectionMatrix();
	}
}

bool act::room::ProjectorRoomNode::createWindow(bool onlyRecreate)
{
	if (m_window)
		m_window->close();
	else if (onlyRecreate)
		return false;
	
	m_window = WindowData::createWindow(getName(), ivec2(m_resolution.x, m_resolution.y));
	CallbackDrawableRef drawable = CallbackDrawable::create();
	drawable->setDrawCallback(std::bind(&ProjectorRoomNode::drawProjection, this));
	drawable->setUpdateCallback(std::bind(&ProjectorRoomNode::updateProjection, this));
	m_window->getUserData<WindowData>()->setDrawable(drawable);
	m_window->getSignalClose().connect([this]() {
		m_window = nullptr; // reset the window pointer when closed
	});

	m_window->getSignalMouseDown().connect([this](ci::app::MouseEvent evt) {
		// onMouse()
		if (!m_evaluateDots)
			return;

		auto mousePos = evt.getPos();

		auto dotPos = getDotFromIndex(m_currentDot);
		auto error = ci::distance(vec2(mousePos), dotPos);

		m_trueTotalError += error;
		m_trueTotalSpuareError += error * error;

		if (error < m_trueMinError)
			m_trueMinError = error;
		if (error > m_trueMaxError)
			m_trueMaxError = error;

		m_currentDot++;
		if (m_currentDot >= m_totalDots)
		{
			m_evaluateDots = false;
		}
	});

	return true;
}

bool act::room::ProjectorRoomNode::createWindowOnDisplay(bool onlyRecreate)
{
	if (!createWindow(onlyRecreate)) // create new window otherwise context will be wrong
		return false;

	FullScreenOptions fullScreenOptions;
	if (m_DisplayNumber >= 0 && m_DisplayNumber < Display::getDisplays().size()) {
		fullScreenOptions.display(Display::getDisplays()[m_DisplayNumber]);
		m_window->setFullScreen(true, fullScreenOptions);
	}
	else {
		m_window->setFullScreen(false);
	}

	return true;
}

void act::room::ProjectorRoomNode::updateProjection()
{
}

void act::room::ProjectorRoomNode::drawProjection()
{	
	gl::clear(ci::Color::black());
	if (m_isCalibrating)
	{
		if (m_nextCorrespondence < m_totalPoints)
		{
			gl::ScopedMatrices();
			gl::setMatricesWindow(getWindowSize());

			int currentRay = getCurrentRay();
			gl::translate(m_calibrationRayCoords[currentRay].x * m_resolution.x, m_calibrationRayCoords[currentRay].y * m_resolution.y);
			drawCalibrationPoint();
		}
		else
		{
			setIsCalibrating(false);
		}
	}
	else if (m_showDotPattern)
	{
		drawDotPattern();
	}
	else
	{
		if (m_currentImageTex) {
			gl::draw(m_currentImageTex);
		}

		//normal rendering
		if (m_showDebugGrid) {
			gl::ScopedMatrices mat();
			ci::gl::color(1.0f, 1, 1);

			if (m_useCameraPersp)
				gl::setMatrices(m_cameraPersp);
			else
				gl::setProjectionMatrix(m_glProjectionMatrix);

			calculateViewMatrix();
			gl::setViewMatrix(m_glViewMatrix);

			gl::ScopedLineWidth lineWidth(3.0f);
			m_wirePlane->draw();

			gl::ScopedLineWidth lineWidth2(5.0f);
			gl::color(ci::Color(1.0f, 0.5f, 0.5f)); // red for X axis
			gl::drawLine(ci::vec3(0.0f, 0.0f, 0.0f), ci::vec3(0.5f, 0.0f, 0.0f));
			gl::color(ci::Color(0.5f, 0.9f, 1.0f)); // Blue for Z axis
			gl::drawLine(ci::vec3(0.0f, 0.0f, 0.0f), ci::vec3(0.0f, 0.0f, 0.5f));
		}

		if(m_showDebugGridCV )
		{
			drawDotGroundGrid();
		}
	}

	if (m_showWindowBorders)
	{
		gl::ScopedMatrices();
		gl::setMatricesWindow(getWindowSize());
		gl::color(ci::Color::white());

		gl::ScopedLineWidth lineWidth(5.0f);
		// Top edge
		gl::drawLine(ci::vec2(0, 0), ci::vec2(m_resolution.x, 0));
		// Right edge
		gl::drawLine(ci::vec2(m_resolution.x, 0), ci::vec2(m_resolution.x, m_resolution.y));
		// Bottom edge
		gl::drawLine(ci::vec2(m_resolution.x, m_resolution.y), ci::vec2(0, m_resolution.y));
		// Left edge
		gl::drawLine(ci::vec2(0, m_resolution.y), ci::vec2(0, 0));
	}

}

void act::room::ProjectorRoomNode::drawCalibrationPoint()
{
	gl::color(ci::Color::white());
	float radius = 2;
	gl::drawSolidCircle(vec2(0), radius);

	int numCircles = 25;

	gl::ScopedLineWidth lineWidth(2.0f);
	for (int i = 2; i <= numCircles; i++)
	{
		gl::color(ci::Color::gray(1.0f - float(i) / float(numCircles)));
		gl::drawStrokedCircle(vec2(), 10 * i * i /2);

	}


}

void act::room::ProjectorRoomNode::drawDotPattern()
{
	gl::ScopedMatrices mat();
	gl::setMatricesWindow(getWindowSize());

	float radius = 2;

	//5 points in 2 rows
	for (int i = 0; i < m_totalDots; i++)
	{
		gl::ScopedColor color(1, 1, 1);

		auto pos = getDotFromIndex(i);

		if (i == m_currentDot)
		{
			gl::color(1, 0, 0);
		}

		gl::drawSolidCircle(pos, radius);
	}
}



void act::room::ProjectorRoomNode::drawDotGroundGrid()
{
	gl::ScopedMatrices mat();
	gl::setMatricesWindow(getWindowSize());
	gl::ScopedColor color(1, 1, 1);

	float spacing = 0.20f; //in meters
	float radius = 5;

	for (int x = -50; x <= 50; x++)
	{
		for (int z = -50; z <= 50; z++)
		{
			cv::Mat X = (cv::Mat_<double>(4, 1) << x * spacing, 0, z * spacing, 1.0); //dot in 3D

			//project 
			cv::Mat x = m_P * X;

			//Unhomogenize reprojectefc point
			double u = x.at<double>(0, 0) / x.at<double>(2, 0);
			double v = x.at<double>(1, 0) / x.at<double>(2, 0);

			gl::drawSolidCircle(vec2(u, v), radius);
		}
	}

	cv::Mat X = (cv::Mat_<double>(4, 1) << 0, 0, 0, 1.0);

	//reproject 
	cv::Mat x = m_P * X;

	//Unhomogenize reprojectefc point
	double u = x.at<double>(0, 0) / x.at<double>(2, 0);
	double v = x.at<double>(1, 0) / x.at<double>(2, 0);

	gl::color(1, 0, 0);
	gl::drawSolidCircle(vec2(u, v), radius);
}

ci::vec2 act::room::ProjectorRoomNode::getDotFromIndex(int i)
{
	float spacing = 0.20f; //in meters

	int row = 0;

	if (i >= 6)
		row = 1;
	if (i >= 12)
		row = 2;
	if (i >= 18)
	{
		row = 3;
	}

	int collumn = i % 6;

	cv::Mat X = (cv::Mat_<double>(4, 1) << 0, (row + 1) * spacing, (collumn + 1) * spacing, 1.0); //dot in 3D

	//project 
	cv::Mat x = m_P * X;

	//Unhomogenize reprojectefc point
	double u = x.at<double>(0, 0) / x.at<double>(2, 0);
	double v = x.at<double>(1, 0) / x.at<double>(2, 0);

	return ci::vec2(u, v);
}



void act::room::ProjectorRoomNode::updateCameraPersp()
{
	float f = (m_focalLenghtPixel.x + m_focalLenghtPixel.y) / 2;
	float fov = 2 * atan(m_resolution.x / (2 * f)) * 180.0 / CV_PI;
	//float fovY = 2 * atan(m_resolution.y / (2 * m_focalLenghtPixel.y)) * 180.0 / CV_PI;
	m_cameraPersp = ci::CameraPersp(m_resolution.x, m_resolution.y, fov, 0.1f, 30.0f);

	//lens shift 1 -> shifte half the viewport size to the right
	float shiftX = -(m_principalPoint.x / m_resolution.x * 2.0f - 1.0f);
	float shiftY = (m_principalPoint.y / m_resolution.y * 2.0f - 1.0f); //needs to be negated (twice) since y up
	m_cameraPersp.setLensShift(shiftX, shiftY);

	m_cameraPersp.setEyePoint(vec3(0.0f));
	m_cameraPersp.lookAt(vec3(0.0f, 0.0f, -1.0f)); //along negative z (follows from calibration coordinate system convertion)
}

void act::room::ProjectorRoomNode::calculateViewMatrix()
{
	//view
	glm::mat4 rotationMatrix = glm::toMat4(m_orientation); // Convert quaternion to rotation matrix
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position); // Translate to position

	glm::mat4 cameraMatrix = translationMatrix * rotationMatrix;

	m_glViewMatrix = glm::inverse(cameraMatrix);
}

void act::room::ProjectorRoomNode::calculateProjectionMatrix()
{
	//projection
	float nearZ = 0.1f, farZ = 1000.0f;

	float left = -(m_principalPoint.x) * nearZ / m_focalLenghtPixel.x;
	float right = (m_resolution.x - m_principalPoint.x) * nearZ / m_focalLenghtPixel.x;
	float bottom = -(m_resolution.y - m_principalPoint.y) * nearZ / m_focalLenghtPixel.y;
	float top = (m_principalPoint.y) * nearZ / m_focalLenghtPixel.y;

	m_glProjectionMatrix = glm::frustum(left, right, bottom, top, nearZ, farZ);
	m_glProjectionMatrix[1][0] = -2.0f * m_skew / m_resolution.x;
}

void act::room::ProjectorRoomNode::getTestPairs(std::vector<cv::Point3f>& objectPoints, std::vector<cv::Point2f>& imagePoints)
{
	//points from calibration
	/*
	objectPoints = {
	{-0.969012201,  0.104362249,  1.51150644},
	{-0.353259593,  0.495392919,  1.27609563},
	{ 0.859388232,  0.113188505,  0.946744561},
	{ 1.33750129,   0.713171959,  0.730579376},
	{-1.16010177,   0.161237001, -1.73243642},
	{-0.515584528,  0.551542282, -1.41896713},
	{ 0.896920264,  0.096350193, -0.897632599},
	{ 1.17355955,   0.457464814, -0.755033493}
	};

	imagePoints = {
	{192.000000, 108.000000},
	{192.000000, 108.000000},
	{192.000000, 972.000000},
	{192.000000, 972.000000},
	{1728.000000, 108.000000},
	{1728.000000, 108.000000},
	{1728.000000, 972.000000},
	{1728.000000, 972.000000}
	};
	//*/

	// synthetic test pairs
	///*
	objectPoints = {
		{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
		{-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
	};

	double fx = 800.0;   
	double fy = 800.0;
	double cx = 1920.0 / 2.0;   
	double cy = 240.0;
	double skew = 100;  

	//camera rotation
	double pitch = -45.0 * CV_PI / 180.0; // rotation around X
	double yaw = 45.0 * CV_PI / 180.0; // rotation around Y
	double roll = 0.0 * CV_PI / 180.0; // rotation around Z


	cv::Mat Rx = (cv::Mat_<double>(3, 3) <<
		1, 0, 0,
		0, cos(pitch), -sin(pitch),
		0, sin(pitch), cos(pitch));

	cv::Mat Ry = (cv::Mat_<double>(3, 3) <<
		cos(yaw), 0, sin(yaw),
		0, 1, 0,
		-sin(yaw), 0, cos(yaw));

	cv::Mat Rz = (cv::Mat_<double>(3, 3) <<
		cos(roll), -sin(roll), 0,
		sin(roll), cos(roll), 0,
		0, 0, 1);

	//combine in order: yaw pitch roll 
	cv::Mat R = Ry * Rx * Rz;
	R = R.t(); //to inverse world to cam rotation
	
	//camera center in world space
	cv::Mat t = (cv::Mat_<double>(3, 1) << 5, 5, 5);
	t = R * -t; //to world to cam translation

	//calculate correspondences
	for (auto& p : objectPoints) {
		cv::Mat Pw = (cv::Mat_<double>(3, 1) << p.x, p.y, p.z);
		cv::Mat Pc = R * Pw + t;

		double Xc = Pc.at<double>(0);
		double Yc = Pc.at<double>(1);
		double Zc = -Pc.at<double>(2); // negative z

		double u = fx * Xc / Zc - skew * Yc / Zc + cx; // negative skew cause y flipped
		double v = fy * Yc / Zc + m_resolution.y - cy;
		v = m_resolution.y - v; //flip y

		imagePoints.push_back(cv::Point2f(u, v));
	}
	//*/
}

void act::room::ProjectorRoomNode::addCorrespondence(cv::Point3f objectPoint, bool calibrateIfPossible)
{
	if (!m_isCalibrating || m_nextCorrespondence >= m_totalPoints) {
		return;
	}

	int currentRay = getCurrentRay();

	cv::Point2f imagePoint(m_calibrationRayCoords[currentRay].x * m_resolution.x, m_calibrationRayCoords[currentRay].y * m_resolution.y);
	m_imagePoints.push_back(imagePoint);
	m_objectPoints.push_back(objectPoint);

	//update next image Point
	m_nextCorrespondence++;
	
	// issue calibration
	if (calibrateIfPossible && m_nextCorrespondence >= 6)
	{
		calibrateDLT();
	}
}

int act::room::ProjectorRoomNode::getCurrentRay()
{

	//prefer next ray
	return m_nextCorrespondence % m_totalCalibrationRays;

	/*
	//prefer same ray
	int pointsPerRay = m_totalPoints / m_totalCalibrationRays;
	int rest = m_totalPoints % m_totalCalibrationRays;

	std::vector<int> rayOrder;
	rayOrder.reserve(m_totalPoints);

	for (int ray = 0; ray < m_totalCalibrationRays; ++ray) {
		int count = pointsPerRay + (ray < rest ? 1 : 0); //ditribute rest
		for (int i = 0; i < count; ++i) {
			rayOrder.push_back(ray);
		}
	}
	return rayOrder[m_nextCorrespondence]; 
	*/

}

void act::room::ProjectorRoomNode::resetCorrespondences()
{
	m_imagePoints.clear();
	m_objectPoints.clear();
	m_nextCorrespondence = 0;
}

void act::room::ProjectorRoomNode::calibrateDLT(const bool useTestPairs)
{
	std::vector<cv::Point3f> objectPoints;
	std::vector<cv::Point2f> imagePoints;
	
	if (useTestPairs)
	{
		getTestPairs(objectPoints, imagePoints);
	}
	else
	{
		assert(m_objectPoints.size() == m_imagePoints.size() && m_imagePoints.size() >= 6);
		objectPoints = m_objectPoints;
		imagePoints = m_imagePoints;
	}	

	cv::Mat P = dltSolveP(objectPoints, imagePoints);

	m_P = P;

	cv::Mat K, R, t;

	cv::decomposeProjectionMatrix(P, K, R, t);

	//set intrinsics
	K /= K.at<double>(2, 2); //normalize
	setFocalLengthPixel(ci::vec2(K.at<double>(0, 0), K.at<double>(1, 1)), true, false);
	setSkew(K.at<double>(0, 1), true, false);
	setPrincipalPoint(ci::vec2(K.at<double>(0, 2), K.at<double>(1, 2)), true, false);

	//set extrinsics
	cv::Mat convertToGL = (cv::Mat_<double>(3, 3) <<
		1, 0, 0,
		0, -1, 0,
		0, 0, -1); // convert to openGL

	R = convertToGL * R; // rotation in World Coordinates optionally convertToCV with right side duplication

	ci::mat3 ciR(
		(float)R.at<double>(0, 0), (float)R.at<double>(0, 1), (float)R.at<double>(0, 2),
		(float)R.at<double>(1, 0), (float)R.at<double>(1, 1), (float)R.at<double>(1, 2),
		(float)R.at<double>(2, 0), (float)R.at<double>(2, 1), (float)R.at<double>(2, 2)
	);
	setOrientation(glm::toQuat(ciR));

	t = t.rowRange(0, 3) / t.at<double>(3); //unhomogenize and cut W
	//t = R * t; //transform to position in world coordinates... not necesarry is already in world coordinates
	setPosition(ci::vec3(t.at<double>(0), t.at<double>(1), t.at<double>(2)));

	//calculate Matrices
	updateCameraPersp();
	calculateViewMatrix();
	calculateProjectionMatrix();

	calculateErrors(m_P, objectPoints, imagePoints);
}

cv::Mat act::room::ProjectorRoomNode::dltSolveP(const std::vector<cv::Point3f> objectPoints, const std::vector<cv::Point2f> imagePoints)
{
	//construc matrix of DLT equation
	cv::Mat A = dltCreateMat(objectPoints, imagePoints);

	//compute SVD
	cv::Mat w, u, vt;
	cv::SVD::compute(A, w, u, vt);

	//use smalles singular value (last row of vt as collum)
	cv::Mat pVec = vt.row(vt.rows - 1);


	//transform Vec to Mat (3 Rows)
	cv::Mat P = pVec.reshape(0, 3);

	//normalize for stability
	if (P.at<double>(2, 3) != 0.0)
		P = P / P.at<double>(2, 3);
	else
		pVec /= cv::norm(pVec) * 12;

	
	return P;
}


//create matrix A of DLT equation
cv::Mat act::room::ProjectorRoomNode::dltCreateMat(const std::vector<cv::Point3f> objectPoints, const std::vector<cv::Point2f> imagePoints)
{	
	//check if object = image points !!!
	
	//create Matrix
	int	nPairs = (int)objectPoints.size();
	cv::Mat A(2 * nPairs, 12, CV_64F, cv::Scalar(0));

	//asign corespondences according to sheme
	for (int i = 0; i < nPairs; i++)
	{
		//current opject pos
		double X = objectPoints[i].x;
		double Y = objectPoints[i].y;
		double Z = objectPoints[i].z;

		//current imagePos
		double u = imagePoints[i].x;
		double v = imagePoints[i].y;

		//set row 2i of matrix
		A.at<double>(2 * i, 0) = X;
		A.at<double>(2 * i, 1) = Y;
		A.at<double>(2 * i, 2) = Z;
		A.at<double>(2 * i, 3) = 1.0;

		A.at<double>(2 * i, 8) = -u*X;
		A.at<double>(2 * i, 9) = -u*Y;
		A.at<double>(2 * i, 10) = -u*Z;
		A.at<double>(2 * i, 11) = -u;

		//set row 2i + 1 of matrix
		A.at<double>(2 * i + 1, 4) = X;
		A.at<double>(2 * i + 1, 5) = Y;
		A.at<double>(2 * i + 1, 6) = Z;
		A.at<double>(2 * i + 1, 7) = 1.0;

		A.at<double>(2 * i + 1, 8) = -v * X;
		A.at<double>(2 * i + 1, 9) = -v * Y;
		A.at<double>(2 * i + 1, 10) = -v * Z;
		A.at<double>(2 * i + 1, 11) = -v;
	}

	return A;
}

void act::room::ProjectorRoomNode::calculateErrors(const cv::Mat& P, const std::vector<cv::Point3f>& objectPoints, const std::vector<cv::Point2f>& imagePoints)
{
	int numPoints = objectPoints.size();

	double totalSquaredError = 0.0;
	double totalError = 0.0;
	double totalGLError = 0.0;

	double minError = std::numeric_limits<double>::max();
	double maxError = 0.0;

	for (int i = 0; i < numPoints; ++i)
	{
		const cv::Point3f& objPt = objectPoints[i];
		const cv::Point2f& imgPt = imagePoints[i];

		//Convert to homogeneous coordinates
		cv::Mat X = (cv::Mat_<double>(4, 1) << objPt.x, objPt.y, objPt.z, 1.0);

		//reproject 
		cv::Mat x = P * X;

		//Unhomogenize reprojectefc point
		double u = x.at<double>(0, 0) / x.at<double>(2, 0);
		double v = x.at<double>(1, 0) / x.at<double>(2, 0);

		//Compute error
		double dx = imgPt.x - u;
		double dy = imgPt.y - v;
		double error = std::sqrt(dx * dx + dy * dy); //distance

		if (error < minError)
			minError = error;
		if (error > maxError)
			maxError = error;

		totalError += error;
		totalSquaredError += error * error;

		//gL projection
		glm::vec4 clip = m_glProjectionMatrix * m_glViewMatrix * glm::vec4(objPt.x, objPt.y, objPt.z, 1.0f);
		
		glm::vec3 ndc = glm::vec3(clip) / clip.w;
		float uGL = (ndc.x + 1.0f) * 0.5f * m_resolution.x;
		float vGL = (1 - (ndc.y + 1.0f) * 0.5f) * m_resolution.y;

		double dxGL = imgPt.x - uGL;
		double dyGL = imgPt.y - vGL;
		double errorGL = dxGL * dxGL + dyGL * dyGL; //square distance

		totalGLError += std::sqrt(errorGL);
	}

	m_totalError = static_cast<float>(totalError);
	m_totalSpuareError = static_cast<float>(totalSquaredError);

	m_minError = static_cast<float>(minError);
	m_maxError = static_cast<float>(maxError);

	m_meanError = static_cast<float>(totalError / numPoints);
	m_rmsError = static_cast<float>(std::sqrt(totalSquaredError / numPoints));
	m_glMeanError = static_cast<float>(totalGLError / numPoints);

}
