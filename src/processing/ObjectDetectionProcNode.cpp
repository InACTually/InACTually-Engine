
/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
    Copyright (c) 2025 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2021, 2026

    contributors:
    Lars Engeln - mail@lars-engeln.de
    Fabian Töpfer
*/

#include "procpch.hpp"
#include "ObjectDetectionProcNode.hpp"


act::proc::ObjectDetectionProcNode::ObjectDetectionProcNode() : ProcNodeBase("ObjectDetection") {

	m_displayScale = 0.8f;
    m_minConfidence = 0.3f;
    m_nmsThreshold = 0.5f;
    m_objThreshold = 0.5f;

	m_textureHelper = proc::TextureHelper::create();

    m_show = false;

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });
	
	m_imagePort = createImageOutput("pass-through image");
    m_detectionImagePort = createImageOutput("detection image");
    m_featureListPort = createFeatureListOutput("feature list");

	initNetwork();

    m_currentObjects.resize(0);
}


act::proc::ObjectDetectionProcNode::~ObjectDetectionProcNode() {
}


void act::proc::ObjectDetectionProcNode::initNetwork() {

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
    m_inputSize = cv::Size(640, 640);

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
}

void act::proc::ObjectDetectionProcNode::update() {
}

void act::proc::ObjectDetectionProcNode::draw() {
    beginNodeDraw();
	
    ImGui::Checkbox("show", &m_show);
    

	if (m_show && m_textureHelper->hasTexture()) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));

        glm::vec2 texSize = ci::Rectf(glm::vec2(0, 0), ci::fromOcv(m_textureHelper->getTexture().size())).getCenteredFit(ci::Rectf(glm::ivec2(0, 0), m_drawSize), true).getSize();
        ImGui::Image(m_textureHelper->getTexture().texId(), texSize);

		ci::gl::pushMatrices();
	}
    ImGui::SetNextItemWidth(m_drawSize.x);
    preventDrag(ImGui::SliderFloat("min confidence", &m_minConfidence, 0.01f, 1.0f));

    for (auto r : m_currentObjects) {
        ImGui::TextUnformatted(r.first.c_str());
        ImGui::SameLine();
        ImGui::Text("%f", r.second);
    }

    endNodeDraw();
}

void act::proc::ObjectDetectionProcNode::generateAnchors()
{
    std::vector< std::tuple<int, int, int> > nb;
    int total = 0;

    for (auto v : m_strides)
    {
        int w = m_inputSize.width / v;
        int h = m_inputSize.height / v;
        nb.push_back(std::tuple<int, int, int>(w * h, w, v));
        total += w * h;
    }
    m_grids = cv::Mat(total, 2, CV_32FC1);
    m_expandedStrides = cv::Mat(total, 1, CV_32FC1);
    float* ptrGrids = m_grids.ptr<float>(0);
    float* ptrStrides = m_expandedStrides.ptr<float>(0);
    int pos = 0;
    for (auto le : nb)
    {
        int r = get<1>(le);
        for (int i = 0; i < get<0>(le); i++, pos++)
        {
            *ptrGrids++ = float(i % r);
            *ptrGrids++ = float(i / r);
            *ptrStrides++ = float((get<2>(le)));
        }
    }
}

void act::proc::ObjectDetectionProcNode::onMat(cv::UMat event) {
	m_imagePort->send(event);
	
    double ratio = std::min(m_inputSize.height / double(event.rows), m_inputSize.width / double(event.cols));


    cv::UMat blob = preprocess(event, m_inputSize, ratio);

    cv::Mat predictions = detect(blob);

    processDetection(event.clone(), predictions, ratio);
}

cv::UMat act::proc::ObjectDetectionProcNode::preprocess(cv::UMat frame, cv::Size targetSize, float ratio)
{
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

cv::Mat act::proc::ObjectDetectionProcNode::detect(cv::UMat blob)
{
    m_network.setInput(blob);
    std::vector<cv::Mat> outputs;
    m_network.forward(outputs, m_network.getUnconnectedOutLayersNames());

    cv::Mat predictions = postprocess(outputs[0]);
    return predictions;
}

cv::Mat act::proc::ObjectDetectionProcNode::postprocess(cv::Mat outputs)
{
    cv::Mat dets = outputs.reshape(0, outputs.size[1]);
    cv::Mat col01;
    cv::add(dets.colRange(0, 2), m_grids, col01);
    cv::Mat col23;
    cv::exp(dets.colRange(2, 4), col23);
    std::vector<cv::Mat> col = { col01, col23 };
    cv::Mat boxes;
    cv::hconcat(col, boxes);
    float* ptr = m_expandedStrides.ptr<float>(0);
    for (int r = 0; r < boxes.rows; r++, ptr++)
    {
        boxes.rowRange(r, r + 1) = *ptr * boxes.rowRange(r, r + 1);
    }
    // get boxes
    cv::Mat boxes_xywh(boxes.rows, boxes.cols, CV_32FC1, cv::Scalar(1));
    cv::Mat scores = dets.colRange(5, dets.cols).clone();
    std::vector<float> maxScores(dets.rows);
    std::vector<int> maxScoreIdx(dets.rows);
    std::vector<cv::Rect2d> boxesXYWH(dets.rows);
    for (int r = 0; r < boxes_xywh.rows; r++, ptr++)
    {
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
    for (auto idx : keep)
    {
        boxes_xywh.rowRange(idx, idx + 1).copyTo(candidates(cv::Rect(0, row, 4, 1)));
        candidates.at<float>(row, 4) = maxScores[idx];
        candidates.at<float>(row, 5) = float(maxScoreIdx[idx]);
        row++;
    }
    if (keep.size() == 0)
        return cv::Mat();
    return candidates;
}


std::vector<cv::Rect>  act::proc::ObjectDetectionProcNode::processDetection(cv::UMat frame, const cv::Mat& outs, float scaleFactor)
{
    std::vector<int> classIDs;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int row = 0; row < outs.rows; row++)
    {
        cv::Mat boxF = outs(cv::Rect(0, row, 4, 1));// / scaleFactor;
        cv::Mat box;
        boxF.convertTo(box, CV_32S);

        float score = outs.at<float>(row, 4);
        if(score < m_minConfidence)
			continue;

        int classId = int(outs.at<float>(row, 5));
        cv::Point classIDPoint;

        int x0 = box.at<int>(0, 0);
        int y0 = box.at<int>(0, 1);
        int x1 = box.at<int>(0, 2);
        int y1 = box.at<int>(0, 3);

        classIDs.push_back(classId);
        confidences.push_back((float)score);
        boxes.push_back(cv::Rect(x0, y0, x1 + x0, y1 + y0));
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, m_minConfidence, m_nmsThreshold, indices);

    std::vector<int> msgLabel;
    std::vector<float> msgConf;
    std::vector<cv::Rect> msgBoxes;
    m_currentObjects.resize(0);
	
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        cv::Rect box = boxes[idx];

        if (!m_classes.empty() && idx < (int)m_classes.size())
        {
            m_currentObjects.push_back(std::make_pair(m_classes[classIDs[idx]], confidences[idx]));
    	
	        drawBox(frame, box, m_classes[classIDs[idx]], confidences[idx]);

	        msgLabel.push_back(classIDs[idx]);
	        msgConf.push_back(confidences[idx]);
	        msgBoxes.push_back(boxes[idx]);
        }
    }

	m_textureHelper->toTextureAsync(frame);
	m_detectionImagePort->send(frame);

    return boxes;
}

void act::proc::ObjectDetectionProcNode::drawBox(cv::UMat& frame, cv::Rect box, std::string className, float confidence)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame, box.tl(), box.br(), ci::toOcv(util::Design::primaryColor()), 3);

    //Get the label for the class name and its confidence
    std::string label = cv::format("%.2f", confidence);
    label = className + ":" + label;

    //Display the label at the top of the bounding box
    int baseLine;
    cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    box.y = cv::max(box.y, labelSize.height);
    rectangle(frame, cv::Point(box.x, box.y - round(1.5 * labelSize.height)), cv::Point(box.x + round(1.5 * labelSize.width), box.y + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
    putText(frame, label, cv::Point(box.x, box.y), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0), 1);
}


