
/*
    InACTually
    > interactive theater for actual acts
    > this file is part of the "InACTually Engine", a MediaServer for driving all technology

    Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
    Copyright (c) 2025 InACTually Community
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    This file is created and substantially modified: 2021-2023

    contributors:
    Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/Vector.h"

#include "cinder/Json.h"

using namespace ci;

namespace act {
	namespace util {

        static std::string reduceFloat(float value, int precision) {
            std::ostringstream out;
            if (precision == 0)
                out << std::fixed << value;
            else
                out << std::fixed << std::setprecision(precision) << value;
            return out.str();
        }

        static bool setValueFromJson(ci::Json json, std::string key, int& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, float& value) {
            if (json.contains(key)) {
                try {
                    if (json[key].type() == ci::Json::value_t::string)
                        value = std::stof((std::string)json[key]);
                    else
                        value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, double& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, std::string& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, bool& value) {
            if (json.contains(key)) {
                try {
                    value = (bool)(json[key]);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, vec2& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, ivec2& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, vec3& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y) && setValueFromJson(vec, "z", value.z);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, Color& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "r", value.r) && setValueFromJson(vec, "g", value.g) && setValueFromJson(vec, "b", value.b);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, quat& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y) && setValueFromJson(vec, "z", value.z) && setValueFromJson(vec, "w", value.w);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }
	

		static ci::Json valueToJson(ci::vec2 vec) {
			ci::Json json = ci::Json::object();
			json["x"] = vec.x;
			json["y"] = vec.y;
			return json;
		}

        static ci::Json valueToJson(ci::vec3 vec) {
            ci::Json json = ci::Json::object();
            json["x"] = vec.x;
            json["y"] = vec.y;
            json["z"] = vec.z;
            return json;
        }
		
        static ci::Json valueToJson(ci::vec3 vec, int precision) {
			ci::Json json = ci::Json::object();
            json["x"] = reduceFloat(vec.x, precision);
            json["y"] = reduceFloat(vec.y, precision);
            json["z"] = reduceFloat(vec.z, precision);
            return json; 
		}

        static ci::Json valueToJson(ci::Color color) {
            ci::Json json = ci::Json::object();
            json["r"] = color.r;
            json["g"] = color.g;
            json["b"] = color.b;
            return json;
        }

        static ci::Json valueToJson(ci::quat q, int precision) {
            ci::Json json = ci::Json::object();
            json["x"] = reduceFloat(q.x, precision);
            json["y"] = reduceFloat(q.y, precision);
            json["z"] = reduceFloat(q.z, precision);
            json["w"] = reduceFloat(q.w, precision);
            return json;
        }

        static ci::Json valueToJson(ci::quat q) {
            ci::Json json = ci::Json::object();
            json["x"] = q.x;
            json["y"] = q.y;
            json["z"] = q.z;
            json["w"] = q.w;
            return json;
        }
	}
}