
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

#pragma once

#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"

#include "opencv2/core/opengl.hpp"

namespace act {
	namespace proc {


		class TextureHelper : public std::enable_shared_from_this<TextureHelper>
		{
		public:
			TextureHelper() {};
			virtual ~TextureHelper() {};	
			static std::shared_ptr<TextureHelper> create() { return std::make_shared<TextureHelper>(); };
			
			cv::ogl::Texture2D toTexture(cv::UMat mat);
			void toTextureAsync(cv::UMat mat, std::function<void(cv::ogl::Texture2D texture)> callback);
			cv::UMat fromTexture(ci::gl::TextureRef texture);
			void fromTextureAsync(ci::gl::TextureRef texture, std::function<void(cv::UMat mat)> callback);
			
			cv::ogl::Texture2D getTexture() { return m_toTex; };
			ci::gl::Texture2dRef getTextureRef() { return ci::gl::Texture2d::create(GL_TEXTURE_2D, m_toTex.texId(), m_toTex.cols(), m_toTex.rows(), true); };
			cv::UMat getUMat() { return m_mat; };

		private:
			cv::ogl::Texture2D		m_toTex;

			cv::UMat				m_mat;
			cv::ogl::Texture2D		m_fromTex;

			std::atomic_bool		m_toTexInFlight{ false };
			std::atomic_bool		m_fromTexInFlight{ false };

		}; using TextureHelperRef = std::shared_ptr<TextureHelper>;

	}
}