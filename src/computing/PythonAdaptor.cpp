
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Fabian Töpfer - baniaf@uber.space
*/

#include "PythonAdaptor.hpp"
#include <filesystem>
#include "cinder/Log.h"
#include "cinder/app/App.h"

namespace fs = std::filesystem;

act::comp::PythonAdaptor::PythonAdaptor()   {

}

act::comp::PythonAdaptor::PythonAdaptor(
	const std::string& modulePath,
	const std::string& venvName,
	const std::string& moduleName,
	const std::string& functionName)
{
    try{


        fs::path assetPath = ci::app::getAssetPath("");
        fs::path baseDir = assetPath.parent_path();

        fs::path scriptDir = (baseDir / "../scripts" / modulePath).lexically_normal();
        std::string fullPath = scriptDir.string();
        
        if (!fs::exists(fullPath)) {
            CI_LOG_E("Module path does not exist: " << fullPath);
            throw std::runtime_error("Invalid module path");
        }
        
        auto sys = py::module::import("sys");

        sys.attr("path").attr("insert")(0, fullPath);

        std::string packages = fullPath + venvName + "/Lib/site-packages";
        sys.attr("path").attr("insert")(0, packages);

        sys.attr("prefix") = fullPath + venvName;
        sys.attr("exec_prefix") = fullPath + venvName;

        CI_LOG_I("Python version: " << std::string(pybind11::str(sys.attr("version"))));
        CI_LOG_I("Python executable: " << std::string(pybind11::str(sys.attr("executable"))));
        CI_LOG_I("sys.prefix: " << std::string(pybind11::str(sys.attr("prefix"))));
        CI_LOG_I("sys.exec_prefix: " << std::string(pybind11::str(sys.attr("exec_prefix"))));


        // assigns custom python script to module variable
        try {
            m_module = py::module::import(moduleName.c_str());
        }
        catch (const py::error_already_set& e) {
            CI_LOG_E("Python import error: " << e.what());
        }
        CI_LOG_I("Imported Python module: " << moduleName);

        if (!py::hasattr(m_module, functionName.c_str())) {
            CI_LOG_E("Function '" << functionName << "' not found in module " << moduleName);
            throw std::runtime_error("Missing Python function");
        }


        //assigns target function from module to function variable 
        m_function = m_module.attr(functionName.c_str());

        if (!PyCallable_Check(m_function.ptr())) {
            CI_LOG_E("Attribute '" << functionName << "' is not callable");
            throw std::runtime_error("Python function not callable");
        }

        CI_LOG_I("Successfully bound function: " << functionName);
    }
    catch (const py::error_already_set& e) {
        CI_LOG_E("Python initialization error: " << e.what());
        throw;
    }
}


act::comp::PythonAdaptor::~PythonAdaptor() {}


py::object act::comp::PythonAdaptor::call(py::args args, py::kwargs kwargs) {
    try {
        //try to call the function
        return m_function(*args, **kwargs);
    }
    catch (const py::error_already_set& e) {
        CI_LOG_E("Python exception: " << e.what());

        // Optional: fetch traceback for deeper debugging
        try {
      
          //  auto traceback = py::module::import("traceback");
          //  auto format_exc = traceback.attr("format_exc")();
          //  CI_LOG_E("Traceback: " << std::string(py::str(format_exc)));
        }
        catch (...) {
            CI_LOG_E("Failed to fetch Python traceback.");
        }

        return py::none();
    }
}

