
/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
    Copyright (c) 2025 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2021

    contributors:
    Lars Engeln - mail@lars-engeln.de
    Fabian Töpfer
*/

#include "procpch.hpp"
#include "ObjectDetectionProcNode.hpp"


act::proc::ObjectDetectionProcNode::ObjectDetectionProcNode() : ProcNodeBase("ObjectDetection") {

	m_displayScale = 0.8f;
    m_isProcessing = false;
    m_minConfidence = 0.3f;

    m_show = false;

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });
	
	m_imagePort = createImageOutput("pass-through image");
    m_detectionImagePort = createImageOutput("detection image");
    m_featureListPort = createFeatureListOutput("feature list");

	initNetwork();

    m_currentObjects.resize(0);
}


act::proc::ObjectDetectionProcNode::~ObjectDetectionProcNode() {
    if (m_isProcessing)
        m_thread.join();
}


void act::proc::ObjectDetectionProcNode::initNetwork() {

	// get labels of all classes
	std::string classesFile = ci::app::getAssetPath("your-model-here").string();

	std::ifstream ifs(classesFile);
	std::string line;
	while (getline(ifs, line)) m_classes.push_back(line);


	std::string cfgFile = ci::app::getAssetPath("your-model-here").string();
	std::string weightsFile = ci::app::getAssetPath("your-model-here").string();

    if (cfgFile.empty() || weightsFile.empty()) {
        CI_LOG_E("files not avaiable.");
        return;
    }
    try {
        m_network = cv::dnn::readNetFromDarknet(cfgFile, weightsFile);
    }
    catch (cv::Exception exc) {
        CI_LOG_EXCEPTION("ObjectDetection", exc);
    }

	//add this for cuda support
	// 
	m_network.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	m_network.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

	if (m_network.empty()) {
		std::ostringstream ss;
		ss << "Failed to load network with the following settings:\n";
		throw std::invalid_argument(ss.str());
	}

	std::vector<std::string> layers = m_network.getLayerNames();
	auto i = m_network.getUnconnectedOutLayers();
	m_outputLayer = layers[i[0] - 1];

}

void act::proc::ObjectDetectionProcNode::update() {

	if(m_isProcessingDone) {
        m_thread.join();
        m_isProcessingDone = false;
  
        cv::UMat out;
        cv::flip(m_processedFrame, out, 1);

        if (m_show) {
            m_texture = gl::Texture2d::create(fromOcv(out));
		}
        m_detectionImagePort->send(out);

        m_featureListPort->send(m_currentObjects);
		
        m_isProcessing = false;
	}
}

void act::proc::ObjectDetectionProcNode::draw() {
    beginNodeDraw();
	
    ImGui::Checkbox("show", &m_show);
    

	if (m_show && m_texture) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));

        ci::vec2 texSize = Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(ivec2(0, 0), m_drawSize), true).getSize();
		ImGui::Image(m_texture, texSize, vec2(1, 1), vec2(0, 0));

		gl::pushMatrices();
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

void act::proc::ObjectDetectionProcNode::onMat(cv::UMat event) {
	m_imagePort->send(event);

    if (m_isProcessing)
        return;
	
    cv::UMat frame;
    event.copyTo(frame);
    event.copyTo(m_processedFrame);

    

    m_thread = std::thread([&](cv::UMat mat) {

        cv::Mat input_blob = cv::dnn::blobFromImage(mat, 1 / 255.0, cv::Size(416, 416), false, false);
        m_network.setInput(input_blob, "data");

        m_detection.clear();
        m_network.forward(m_detection, getOutputsNames(m_network));

        std::vector<cv::Rect> boxes;
        boxes = processDetection(m_processedFrame, m_detection);
    	
        m_isProcessingDone = true;
    }, frame);
    m_isProcessing = true;
}

std::vector<std::string> act::proc::ObjectDetectionProcNode::getOutputsNames(const cv::dnn::Net& net)
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


std::vector<cv::Rect>  act::proc::ObjectDetectionProcNode::processDetection(cv::UMat& frame, const std::vector<cv::Mat>& outs)
{
    std::vector<int> classIDs;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIDPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIDPoint);

            if (confidence > m_minConfidence)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIDs.push_back(classIDPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

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
    	
	        drawBox(m_classes[classIDs[idx]], confidences[idx], box.x, box.y,
	            box.x + box.width, box.y + box.height, frame);

	        msgLabel.push_back(classIDs[idx]);
	        msgConf.push_back(confidences[idx]);
	        msgBoxes.push_back(boxes[idx]);
        }
    }

    return boxes;
}

void act::proc::ObjectDetectionProcNode::drawBox(std::string className, float conf, int left, int top, int right, int bottom, cv::UMat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), toOcv(util::Design::primaryColor()), 3);

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


