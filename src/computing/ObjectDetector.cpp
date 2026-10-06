
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "ObjectDetector.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::comp::ObjectDetector::ObjectDetector() : DetectorBase("objectDetector") {
	initNetwork();
}

act::comp::ObjectDetector::ObjectDetector(room::CameraRoomNodeRef camera) : DetectorBase("objectDetector", camera)
{
	m_minConfidence = 0.3f;
	m_nmsThreshold = 0.5f;
	m_objThreshold = 0.5f;

	initNetwork();
}


act::comp::ObjectDetector::~ObjectDetector()
{

}


ci::Json act::comp::ObjectDetector::toJson() {
	auto json = ci::Json::object();


	return json;
}

void act::comp::ObjectDetector::fromJson(ci::Json json) {

}

void act::comp::ObjectDetector::refreshObjPoints()
{
	float objSize = 1.0f;
	m_objPoints = cv::Mat(4, 1, CV_32FC3);
	m_objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-objSize / 2.f, objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(objSize / 2.f, objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(objSize / 2.f, -objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-objSize / 2.f, -objSize / 2.f, 0);
}

void act::comp::ObjectDetector::generateAnchors() 
{
	std::vector< std::tuple<int, int, int> > nb;
	int total = 0;

	for (auto v : m_strides) {
		int w = m_blobSize.width / v;
		int h = m_blobSize.height / v;
		nb.push_back(std::tuple<int, int, int>(w * h, w, v));
		total += w * h;
	}
	m_grids = cv::Mat(total, 2, CV_32FC1);
	m_expandedStrides = cv::Mat(total, 1, CV_32FC1);
	float* ptrGrids = m_grids.ptr<float>(0);
	float* ptrStrides = m_expandedStrides.ptr<float>(0);
	int pos = 0;
	for (auto le : nb) {
		int r = get<1>(le);
		for (int i = 0; i < get<0>(le); i++, pos++) {
			*ptrGrids++ = float(i % r);
			*ptrGrids++ = float(i / r);
			*ptrStrides++ = float((get<2>(le)));
		}
	}
}

void act::comp::ObjectDetector::initNetwork()
{
	m_classes = std::vector<std::string>{
		"person",
		"bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
		"traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
		"bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
		"backpack", "umbrella", "handbag", "tie", "suitcase",
		"frisbee", "skis", "snowboard", "sports ball", "kite",
		"baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
		"bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
		"banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
		"chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv",
		"laptop", "mouse", "remote", "keyboard", "cell phone",
		"microwave", "oven", "toaster", "sink", "refrigerator",
		"book", "clock", "vase", "scissors", "teddy bear",
		"hair drier", "toothbrush"
	};

	m_strides = std::vector<int>{ 8, 16, 32 };
	m_blobSize = cv::Size(640, 640);

	//if (m_isUsingTiny)
	std::string path = ci::app::getAssetPath("3rd/models/object_detection_yolox/object_detection_yolox_2022nov.onnx").string();

	if (path.empty()) {
		CI_LOG_E("File not avaiable.");
		return;
	}
	try {
		m_network = cv::dnn::readNet(path);
	}
	catch (cv::Exception exc) {
		CI_LOG_EXCEPTION("ObjectDetection", exc);
	}

	m_network.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	m_network.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);

	generateAnchors();

	if (m_network.empty()) {
		std::ostringstream ss;
		ss << "Failed to load network with the following settings:\n";
		throw std::invalid_argument(ss.str());
	}

	std::vector<std::string> layers = m_network.getLayerNames();
	auto i = m_network.getUnconnectedOutLayers();
	m_outputLayer = layers[i[0] - 1];

	refreshObjPoints();

	// m_tracker = std::make_shared<byte_track::BYTETracker>(30, 50, 0.5f, 0.5f, 0.9f);

	m_isInitialized = true;
}

cv::UMat act::comp::ObjectDetector::preprocess(cv::UMat frame, cv::Size targetSize, float ratio) {
	cv::UMat resizeImg;
	cv::UMat paddedImg(targetSize.height, targetSize.width, CV_32FC3, cv::Scalar::all(114.0));

	cv::dnn::Image2BlobParams params;
	params.datalayout = cv::DNN_LAYOUT_NCHW;
	params.ddepth = CV_32F;
	params.mean = cv::Scalar::all(0);
	params.scalefactor = cv::Scalar::all(1);
	params.size = paddedImg.size();
	params.swapRB = true;

	cv::resize(frame, resizeImg, cv::Size(int(frame.cols * ratio), int(frame.rows * ratio)), cv::INTER_LINEAR);
	resizeImg.copyTo(paddedImg(cv::Rect(0, 0, int(frame.cols * ratio), int(frame.rows * ratio))));

	cv::UMat inputBlob;
	cv::dnn::blobFromImageWithParams(paddedImg, inputBlob, params);
	return inputBlob;
}

void act::comp::ObjectDetector::detect() {

	for (auto&& it = m_newObjectOccurence.begin(); it != m_newObjectOccurence.end();) {
		auto m = it->second;
		if (!m->wasInLastFrame) {
			it = m_newObjectOccurence.erase(it);
		}
		else {
			it++;
			m->wasInLastFrame = false;
		}
	}

	//save object that were processed by a camera to not do double work
	//std::vector<int> processedObjects;

	room::CameraRoomNodeRef cameraNode = m_camera;
	act::room::CameraDeviceRef camera = cameraNode->getCamera();
	cv::UMat image = m_currentImage;
	if (image.empty())
		return;

	double ratio = std::min(m_blobSize.height / double(image.rows), m_blobSize.width / double(image.cols));

	m_blob = preprocess(image, m_blobSize, ratio);

	try {
		m_network.setInput(m_blob); // , "data");

		bool hadDetections = m_detection.size() > 0;

		m_detection.clear();
		m_network.forward(m_detection, m_network.getUnconnectedOutLayersNames()); // getOutputsNames(m_network));

		if (hadDetections && m_detection.size() == 0) // skip if there is just a blind detection
			return; 

		cv::Mat predictions = postprocess(m_detection[0]);

		cv::UMat outputImage = image.clone();

		processDetection(outputImage, predictions, ratio);

		m_feedbackImage = outputImage;
	}
	catch (cv::Exception exc) {
		CI_LOG_E("Failed to detect Objects: " << exc.what());
	}
}

cv::Mat act::comp::ObjectDetector::postprocess(cv::Mat outputs) {
	cv::Mat dets = outputs.reshape(0, outputs.size[1]);
	cv::Mat col01;
	cv::add(dets.colRange(0, 2), m_grids, col01);
	cv::Mat col23;
	cv::exp(dets.colRange(2, 4), col23);
	std::vector<cv::Mat> col = { col01, col23 };
	cv::Mat boxes;
	cv::hconcat(col, boxes);
	float* ptr = m_expandedStrides.ptr<float>(0);
	for (int r = 0; r < boxes.rows; r++, ptr++) {
		boxes.rowRange(r, r + 1) = *ptr * boxes.rowRange(r, r + 1);
	}
	// get boxes
	cv::Mat boxes_xywh(boxes.rows, boxes.cols, CV_32FC1, cv::Scalar(1));
	cv::Mat scores = dets.colRange(5, dets.cols).clone();
	std::vector<float> maxScores(dets.rows);
	std::vector<int> maxScoreIdx(dets.rows);
	std::vector<cv::Rect2d> boxesXYWH(dets.rows);
	for (int r = 0; r < boxes_xywh.rows; r++, ptr++) {
		boxes_xywh.at<float>(r, 0) = boxes.at<float>(r, 0) - boxes.at<float>(r, 2) / 2.f;
		boxes_xywh.at<float>(r, 1) = boxes.at<float>(r, 1) - boxes.at<float>(r, 3) / 2.f;
		boxes_xywh.at<float>(r, 2) = boxes.at<float>(r, 2);
		boxes_xywh.at<float>(r, 3) = boxes.at<float>(r, 3);
		// get scores and class indices
		scores.rowRange(r, r + 1) = scores.rowRange(r, r + 1) * dets.at<float>(r, 4);
		double minVal, maxVal;
		cv::Point maxIdx;
		minMaxLoc(scores.rowRange(r, r + 1), &minVal, &maxVal, nullptr, &maxIdx);
		maxScoreIdx[r] = maxIdx.x;
		maxScores[r] = float(maxVal);
		boxesXYWH[r].x = boxes_xywh.at<float>(r, 0);
		boxesXYWH[r].y = boxes_xywh.at<float>(r, 1);
		boxesXYWH[r].width = boxes_xywh.at<float>(r, 2);
		boxesXYWH[r].height = boxes_xywh.at<float>(r, 3);
	}

	std::vector<int> keep;
	cv::dnn::NMSBoxesBatched(boxesXYWH, maxScores, maxScoreIdx, m_minConfidence, m_nmsThreshold, keep);
	cv::Mat candidates(int(keep.size()), 6, CV_32FC1);
	int row = 0;
	for (auto idx : keep) {
		boxes_xywh.rowRange(idx, idx + 1).copyTo(candidates(cv::Rect(0, row, 4, 1)));
		candidates.at<float>(row, 4) = maxScores[idx];
		candidates.at<float>(row, 5) = float(maxScoreIdx[idx]);
		row++;
	}
	if (keep.size() == 0)
		return cv::Mat();
	return candidates;
}

std::vector<std::string> act::comp::ObjectDetector::getOutputsNames(const cv::dnn::Net& net)
{
	if (m_names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		std::vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<std::string> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		m_names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			m_names[i] = layersNames[outLayers[i] - 1];
	}
	return m_names;
}

void act::comp::ObjectDetector::processDetection(cv::UMat& frame, const cv::Mat& outs, float ratio)
{
	std::vector<int> classIDs;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	// Scan through all the bounding boxes output from the network and keep only the
	// ones with high confidence scores. Assign the box's class label as the class
	// with the highest score for the box.
	
	//float* data = (float*)outs.data;
	for (int row = 0; row < outs.rows; ++row) //, data += outs.cols)
	{
		cv::Mat boxF = outs(cv::Rect(0, row, 4, 1));// / scaleFactor;
		cv::Mat box;
		boxF.convertTo(box, CV_32S);

		float score = outs.at<float>(row, 4);
		if (score < m_minConfidence)
			continue;

		int classId = int(outs.at<float>(row, 5));
		cv::Point classIDPoint;

		int x0 = box.at<int>(0, 0);
		int y0 = box.at<int>(0, 1);
		int x1 = box.at<int>(0, 2);
		int y1 = box.at<int>(0, 3);

		/*
		int centerX = (int)(data[0] * frame.cols);
		int centerY = (int)(data[1] * frame.rows);
		int width = (int)(data[2] * frame.cols);
		int height = (int)(data[3] * frame.rows);
		int left = centerX - width / 2;
		int top = centerY - height / 2;
		*/

		classIDs.push_back(classId);
		confidences.push_back((float)score);
		boxes.push_back(cv::Rect(x0, y0, x1 + x0, y1 + y0));
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

	//std::vector<int> msgLabel;
	//std::vector<float> msgConf;
	//std::vector<cv::Rect> msgBoxes;
	std::vector<ObjectCandidate> candidates;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];

		if (!m_classes.empty() && idx < (int)m_classes.size())
		{
			auto candidate = ObjectCandidate();
			candidate.id = idx;
			candidate.className = m_classes[classIDs[idx]];
			candidate.classID = classIDs[idx];
			candidate.confidence = confidences[idx];
			candidate.box = ci::Rectf(box.x, box.y, box.x + box.width, box.y + box.height);

			candidates.push_back(candidate);

			//drawBox(m_classes[classIDs[idx]], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame);

			//msgLabel.push_back(classIDs[idx]);
			//msgConf.push_back(confidences[idx]);
			//msgBoxes.push_back(boxes[idx]);
		}
	}

	if (candidates.size() == 0)
		return;

	trackObjects(candidates);


	for (auto&& candidate : m_currentCandidates) {
		std::stringstream strstr;
		strstr << candidate.id << " - " << candidate.className << "[" << candidate.classID << "]";
		drawBox(strstr.str(), candidate.confidence, candidate.box.x1, candidate.box.y1,
			candidate.box.x2, candidate.box.y2, frame);
	}
		
	m_areNewCandidatesAvailable = true;
}

void act::comp::ObjectDetector::drawBox(std::string className, float conf, int left, int top, int right, int bottom, cv::UMat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), ci::toOcv(util::Design::primaryColor()), 3);

	//Get the label for the class name and its confidence
	std::string label = cv::format("%.2f", conf);
	label = className + ":" + label;

	//Display the label at the top of the bounding box
	int baseLine;
	cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = cv::max(top, labelSize.height);
	rectangle(frame, cv::Point(left, top - round(1.5 * labelSize.height)), cv::Point(left + round(1.5 * labelSize.width), top + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
	putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0), 1);
}

void act::comp::ObjectDetector::checkOccurency(int id)
{
	bool isNew = true;
	for (auto&& occurence : m_newObjectOccurence) {
		auto&& m = occurence.second;
		if (m->id == id) {
			isNew = false;

			m->occurence++;
			m->wasInLastFrame = true;

			if (m->occurence > 10) {
				m->wasInLastFrame = false;
				m_validObjectIDs[m->id] = true;
			}

			break;
		}
	}
	if (isNew) {
		m_newObjectOccurence[id] = ObjectOccurence::create(id);
	}
}


void act::comp::ObjectDetector::trackObjects(std::vector<ObjectCandidate>& candidates)
{
	/*
	std::vector<byte_track::Object> objects;

	for (auto&& c : candidates) {
		auto rect = byte_track::Rect<float>(c.box.getX1(), c.box.getY1(), c.box.getWidth(), c.box.getHeight());
		byte_track::Object obj(rect, c.classID, c.confidence);
		objects.push_back(obj);
	}
	
	const auto results = m_tracker->update(objects);
	
	candidates.clear();
	for (auto&& r : results) {
		auto candidate = ObjectCandidate();
		candidate.id = r->getTrackId();
		candidate.confidence = r->getScore();
		auto rect = r->getRect();
		candidate.box = ci::Rectf(rect.tl_x(), rect.tl_y(), rect.br_x(), rect.br_y());
		candidate.classID = r->getLabel();
		candidate.className = m_classes[candidate.classID];
		candidates.push_back(candidate);
	}*/

	/*if (results.size() != candidates.size()) {
		m_currentCandidates.resize(0);
		return;
	}

	for (int i = 0; i < results.size() && i < candidates.size(); i++) {
		checkOccurency(results[i]->getTrackID());
		if (!m_validObjectIDs[results[i]->getTrackID()]) {
			candidates.erase(candidates.begin() + i);
			continue;
		}

		candidates[i].id = results[i]->getTrackID();
		auto r = results[i]->getRect();
		candidates[i].box = ci::Rectf(r.tl_x(), r.tl_y(), r.br_x(), r.br_y());
	}*/

	m_currentCandidates = candidates;
}
