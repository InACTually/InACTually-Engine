
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

#pragma once
#include <pybind11/embed.h>
#include <string>
#include <memory>

namespace py = pybind11;


namespace act {
	namespace comp {


		class PythonAdaptor  {
		public:
			PythonAdaptor();
			PythonAdaptor(const std::string& modulePath,
				const std::string& venvName,
				const std::string& moduleName,
				const std::string& functionName);

			~PythonAdaptor();

			py::object call(py::args args = py::make_tuple(), py::kwargs kwargs = py::dict());


		private:			
			py::object m_module;
			py::object m_function;

		};
		using PythonAdpatorRef = std::shared_ptr<PythonAdaptor>;
	}
}
