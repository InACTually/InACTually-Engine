
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
*/

#pragma once

#include "cinder/Vector.h"
#include <cinder/Color.h>
#include <glm/gtx/color_space.hpp>

// values determined by observation of device's colors, maybe inaccurate
#define amber_r 1.0f
#define amber_g 0.7647f


// Data structure for color with 5 channels
struct RGBAWColor
{
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 0;
	uint8_t w = 0;
};

// two static functions to calculate RGB(A)W color from RGB color by converting it into HSV and back with changed factors
class RGBAWHelper
{
public:
	static RGBAWColor RGBtoRGBAW(ci::Color original)
	{
		glm::vec3 hsv = rgbToHsv(original);				// detailed explanation of what is happening here:
														// used https://www.rapidtables.com/convert/color/hsv-to-rgb.html as reference
		hsv.x = hsv.x * 360.0f;							// calculate hsv from rgb and multiply h*360 to get [H,S,V] = {0-360,0-1,0-1}
		float c = hsv.z * hsv.y;						// 
		float m = hsv.z * (1 - hsv.y);					// calculate (V * (1 - S)) , corresponds to white content of resulting color
		int h = static_cast<int>(hsv.x) % 60;			// hsv to rgb conversion is done stepwise in 60/360 steps based on Hue
		float p = h / 60.0f;							// get fraction of Hue in current step

		float r = 0, g = 0, b = 0, a = 0, w = 0;		// initialize all resulting values

		int s = (int)floor(hsv.x / 60.0f);				// s determines current step 0-5
														// calculation of R, G, B values ist done according to https://upload.wikimedia.org/wikipedia/commons/5/5d/HSV-RGB-comparison.svg
		if (s < 1)										// variances in first two steps to include A channel
		{												// amber is normally made up of R and G, with their fractions defined by amber_r and amber_g
			r = c * (1 - p * amber_r);					// to accomodate for the included amber led, red and green leds are darkened
			g = c * p * (1 - amber_g);
			a = c * p;									// writing to the amber channel, in this step: falling R slope, slightly rising G slope, rising A slope
		}
		else if (s < 2)									// same as befor, but in this step: falling R slope, rising G slope, falling A slope
		{
			r = c * (1 - p) * (1 - amber_r);
			g = c * p * amber_g + (1 - amber_g);
			a = c * (1 - p);
		}
		else if (s < 3)									// from now one, normal calculation of rgb to hsv
		{
			g = c;
			b = c * p;
		}
		else if (s < 4)
		{
			g = c * (1 - p);
			b = c;
		}
		else if (s < 5)
		{
			r = c * p;
			b = c;
		}
		else if (s < 6)
		{
			r = c;
			b = c * (1 - p);
		}

		RGBAWColor res;
		res.r = (uint8_t)(r * 255);						// bring the results from 0-1 to 0-255 as required for DMX channel values
		res.g = (uint8_t)(g * 255);						// normally (g+m) * 255, but this is used to accomodate for saturation (white content of color)
		res.b = (uint8_t)(b * 255);
		res.a = (uint8_t)(a * 255);
		res.w = (uint8_t)(m * 255);						// white content is done by including white led
		
		return res;										// for more information, see "Technische Dokumentation"
	}

	static RGBAWColor RGBtoRGBW(ci::Color original)
	{
		glm::vec3 hsv = rgbToHsv(original);				// calculation from hsv to rgb similar to RGBtoRGBAW(), but without weird amber corrections, see step 0 and 1
		hsv.x = hsv.x * 360.0f;
		float c = hsv.z * hsv.y;
		float m = hsv.z * (1 - hsv.y);
		int h = static_cast<int>(hsv.x) % 60;
		float p = h / 60.0f;

		float r = 0, g = 0, b = 0, a = 0, w = 0;

		int s = (int)floor(hsv.x / 60.0f);
		if (s < 1)
		{
			r = c;
			g = c * p;
		}
		else if (s < 2)
		{
			r = c * (1 - p);
			g = c;
		}
		else if (s < 3)
		{
			g = c;
			b = c * p;
		}
		else if (s < 4)
		{
			g = c * (1 - p);
			b = c;
		}
		else if (s < 5)
		{
			r = c * p;
			b = c;
		}
		else if (s < 6)
		{
			r = c;
			b = c * (1 - p);
		}

		RGBAWColor res;
		res.r = (uint8_t)(r * 255);
		res.g = (uint8_t)(g * 255);
		res.b = (uint8_t)(b * 255);
		res.w = (uint8_t)(m * 255);

		return res;
	}

	static ci::Color HEXtoRGB(std::string hexString)
	{
		size_t startPos = hexString.find("#");
		if (startPos != std::string::npos)
			if (startPos == 0)
				hexString.erase(0, 1);
			else
				throw std::invalid_argument("hexString contains # but not as first character!");

		uint32_t const& hexuint = std::stoul(hexString, nullptr, 16);

		return ci::Color::hex(hexuint);
	}
};
