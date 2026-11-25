
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
		if (mat.empty())
			return cv::ogl::Texture2D();

		cv::UMat rgba;
		switch (mat.channels()) {
		case 1:
			cv::cvtColor(mat, rgba, cv::COLOR_GRAY2RGBA);
			break;
		case 3:
			cv::cvtColor(mat, rgba, cv::COLOR_BGR2RGBA);
			break;
		case 4:
			cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
			break;
		default:
			return cv::ogl::Texture2D();
		}

		cv::ogl::Texture2D toTex(rgba.rows, rgba.cols, cv::ogl::Texture2D::RGBA, true);
		cv::ogl::convertToGLTexture2D(rgba, toTex);
		{
			std::lock_guard<std::mutex> lock(m_toTexMutex);
			m_toTex = toTex;
		}
		return toTex;
	}
	catch (const cv::Exception& e) {
		CI_LOG_E("OpenCV OGL error: " << e.what());
		return cv::ogl::Texture2D();
	}
	catch (const std::exception& e) {
		CI_LOG_E("Texture conversion error: " << e.what());
		return cv::ogl::Texture2D();
	}
	catch (...) {
		CI_LOG_E("Texture conversion error: unknown exception");
		return cv::ogl::Texture2D();
	}
}

void act::proc::TextureHelper::toTextureAsync(cv::UMat mat, std::function<void(cv::ogl::Texture2D texture)> callback) {
	bool expected = false;
	if (!m_toTexInFlight.compare_exchange_strong(expected, true))
		return;

	auto weakSelf = std::weak_ptr<TextureHelper>(shared_from_this());
	cv::UMat matCopy = mat.clone();

	ci::app::App::get()->dispatchSync([weakSelf, matCopy, callback]() {
		if (auto self = weakSelf.lock()) {
			cv::ogl::Texture2D texture;
			try {
				texture = self->toTexture(matCopy);
			}
			catch (const std::exception& e) {
				CI_LOG_E("toTextureAsync error: " << e.what());
			}
			catch (...) {
				CI_LOG_E("toTextureAsync error: unknown exception");
			}

			self->m_toTexInFlight = false;
			try {
				callback(texture);
			}
			catch (const std::exception& e) {
				CI_LOG_E("toTextureAsync callback error: " << e.what());
			}
			catch (...) {
				CI_LOG_E("toTextureAsync callback error: unknown exception");
			}
		}
	});
}

cv::UMat act::proc::TextureHelper::fromTexture(ci::gl::TextureRef texture) {
	try {
		auto fromTex = cv::ogl::Texture2D(texture->getHeight(), texture->getWidth(), cv::ogl::Texture2D::RGBA, texture->getId(), false);
		cv::UMat mat;
		cv::ogl::convertFromGLTexture2D(fromTex, mat);
		{
			std::lock_guard<std::mutex> lock(m_matMutex);
			m_mat = mat;
		}
		return mat;
	}
	catch (const cv::Exception& e) {
		CI_LOG_E("OpenCV OGL error: " << e.what());
		m_fromTexInFlight = false;
		return cv::UMat();
	}
}

void act::proc::TextureHelper::fromTextureAsync(ci::gl::TextureRef texture, std::function<void(cv::UMat mat)> callback) {
	bool expected = false;
	if (!texture || !m_fromTexInFlight.compare_exchange_strong(expected, true))
		return;

	auto weakSelf = std::weak_ptr<TextureHelper>(shared_from_this());

	ci::app::App::get()->dispatchSync([weakSelf, texture, callback]() {
		if (auto self = weakSelf.lock()) {
			cv::UMat mat;
			try {
				mat = self->fromTexture(texture);
			}
			catch (const std::exception& e) {
				CI_LOG_E("fromTextureAsync error: " << e.what());
			}
			catch (...) {
				CI_LOG_E("fromTextureAsync error: unknown exception");
			}

			self->m_fromTexInFlight = false;
			try {
				callback(mat);
			}
			catch (const std::exception& e) {
				CI_LOG_E("fromTextureAsync callback error: " << e.what());
			}
			catch (...) {
				CI_LOG_E("fromTextureAsync callback error: unknown exception");
			}
		}
	});
}

cv::ogl::Texture2D act::proc::TextureHelper::getTexture() {
	std::lock_guard<std::mutex> lock(m_toTexMutex);
	return m_toTex;
}

ci::gl::Texture2dRef act::proc::TextureHelper::getTextureRef() {
	std::lock_guard<std::mutex> lock(m_toTexMutex);
	if (m_toTex.empty())
		return nullptr;
	return ci::gl::Texture2d::create(GL_TEXTURE_2D, m_toTex.texId(), m_toTex.cols(), m_toTex.rows(), true);
}

cv::UMat act::proc::TextureHelper::getUMat() {
	std::lock_guard<std::mutex> lock(m_matMutex);
	return m_mat;
}

bool act::proc::TextureHelper::hasTexture() {
	std::lock_guard<std::mutex> lock(m_toTexMutex);
	return !m_toTex.empty();
}

bool act::proc::TextureHelper::hasUMat() {
	std::lock_guard<std::mutex> lock(m_matMutex);
	return !m_mat.empty();
}