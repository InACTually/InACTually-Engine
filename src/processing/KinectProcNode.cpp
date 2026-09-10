
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021â€“2025 Lars Engeln, Fabian TÃ¶pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "KinectProcNode.hpp"

act::proc::KinectProcNode::KinectProcNode() : ProcNodeBase("Kinect", NT_INPUT)
{
	m_drawSize = glm::ivec2(336, 189);

	m_kinectImageInPort = createImageInput("RGB", [&](cv::UMat image) {
		if (image.rows == 0)
			return;

		if (m_isRecording || m_isPreparedForRecording)
			onRGBImage(image);

		m_kinectImageOutPort->send(image);
	}, false);

	m_kinectDepthInPort = createImageInput("Depth", [&](cv::UMat image) {
		if (image.rows == 0)
			return;

		m_kinectDepthRawOutPort->send(image);

		cv::UMat depth;
		if (image.type() != CV_8UC1 && !m_kinect->isDummy()) // :/ have to yield dummyness
			image.convertTo(depth, CV_8UC1, 1.0f / 16.0f); //  
		else {
			image.copyTo(depth);
		}
		m_kinectDepthOutPort->send(depth);

		if (m_isRecording || m_isPreparedForRecording)
			onDepthImage(depth);

		/*

		const unsigned char noDepth = 0;
		cv::UMat mask = cv::UMat(depth.rows, depth.cols, CV_8UC1);

		cv::threshold(depth, mask, 1, 255, cv::THRESH_BINARY); // THRESH_BINARY_INV for inpaint
		cv::UMat median;
		cv::Mat scalar = cv::Mat(depth.rows, depth.cols, CV_8UC1); scalar.setTo(cv::mean(depth));
		cv::UMat filled = scalar.getUMat(cv::ACCESS_FAST);
		//cv::inpaint(depth, mask, filled, 2.0, cv::INPAINT_TELEA); // way to expensive

		//depth.copyTo(filled, mask);
		//cv::resize(filled, filled, cv::Size(depth.cols * 0.05f, depth.rows * 0.05f)); // pseudo adaptive-mean approach :D cheap blurring
		//cv::resize(filled, filled, cv::Size(depth.cols, depth.rows));

		depth.copyTo(filled, mask);
		cv::resize(filled, median, cv::Size(depth.cols * 0.25f, depth.rows * 0.25f));
		cv::medianBlur(median, median, 15);
		cv::resize(median, filled, cv::Size(depth.cols, depth.rows));

		depth.copyTo(filled, mask);

		m_kinectDepthFilledOutPort->send(filled);

		cv::UMat visual;
		image.convertTo(depth, CV_8UC1, 1.0f / 12.0f);
		cv::applyColorMap(filled, visual, cv::COLORMAP_VIRIDIS);
		m_kinectDepthVisualizedOutPort->send(visual);
		*/
		}, false);

	m_kinectInfraRedInPort = createImageInput("IR", [&](cv::UMat image) {
		if (image.rows == 0)
			return;

		if (m_isRecording || m_isPreparedForRecording)
			onIRImage(image);

		m_kinectInfraRedOutPort->send(image);
	}, false);

	m_kinectBIMInPort = createImageInput("BodyIndexMap", [&](cv::UMat image) {
		if (image.rows == 0)
			return;

		if (m_isRecording || m_isPreparedForRecording)
			onBodyIndexMap(image);

		cv::UMat visual = image.mul(32);
		cv::UMat mask;
		cv::threshold(image, mask, 128, 255, cv::THRESH_BINARY);

		//TODO: reformat recorded depth video format for dummy kinect

		//cv::applyColorMap(visual, visual, m_kinect->getColorMap());

		cv::Mat scalar = cv::Mat(visual.rows, visual.cols, CV_8UC3);
		scalar.setTo(255);
		cv::UMat fill = scalar.getUMat(cv::ACCESS_FAST);
		fill.copyTo(visual, mask);

		m_kinectVisualizedBIMOutPort->send(visual);

		m_kinectBIMOutPort->send(image);
	}, false);

	m_kinectBodiesInPort = InputPort<room::BodyRefList>::create(PT_BODYLIST, "Bodies", [&](room::BodyRefList bodies) {
		//if (bodies.empty())
		//	return;

		if (m_isRecording || m_isPreparedForRecording)
			onBodies(bodies);
		m_kinectBodiesOutPort->send(bodies);
	});

	m_kinectImageOutPort = createImageOutput("RGB");
	m_kinectDepthRawOutPort = createImageOutput("raw Depth");
	m_kinectDepthOutPort = createImageOutput("Depth");
	m_kinectDepthFilledOutPort = createImageOutput("filled Depth");
	m_kinectDepthVisualizedOutPort = createImageOutput("visualized Depth");
	m_kinectInfraRedOutPort = createImageOutput("IR");
	m_kinectBIMOutPort = createImageOutput("BodyIndexMap");
	m_kinectVisualizedBIMOutPort = createImageOutput("visualized BodyIndexMap");
	m_kinectFOVPort = createVec2Output("FOV");

	m_kinectBodiesOutPort = OutputPort<room::BodyRefList>::create(PT_BODYLIST, "Bodies");
	m_outputPorts.push_back(m_kinectBodiesOutPort);

	m_kinectImageOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingImage(true); });
	m_kinectImageOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingImage(false); });

	m_kinectDepthOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(true); });
	m_kinectDepthOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(false); });

	m_kinectDepthRawOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(true); });
	m_kinectDepthRawOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(false); });

	m_kinectDepthFilledOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(true); });
	m_kinectDepthFilledOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(false); });

	m_kinectDepthVisualizedOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(true); });
	m_kinectDepthVisualizedOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingDepth(false); });

	m_kinectInfraRedOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingIR(true); });
	m_kinectInfraRedOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingIR(false); });

	m_kinectBIMOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(true); });
	m_kinectBIMOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(false); });

	m_kinectVisualizedBIMOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(true); });
	m_kinectVisualizedBIMOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(false); });

	m_kinectBodiesOutPort->setConnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(true); });
	m_kinectBodiesOutPort->setDisconnectionCB([&]() {if (m_kinect) m_kinect->setIsCapturingBodies(false); });

	m_kinectFOVPort->setConnectionCB([&]() {  if (m_kinect) m_kinectFOVPort->send(m_kinect->getFOV()); });
}

act::proc::KinectProcNode::~KinectProcNode()
{
}

void act::proc::KinectProcNode::setup(act::room::RoomManagersRef roomMgrs)
{
	m_kinectMgr = roomMgrs->kinectMgr;

	if (m_kinectMgr->getKinectNames().size() > 0) {
		m_selectedKinect = 0;
		activate();
	}
}

void act::proc::KinectProcNode::update()
{
	if (m_isSaveDialog) {
		m_isSaveDialog = false;
		m_isPreparedForRecording = true;
		std::vector<std::string> exts;
		exts.push_back("json");
		//m_path = ci::app::getSaveFilePath(ci::app::getAssetPath("./../recordings/"), exts).string();
		m_path = ci::app::getAssetPath("./").string();
		if (m_path != "") {
			int namePos = m_path.rfind("\\");
			std::string filename = "kinect_capture";//m_path.substr(namePos + 1, m_path.size() - (namePos + 1));
			m_path = m_path + "\\" + filename;
			ci::writeJson(ci::fs::path(m_path + ".json"), ci::Json::object()); // mkdir, touch json
			prepareRecording();
		}
	}
	if (m_isStartingRecording) {
		m_isStartingRecording = false;
		startRecording();
	}

	if (m_kinect)
		m_kinect->setIsDrawing2dJoints(m_isShowingJoints);
}

void act::proc::KinectProcNode::draw()
{
	if (m_active)
		beginNodeDraw(NDS_ACTIVE);
	else
		beginNodeDraw(NDS_ERROR);

	act::room::kinectConnectionState connState = m_kinectMgr->getDevicesAndStates()[m_selectedKinectName];

	if (m_kinectMgr->getKinectNames().empty()) {
		ImGui::TextColored(util::Design::errorColor(), "No Kinect devices found. Please set up a Kinect device in KinectManager.");
	}
	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Kinect").x);

	if (!m_isDisconnected) {
		if (ImGui::Combo("Kinect", &m_selectedKinect, m_kinectMgr->getKinectNames()))
		{
			activate();
		}

		if (!m_active)
		{
			ImGui::TextColored(util::Design::errorColor(), "Select a Kinect device to enable it");
		}
	}
	else {
		ImGui::TextColored(util::Design::errorColor(), "Kinect device is currently disconnected.\nConnect Kinect device and restart");
	}

	ImGui::NewLine();

	if (m_active && m_kinect) {

		if (m_kinect->isDummy()) {
			ImGui::Text("Recording");
			if(m_kinect->getCurrentState() == act::room::KinectState::KS_OPENED){
				if (ImGui::Button("play recording")) {
					m_kinect->startDevice(true);
				}
			}
			else if (m_kinect->getCurrentState() == act::room::KinectState::KS_CAPTURING) {
				if (ImGui::Button("stop recording")) {
					m_kinect->stopDevice();
				}
			}
		}


		if (!m_isRecording) {
			if (!m_isPreparedForRecording) {
				if (ImGui::Button("prepare recording")) {
					m_isSaveDialog = true;
				}
			}
			else {
				if (ImGui::Button("start recording")) {
					m_isStartingRecording = true;
				}
			}
		}
		else {
			if (ImGui::Button("stop recording")) {
				stopRecording();
			}
		}

		ImGui::Checkbox("show joints", &m_isShowingJoints);

		float temp = m_kinect->getTemperatureSample();
		glm::vec3 acc = m_kinect->getAccelerometerSample();
		glm::vec3 gyro = m_kinect->getGyroscopeSample();

		ImGui::Text("Temperature:    %.2f", temp);
		ImGui::Text("Accelerometer:  (%.2f, %.2f, %.2f)", acc.x, acc.y, acc.z);
		ImGui::Text("Gyroscope:      (%.2f, %.2f, %.2f)", gyro.x, gyro.y, gyro.z);


	}
	endNodeDraw();
}

void act::proc::KinectProcNode::onRGBImage(cv::UMat image)
{
	m_lastRGBImage = image;
	if (m_isPreparedForRecording) {
		m_rgbRec->getVideoInPort()->recieve(image);
	}
}

void act::proc::KinectProcNode::onDepthImage(cv::UMat image)
{
	m_lastDepthImage = image;
	if (m_isPreparedForRecording) {
		m_depthRec->getVideoInPort()->recieve(image);
	}
	if (m_isRecording) {
		recordCurrentFrames();
	}
}

void act::proc::KinectProcNode::onIRImage(cv::UMat image)
{
	m_lastIRImage = image;
	//if (m_isPreparedForRecording) {
	//	m_irRec->getVideoInPort()->recieve(image);
	//}
}

void act::proc::KinectProcNode::onBodyIndexMap(cv::UMat image)
{
	m_lastBIM = image;
	if (m_isPreparedForRecording) {
		m_bimRec->getVideoInPort()->recieve(image);
	}
}

void act::proc::KinectProcNode::onBodies(room::BodyRefList& bodies)
{
	m_lastBodies = ci::Json::array();
/*	int bodyIndex = 0;
	for (auto&& body : bodies) {
		auto bodyJson = ci::Json::object();
		bodyJson["id"]	= body.first;
		bodyJson["index"] = bodyIndex;
		bodyIndex++;

		auto skeletonJson = ci::Json::array();

		int jointIndex = 0;
		auto skeleton = body.second;
		for (auto&& joint : skeleton.joints) {
			auto jointJson = ci::Json::object();

			jointJson["index"] = jointIndex;
			jointIndex++;
			jointJson["confidence_level"] = joint.confidence_level;

			auto posJson = ci::Json::object();
			posJson["x"]	= joint.position.xyz.x;
			posJson["y"]	= joint.position.xyz.y;
			posJson["z"]	= joint.position.xyz.z;
			jointJson["position"] = posJson;

			auto orientJson = ci::Json::object();
			orientJson["w"]	= joint.orientation.wxyz.w;
			orientJson["x"]	= joint.orientation.wxyz.x;
			orientJson["y"]	= joint.orientation.wxyz.y;
			orientJson["z"]	= joint.orientation.wxyz.z;
			jointJson["orientation"] = orientJson;

			skeletonJson.push_back(jointJson);
		}
		bodyJson["skeleton"] = skeletonJson;
		m_lastBodies.push_back(bodyJson);
	}
	*/
}

void act::proc::KinectProcNode::prepareRecording()
{
	m_kinect->setIsCapturingImage(true);
	m_kinect->setIsCapturingDepth(true);
	//m_kinect->setIsCapturingIR(true);
	m_kinect->setIsCapturingBodies(true);

	m_lastBodies = ci::Json::array();
	m_bodyRec = ci::Json::array();
	m_rgbRec = std::make_shared<VideoRecorderProcNode>();
	m_depthRec = std::make_shared<VideoRecorderProcNode>();
	//m_irRec		= make_shared<VideoRecorderNode>();
	m_bimRec = std::make_shared<VideoRecorderProcNode>();

	m_isPreparedForRecording = true;
}

void act::proc::KinectProcNode::startRecording()
{
	m_rgbRec->saveVideo(m_path + "_rgb.mp4", 30);
	m_depthRec->saveVideo(m_path + "_depth.mp4", 30, false);
	//m_irRec->saveVideo(m_path + "_ir.mp4", 30);
	m_bimRec->saveVideo(m_path + "_bim.mp4", 30, false);

	m_isPreparedForRecording = false;

	m_rgbRec->record();
	m_depthRec->record();
	//m_irRec->record();
	m_bimRec->record();

	m_isRecording = true;
}

void act::proc::KinectProcNode::stopRecording()
{
	m_isRecording = false;
	m_isPreparedForRecording = false;

	m_rgbRec->stop();
	m_depthRec->stop();
	//m_irRec->stop();
	m_bimRec->stop();

	m_rgbRec = nullptr;
	m_depthRec = nullptr;
	//m_irRec		= nullptr;
	m_bimRec = nullptr;

	ci::Json json = ci::Json::object();
	std::stringstream strstr;
	glm::vec3 pos = m_kinectMgr->getKinectPositionByUID(m_kinect->getNodeUID());
	glm::vec3 rotation = m_kinectMgr->getKinectOrientationByUID(m_kinect->getNodeUID());

	auto posJson = ci::Json::object();
	posJson["x"] = pos.x;
	posJson["y"] = pos.y;
	posJson["z"] = pos.z;
	json["position"] = posJson;

	auto rotJson = ci::Json::object();
	rotJson["x"] = rotation.x;
	rotJson["y"] = rotation.y;
	rotJson["z"] = rotation.z;
	json["rotation"] = rotJson;

	json["bodyFrames"] = m_bodyRec;
	m_bodyRec = ci::Json::array();

	ci::writeJson(ci::fs::path(m_path + ".json"), json);
}

void act::proc::KinectProcNode::recordCurrentFrames()
{
	m_rgbRec->getVideoInPort()->recieve(m_lastRGBImage);
	m_depthRec->getVideoInPort()->recieve(m_lastDepthImage);
	//m_irRec->getVideoInPort()->recieve(m_lastIRImage);
	m_bimRec->getVideoInPort()->recieve(m_lastBIM);

	m_bodyRec.push_back(m_lastBodies);
}

void act::proc::KinectProcNode::activate()
{
	if (m_selectedKinect >= m_kinectMgr->getKinectNames().size())
		return;

	m_selectedKinectName = m_kinectMgr->getKinectNames()[m_selectedKinect];

	m_kinect = m_kinectMgr->getDeviceByName(m_selectedKinectName);


	{
		auto&& port = m_kinectMgr->getKinectPort(m_selectedKinectName, act::room::kinectImagePort::KP_IMAGE);
		if (port) {
			port->connect(m_kinectImageInPort);
		}
	}
	{
		auto&& port = m_kinectMgr->getKinectPort(m_selectedKinectName, act::room::kinectImagePort::KP_DEPTH);
		if (port) {
			port->connect(m_kinectDepthInPort);
		}
	}
	{
		auto&& port = m_kinectMgr->getKinectPort(m_selectedKinectName, act::room::kinectImagePort::KP_INFRARED);
		if (port) {
			port->connect(m_kinectInfraRedInPort);
		}
	}
	{
		auto&& port = m_kinectMgr->getKinectPort(m_selectedKinectName, act::room::kinectImagePort::KP_BIM);
		if (port) {
			port->connect(m_kinectBIMInPort);
		}
	}
	{
		auto&& port = m_kinectMgr->getBodyPortByName(m_selectedKinectName);
		if (port) {
			port->connect(m_kinectBodiesInPort);
		}
	}


	if (m_kinectImageOutPort->getListenerCount() > 0)		m_kinect->setIsCapturingImage(true); 
	if (m_kinectDepthOutPort->getListenerCount() > 0)		m_kinect->setIsCapturingDepth(true); 
	if (m_kinectDepthRawOutPort->getListenerCount() > 0)	m_kinect->setIsCapturingDepth(true); 
	if (m_kinectDepthFilledOutPort->getListenerCount() > 0) m_kinect->setIsCapturingDepth(true); 
	if (m_kinectDepthVisualizedOutPort->getListenerCount() > 0) m_kinect->setIsCapturingDepth(true); 
	if (m_kinectInfraRedOutPort->getListenerCount() > 0)	m_kinect->setIsCapturingIR(true);
	if (m_kinectBIMOutPort->getListenerCount() > 0)			m_kinect->setIsCapturingBodies(true);
	if (m_kinectVisualizedBIMOutPort->getListenerCount() > 0) m_kinect->setIsCapturingBodies(true); 
	if (m_kinectBodiesOutPort->getListenerCount() > 0)		m_kinect->setIsCapturingBodies(true); 

	m_kinectFOVPort->send(m_kinect->getFOV());

	m_active = true;
}

ci::Json act::proc::KinectProcNode::toParams()
{
	ci::Json json = ci::Json::object();
	json["selectedDevice"]	= m_selectedKinectName;
	json["showJoints"]		= m_isShowingJoints;
	return json;
}

void act::proc::KinectProcNode::fromParams(ci::Json json)
{
	if (util::setValueFromJson(json, "selectedDevice", m_selectedKinectName)) {

		act::room::kinectConnectionState connState = m_kinectMgr->getDevicesAndStates()[m_selectedKinectName];

		if (connState != act::room::kinectConnectionState::K4_DISCONNECTED) {

			m_selectedKinect = 0;
			for (auto&& name : m_kinectMgr->getKinectNames()) {
				if (name == m_selectedKinectName)
					break;
				m_selectedKinect++;
			}

			activate();

			util::setValueFromJson(json, "showJoints", m_isShowingJoints);
		}
		else
		{
			m_isDisconnected = true;
		}
	}
}