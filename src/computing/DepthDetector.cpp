
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "DepthDetector.hpp"
#include "kinect/KinectDevice.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::comp::DepthDetector::DepthDetector() 
	: DetectorBase("DepthDetector")
	, m_env(nullptr)
	, m_session(nullptr)
	, m_inputTensor(nullptr)
	, m_outputTensor(nullptr)
{
	initNetwork();
}

act::comp::DepthDetector::DepthDetector(room::CameraRoomNodeRef camera) 
	: DetectorBase("DepthDetector", camera)
	, m_env(nullptr)
	, m_session(nullptr)
	, m_inputTensor(nullptr)
	, m_outputTensor(nullptr)
{
	initNetwork();
}


act::comp::DepthDetector::~DepthDetector()
{
	try {
		const auto& api = Ort::GetApi();
		// Finally, don't forget to release the provider options
		api.ReleaseCUDAProviderOptions(m_cudaOpts);
	}
	catch (Ort::Exception exc) {
		CI_LOG_E(exc.what());
	}
}


ci::Json act::comp::DepthDetector::toJson() {
	auto json = ci::Json::object();


	return json;
}

void act::comp::DepthDetector::fromJson(ci::Json json) {

}


void act::comp::DepthDetector::initNetwork()
{
	m_blobSize = cv::Size(m_width, m_height);
	m_inputImage.resize(m_width * m_height);

	std::wstring modelFile;

	m_isUsingTiny = false;

	if (m_isUsingTiny) {
		modelFile = ci::app::getAssetPath("3rd/models/depth-anything_v3/depth_anything_v2_vits_dynamic.onnx").wstring();
	}
	else {
		modelFile = ci::app::getAssetPath("3rd/models/depth-anything_v3/DA3METRIC-LARGE.onnx").wstring();
	}

	try {
		CI_LOG_V(Ort::GetVersionString());
		Ort::Env env(ORT_LOGGING_LEVEL_INFO, "test");

		m_env = Ort::Env();
		m_cudaOpts = nullptr;

		auto opts = Ort::SessionOptions(nullptr);
		
		try {
			const auto& api = Ort::GetApi();

			
			api.CreateCUDAProviderOptions(&m_cudaOpts);

			std::vector<const char*> keys{ "cudnn_conv_use_max_workspace" };
			std::vector<const char*> values{ "1" };

			api.UpdateCUDAProviderOptions(m_cudaOpts, keys.data(), values.data(), keys.size());

			//cudaStream_t cuda_stream;
			//cudaStreamCreate(&cuda_stream);
			// this implicitly sets "has_user_compute_stream"
			//api.UpdateCUDAProviderOptionsWithValue(cuda_options, "user_compute_stream", cuda_stream);
			//api.SessionOptionsAppendExecutionProvider_CUDA_V2(opts, m_cudaOpts);

			opts.AppendExecutionProvider_CUDA_V2(*m_cudaOpts);

			
		}
		catch (Ort::Exception exc) {
			CI_LOG_E(exc.what());
		}

		m_session = Ort::Session(m_env, modelFile.data(), opts);
		//m_env = std::make_shared<Ort::Env>(*env);


		{
			Ort::AllocatorWithDefaultOptions allocator;

			size_t num_output_nodes = m_session.GetOutputCount();
			for (size_t i = 0; i < num_output_nodes; ++i)
			{
				auto str = std::string(m_session.GetOutputNameAllocated(i, allocator).get());
				m_outputNameStrings.push_back(str);
				m_outputNames.push_back(str.c_str());
				CI_LOG_V("Output: " << str);
			}
		}
		{
			Ort::AllocatorWithDefaultOptions allocator;

			size_t num_input_nodes = m_session.GetInputCount();
			for (size_t i = 0; i < num_input_nodes; ++i)
			{
				auto str = std::string(m_session.GetInputNameAllocated(i, allocator).get());
				m_inputNameStrings.push_back(str);
				m_inputNames.push_back(str.c_str());
				CI_LOG_V("Input: " << str);
			}
		}


		m_inputImage.resize(m_width * m_height * 3);

		auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
		m_inputTensor = Ort::Value::CreateTensor<float>(memory_info, m_inputImage.data(), m_inputImage.size(), m_inputShape.data(), m_inputShape.size());
		m_outputTensor = Ort::Value::CreateTensor<float>(memory_info, m_results.data(), m_results.size(), m_outputShape.data(), m_outputShape.size());

		m_isInitialized = true;
	}
	catch (Ort::Exception exc) {
		CI_LOG_E(exc.what());
	}
}

void  act::comp::DepthDetector::detect() {
	cv::UMat image;
	m_currentImage.copyTo(m_colorImage); //getMat(cv::ACCESS_READ);
	if (m_colorImage.empty())
		return;

	cv::Mat inputBlob;

	try {
		cv::resize(m_colorImage, image, m_blobSize);
		cv::flip(image, image, 1);
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
		//image.convertTo(image, CV_32FC1, 1. / 255);//1.0f);//1./255);
		//m_feedbackImage = image;
		cv::Mat img = image.getMat(cv::ACCESS_READ);
		/*
		image = image.reshape(1, 1);
		std::vector<float> vec;
		image.convertTo(vec, CV_32FC1, 1. / 255);//1.0f);//1./255);
		//cv::dnn::blobFromImage(image, inputBlob, 1 / 255.0, m_blobSize);
		//m_inputImage = vec;
		for (size_t c = 0; c < 3; c = ++c) {
			for (size_t i = c; i < vec.size(); i += 3)
			{
				m_inputImage[i] = (float)(vec[i] -mean[c]) / std[c];
			}
		}//*/
		int l = 0;
		for (int k = 0; k < 3; k++)
		{
			for (int i = 0; i < img.rows; i++)
			{
				for (int j = 0; j < img.cols; j++)
				{
					
					m_inputImage[l] = ((((float)img.at<cv::Vec3b>(i, j)[k] / 255.0f) -mean[k]) / std[k]);
					l++;
				}
			}
		}

	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}


	

	try {
		m_outputNames.resize(0);
		for(auto&& str : m_outputNameStrings)
			m_outputNames.push_back(str.c_str());
		m_inputNames.resize(0);
		for (auto&& str : m_inputNameStrings)
			m_inputNames.push_back(str.c_str());

		Ort::RunOptions opts;
		m_session.Run(opts, m_inputNames.data(), &m_inputTensor, 1, m_outputNames.data(), &m_outputTensor, 1);
	}
	catch (Ort::Exception exc) {
		CI_LOG_E("Failed to predict depth: " << exc.what());
	}
	
	try {
		//auto result = cv::Mat(m_height, m_width, CV_32FC1, m_results.data()).getUMat(cv::ACCESS_READ);
		//cv::normalize(result, result, 0.0f, 1.0f, cv::NORM_MINMAX);
		//

		

		//cv::normalize(depth_mat, depth_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
		cv::Mat depth_mat = cv::Mat(m_height, m_width, CV_32FC1, m_results.data());
		//cv::UMat depth_mat = result.getUMat(cv::ACCESS_FAST);

		m_currentCandidates.resize(0);
		m_currentCandidates.push_back(depth_mat.getUMat(cv::ACCESS_READ));
		cv::resize(m_colorImage, m_colorImage, depth_mat.size());
		m_currentCandidates.push_back(m_colorImage);

		// Create a colormap from the depth data
		cv::Mat colormap;
		colormap = depth_mat.mul(cv::Mat::ones(depth_mat.size(), depth_mat.type()), 0.05f);
		colormap.convertTo(colormap, CV_8U, 255);
		cv::applyColorMap(colormap, colormap, cv::COLORMAP_INFERNO);

		cv::resize(colormap, colormap, m_currentImage.size());
		colormap.copyTo(m_feedbackImage);


		
		
		//m_feedbackImage = image;// colormap.getUMat(cv::ACCESS_FAST);
		m_areNewCandidatesAvailable = true;
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
}