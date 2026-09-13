
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "stddef.hpp"
#include "Design.hpp"

#include <iostream>
#include <opencv2/opencv.hpp>

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/CinderImGui.h"

#include "CinderOpenCV.h"
#include "imnodes.h"

#include "UniqueIDBase.hpp"
#include "IDBase.hpp"

#include "Port.hpp"
#include "PortMsg.hpp"
#include "PortType.hpp"

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"

#include "RoomNodeBase.hpp"

#include "RPCHandler.hpp"