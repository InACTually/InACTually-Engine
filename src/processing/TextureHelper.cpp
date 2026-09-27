
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "TextureHelper.hpp"


cv::ogl::Texture2D act::proc::TextureHelper::toTexture(cv::UMat mat) {
	try {
		cv::cvtColor(mat, mat, cv::COLOR_BGR2RGBA);

		m_toTex = cv::ogl::Texture2D(mat.rows, mat.cols, cv::ogl::Texture2D::RGBA, true);
		cv::ogl::convertToGLTexture2D(mat, m_toTex);
		//m_texture = ci::gl::Texture2d::create(GL_TEXTURE_2D, m_glTex.texId(), m_glTex.cols(), m_glTex.rows(), true);
	}
	catch (const cv::Exception& e) {
		CI_LOG_E("OpenCV OGL error: " << e.what());
		return cv::ogl::Texture2D();
	}
	return m_toTex;
}

void act::proc::TextureHelper::toTextureAsync(cv::UMat mat, std::function<void(cv::ogl::Texture2D texture)> callback) {
	bool expected = false;
	if (!m_toTexInFlight.compare_exchange_strong(expected, true))
		return;

	auto weakSelf = std::weak_ptr<TextureHelper>(shared_from_this());

	ci::app::App::get()->dispatchAsync([&, weakSelf, mat, callback]() {
		if (auto self = weakSelf.lock()) {
			toTexture(mat);
		}

		m_toTexInFlight = false;
		callback(m_toTex);
	});
}

cv::UMat act::proc::TextureHelper::fromTexture(ci::gl::TextureRef texture) {
	try {
		auto fromTex = cv::ogl::Texture2D(texture->getHeight(), texture->getWidth(), cv::ogl::Texture2D::RGBA, texture->getId(), false);
		cv::ogl::convertFromGLTexture2D(fromTex, m_mat);
	}
	catch (const cv::Exception& e) {
		CI_LOG_E("OpenCV OGL error: " << e.what());
		m_fromTexInFlight = false;
		return cv::UMat();
	}
	return m_mat;
}

void act::proc::TextureHelper::fromTextureAsync(ci::gl::TextureRef texture, std::function<void(cv::UMat mat)> callback) {
	bool expected = false;
	if (!texture || !m_fromTexInFlight.compare_exchange_strong(expected, true))
		return;

	auto weakSelf = std::weak_ptr<TextureHelper>(shared_from_this());

	ci::app::App::get()->dispatchSync([&, weakSelf, texture, callback]() {
		if (auto self = weakSelf.lock()) {
			fromTexture(texture);
		}

		m_fromTexInFlight = false;
		callback(m_mat);
	});
}