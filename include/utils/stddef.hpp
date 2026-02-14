
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once


#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <algorithm> 

#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Log.h"

#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"

#include <cinder/Timer.h>
#include "cinder/Json.h"

#include "CinderOpenCV.h"

#include "cinder/CinderImGui.h"
#include "imgui/imgui_stdlib.h"

#include "jsonHelper.hpp"

#include "./../3rd/IconFontCppHeaders/IconsFontAwesome5.h"


using namespace ci;
using namespace ci::app;

// use ppl.h instead
#define OMP_FOR __pragma("omp parallel for")
#define OMP_FOR_TC(threadCount) __pragma("omp parallel for num_threads("+threadCount+")")
#define OMP_PARALLEL __pragma("omp parallel")
#define OMP_PARALLEL_TC(threadCount) __pragma("omp parallel num_threads("+threadCount+")")

#define STD_LOWPASS 0.15f
#define STD_ANIMTIME 0.5f

#define Ref(className, refName) class ClassName; using refName = std::shared_ptr<className>;

#define CREATE(className, base) static std::shared_ptr<base> create() { return std::make_shared<className>(); }; \
static std::shared_ptr<className> cast(std::shared_ptr<base> baseRef) { return std::dynamic_pointer_cast<className>(baseRef); };

namespace act {

	/**
	* @brief is in one of these State in its whole lifecylce
	*
	*/
	enum GeneralAppState {
		AS_ERROR = -1,  /**< panic */
		AS_STARTUP = 0, /**< setup */
		AS_INITIALISING,/**< init */
		AS_RUNNING,     /**< update/draw */
		AS_CLEANUP,     /**< preparing shutdown */
		AS_FEATURETEST  /**< special for testing purposes */
	};

	/**
	* @brief global AppState
	*
	*/
	class AppState {
	public:
		static GeneralAppState get() {
			return m_state;
		};
		static void set(GeneralAppState state) {
			m_state = state;
		};
	private:
		AppState() {
		}
		static GeneralAppState m_state;
	};
	
	/**
	* @brief SettingsParameter contains all parameter necessary for DFT
	*
	*/
	struct SettingsParameter {
		int			windowSize		= 2048;				/**< FFT input size */
		int			samplerate		= 48000;			/**< samples per second */ // audio::master()->getSampleRate();
		int			fontSize		= 24;				/**< general size of the font */
		ci::ivec2	debugGUISize	= ivec2(0, 0);		/**< window size if debug-GUI will be shown */
		ci::ivec2	guiSize			= ivec2(600, 400);	/**< window size if performace-GUI will be shown */
		bool		showDebugGUI	= false;			/**< show debug-GUI */
	};
	/**
	* @brief Settings contain all fundamental parameter (SpectralParameter)
	*
	*/
	class Settings {
	public:
		/**
		* @brief returns SettingsParameter, at first usage it loads the assets/settings.json
		*
		*/
		static SettingsParameter& get() {
			return getInstance().m_settingsParams;
		}

		static Settings& getInstance() {
			static Settings d;
			return d;
		}

		static void save() {
			getInstance().write();
		}

		SettingsParameter m_settingsParams;

	private:
		Settings() {
			fs::path path = app::getAssetPath("settings.json");
			if (path.empty()) {
				path = app::getAssetPath("").string() + "settings.json";
				ci::writeJson(path, ""); // touch
				write();
			}
			
			cinder::Json json = ci::loadJson(loadFile(path));
			util::setValueFromJson(json, "windowSize",		m_settingsParams.windowSize);
			util::setValueFromJson(json, "samplerate",		m_settingsParams.samplerate);
			util::setValueFromJson(json, "fontSize",		m_settingsParams.fontSize);
			util::setValueFromJson(json, "debugGUISize",	m_settingsParams.debugGUISize);
			util::setValueFromJson(json, "guiSize",			m_settingsParams.guiSize);
			util::setValueFromJson(json, "showGUI",			m_settingsParams.showDebugGUI);
		}

		void write() {
			cinder::Json json = cinder::Json::object();
			json["windowSize"]		= m_settingsParams.windowSize;
			json["samplerate"]		= m_settingsParams.samplerate;
			json["fontSize"]		= m_settingsParams.fontSize;
			json["debugGUISize"]	= m_settingsParams.debugGUISize;
			json["guiSize"]			= m_settingsParams.guiSize;
			json["showGUI"]			= m_settingsParams.showDebugGUI;
			ci::writeJson(getAssetPath("settings.json"), json);
		}
	};

	/**
	* @brief general utilities and helper 
	*
	*/
	namespace util {

		static float angleBetween(glm::vec3 a, glm::vec3 b,	glm::vec3 origin) {
			glm::vec3 da = glm::normalize(a - origin);
			glm::vec3 db = glm::normalize(b - origin);
			return glm::acos(glm::dot(da, db));
		}

		static glm::quat rotationBetween(glm::vec3 a, glm::vec3 b) {
			a = glm::normalize(a);
			b = glm::normalize(b);

			float cosTheta = glm::dot(a, b);
			vec3 rotationAxis;

			if (cosTheta < -1 + 0.001f) {
				// special case when vectors in opposite directions:
				// there is no "ideal" rotation axis
				// So guess one; any will do as long as it's perpendicular to start
				rotationAxis = glm::cross(vec3(0.0f, 0.0f, 1.0f), a);
				if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
					rotationAxis = glm::cross(vec3(1.0f, 0.0f, 0.0f), a);

				rotationAxis = normalize(rotationAxis);
				return glm::angleAxis(glm::radians(180.0f), rotationAxis);
			}

			rotationAxis = glm::cross(a, b);

			float s = sqrt((1 + cosTheta) * 2);
			float invs = 1 / s;

			return glm::quat(
				s * 0.5f,
				rotationAxis.x * invs,
				rotationAxis.y * invs,
				rotationAxis.z * invs
			);

		}

		/**
		* @brief clamps a value between a min and max value
		* @param min min-value
		* @param max max-value
		*/
		template<class T>
		struct MinMaxValue {
			MinMaxValue(T min = 0, T max = 255) : min(min), max(max), value(min) {
			}
			T value;
			T min;
			T max;
			T getValue() { return value; }
			void setValue(T val) { 
				value = std::clamp(val, min, max); 
			}
			void operator= (const MinMaxValue& v) {
				min = v.min;
				max = v.max;
				value = v.value;
			}
			void operator= (const T& v) {
				setValue(v);
			}
			T operator() () {
				return getValue();
			}
		};

		/**
		* @brief crops a var between a min and max value
		* @param var to be croped, contains the result
		* @param min min-value
		* @param max max-value
		* @return cropped var
		*/
		template< typename T >
		T crop(T& var, T min, T max)
		{
			var = (T)fmax(fmin(var, max), min);
			return var;
		}

		static float linearBinToFreq(int i, float samplerate, int numOfBins) {
			return (float)i * (samplerate / (numOfBins * 2.0f)) + ((samplerate / (numOfBins * 2.0f)) * 0.5f); //###REFACTOR
		}

		static int freqToLinearBin(float freq, float samplerate, int numOfBins) {
			return (int)(freq / (samplerate / (numOfBins * 2.0f)));
		}

		/**
		* @brief simple lowPass, that lowpasses a successive timeseries (A and B) - f.e. 'high' fluctuations/differences are punshed
		* @param input value A
		* @param output value B, contains the result
		* @param alpha the near to 1.0f, the less is the punishment
		* @return output
		*/
		template< typename T > // float, vec3, vec4
		T lowPassF(T input, T& output, float alpha = STD_LOWPASS) {
			if (output == T(0.0f)) return input;

			output = output + alpha * (input - output);

			return output;
		}

		/**
		* @brief measures the time a function f consumes
		* @param f function to be measured
		* @return time in milliseconds the function was consuming 
		*/
		template <class Function>
		double stopwatch(Function&& f) {
			ci::Timer timer;
			timer.start();
			double begin = timer.getSeconds();
			f();
			double elapsedTime = timer.getSeconds() - begin;
			std::stringstream strstr;
			strstr << "Time elapsed: " << elapsedTime << "s";
			log::Metadata meta;
			meta.mLevel = log::LEVEL_INFO;
			ci::log::LoggerConsole().write(meta, strstr.str());
			return elapsedTime;
		}

		/**
		* @brief constructs a ColorGradient for easy use
		*/
		class ColorGradient {
		public:
			ColorGradient() {
				add(ci::ColorA::gray(0.0f, 0.0f), 0.0f);
				add(ci::ColorA::gray(0.0f, 0.0f), 1.0f);
			};
			~ColorGradient() {};

			/**
			* @brief adds a color to the gradient at position
			* @param color color to be added
			* @param position position [0.0f, 1.0f] of the color in the gradient
			*/
			void add(ci::ColorA color, float position) {
				colors[position] = color;
			};

			/**
			* @brief get the interpolated color
			* @param position [0.0f, 1.0f] of the color in the gradient
			* @return interpolated result as color
			*/
			ci::ColorA at(float position) {
				if (position <= 0.0f)
					return colors[0.0f];
				if (position >= 1.0f)
					return colors[1.0f];
				if(abs(position) == NAN)
					return colors[0.0f];

				ci::ColorA	lowerColor = colors[0.0f];
				ci::ColorA	higherColor;
				
				float	lowerValue = 0.0f;
				float	higherValue = 1.0f;

				for (auto& kv : colors) {
					if (kv.first > position) {
						higherValue = kv.first;
						higherColor = kv.second;
							//higherColor = ci::rgbToHsv(higherColor);

						break;
					}
					lowerValue = kv.first;
					lowerColor = kv.second;
				}

				float alpha = (position - lowerValue) / (higherValue - lowerValue);
				return ci::lerp(lowerColor, higherColor, alpha);
			}


			cv::Mat toColorMap(ColorGradient colorGradient) {
				cv::Mat colorMap = cv::Mat(256, 1, CV_8UC3, cv::Scalar(0));

				for (int c = 0; c < colorMap.rows; c++)
				{
					colorMap.row(c).setTo(toOcv(colorGradient.at((float)c / 255.0f)));
				}
				return colorMap;
			}

			static std::shared_ptr<ColorGradient> create() {
				return std::make_shared<ColorGradient>();
			}

		private:
			std::map<float, ci::ColorA> colors;
		};

		inline ci::Rectf fitRoi(ci::Rectf roi, cv::UMat dst) //  roi.canonicalize(); oOcv(Area(roi))
		{
			cv::Rect dstRect = cv::Rect(0, 0, dst.cols, dst.rows);
			return fromOcv(toOcv(Area(roi)) & dstRect);
		}
		inline cv::Rect fitRoi(cv::Rect roi, cv::UMat dst) //  roi.canonicalize(); oOcv(Area(roi))
		{
			cv::Rect dstRect = cv::Rect(0, 0, dst.cols, dst.rows);
			return roi & dstRect;
		}

		inline void insertBlend(cv::UMat& src, cv::UMat& dst, int x, int y)
		{
			if (src.rows == 0 || src.cols == 0)
				return;

			cv::UMat dstRoi = dst(fitRoi(cv::Rect(x, y, src.cols, src.rows), dst));
			cv::UMat srcRoi = src(fitRoi(cv::Rect(-x, -y, dst.cols, dst.rows), src));

			cv::max(srcRoi, dstRoi, dstRoi);
		}

		inline void insertMixed(cv::UMat& src, cv::UMat& dst, int x, int y)
		{
			if (src.rows == 0 || src.cols == 0)
				return;

			cv::UMat dstRoi = dst(fitRoi(cv::Rect(x, y, src.cols, src.rows), dst));
			cv::UMat srcRoi = src(fitRoi(cv::Rect(-x, -y, dst.cols, dst.rows), src));

			cv::addWeighted(srcRoi, 0.5f, dstRoi, 0.5f, 0.0f, dstRoi);
		}

		inline void insertCopy(cv::UMat& src, cv::UMat& dst, int x, int y)
		{
			cv::UMat dstRoi = dst(fitRoi(cv::Rect(x, y, src.cols, src.rows), dst));
			cv::UMat srcRoi = src(fitRoi(cv::Rect(-x, -y, dst.cols, dst.rows), src));

			srcRoi.copyTo(dstRoi);
		}

		inline void rotateMat(cv::UMat& src, cv::UMat& dst, float angle, cv::Point2f center)
		{
			// cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);

			cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
			// determine bounding rectangle, center not relevant
			cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect();
			// adjust transformation matrix
			rot.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
			rot.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;

			cv::warpAffine(src, dst, rot, bbox.size());
		}

		inline void scaleMat(cv::UMat& src, cv::UMat& dst, float scaleX, float scaleY)
		{
			dst = cv::UMat((int)(src.rows * scaleY), (int)(src.cols * scaleX), src.type());
			cv::resize(src, dst, dst.size(), 0, 0, 1);
		}
		
		inline std::vector<std::string> splitString(std::string str, std::string delimiter) {
			size_t pos = 0;
			std::vector<std::string> tokens;

			while ((pos = str.find(delimiter)) != std::string::npos) {
				tokens.push_back(str.substr(0, pos));
				str.erase(0, pos + delimiter.length());
			}
			tokens.push_back(str);

			return tokens;
		}

		static void drawCoords(float size = 1.0f) {
			auto c = ci::gl::ScopedColor();
			auto l = ci::gl::ScopedLineWidth(3.0f);
			gl::color(Color(1, 0, 0));
			ci::gl::drawLine(vec3(0, 0, 0), vec3(size, 0, 0));
			gl::color(Color(0, 1, 0));
			ci::gl::drawLine(vec3(0, 0, 0), vec3(0, size, 0));
			gl::color(Color(0, 0, 1));
			ci::gl::drawLine(vec3(0, 0, 0), vec3(0, 0, size));
		}
	}
}