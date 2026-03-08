/**
 * @file ldlidar_driver.cpp
 * @author LDRobot (support@ldrobot.com)
 * @brief  ldlidar processing App
 *         This code is only applicable to LDROBOT LiDAR LD14
 * products sold by Shenzhen LDROBOT Co., LTD
 * @version 0.1
 * @date 2021-05-12
 *
 * @copyright Copyright (c) 2022  SHENZHEN LDROBOT CO., LTD. All rights
 * reserved.
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License in the file LICENSE
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
	modified by Lars Engeln in 2026 to run on Windows (and removed network communication mode)
*/

#include "ldlidar_driver.h"

namespace ldlidar {

bool LDLidarDriver::is_ok_ = false;

LDLidarDriver::LDLidarDriver()
    : sdk_version_number_("v3.0.5"),
      is_start_flag_(false),
      register_get_timestamp_handle_(nullptr),
      comm_pkg_(new LiPkg()) {
  last_pubdata_times_ = std::chrono::steady_clock::now();
}

LDLidarDriver::~LDLidarDriver() {
  if (comm_pkg_ != nullptr) {
    delete comm_pkg_;
  }
}

std::string LDLidarDriver::GetLidarSdkVersionNumber(void) {
  return sdk_version_number_;
}

bool LDLidarDriver::Stop(void) {
  if (!is_start_flag_) {
    return true;
  }

  comm_serial_.Close();

  is_start_flag_ = false;
  SetIsOkStatus(false);

  return true;
}

bool LDLidarDriver::WaitLidarCommConnect(int64_t timeout) {
  auto last_time = std::chrono::steady_clock::now();

  bool is_recvflag = false;
  do {
    if (comm_pkg_->GetLidarPowerOnCommStatus()) {
      is_recvflag = true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (!is_recvflag &&
           std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - last_time).count() < timeout);

  if (is_recvflag) {
    last_pubdata_times_ = std::chrono::steady_clock::now();
    return true;
  }
  return false;
}

LidarStatus LDLidarDriver::GetLaserScanData(Points2D& dst, int64_t timeout) {
  if (!is_start_flag_) {
    return LidarStatus::STOP;
  }

  LidarStatus status = comm_pkg_->GetLidarStatus();
  if (LidarStatus::NORMAL == status) {
    if (comm_pkg_->GetLaserScanData(dst)) {
      last_pubdata_times_ = std::chrono::steady_clock::now();
      return LidarStatus::NORMAL;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - last_pubdata_times_).count();
    return elapsed > timeout ? LidarStatus::DATA_TIME_OUT : LidarStatus::DATA_WAIT;
  }

  last_pubdata_times_ = std::chrono::steady_clock::now();
  return status;
}

LidarStatus LDLidarDriver::GetLaserScanData(LaserScan& dst, int64_t timeout) {
  if (!is_start_flag_) {
    return LidarStatus::STOP;
  }

  LidarStatus status = comm_pkg_->GetLidarStatus();
  if (LidarStatus::NORMAL == status) {
    Points2D recvpcd;
    if (comm_pkg_->GetLaserScanData(recvpcd)) {
      last_pubdata_times_ = std::chrono::steady_clock::now();
      dst.stamp  = recvpcd.front().stamp;
      dst.points = recvpcd;
      return LidarStatus::NORMAL;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - last_pubdata_times_).count();
    return elapsed > timeout ? LidarStatus::DATA_TIME_OUT : LidarStatus::DATA_WAIT;
  }

  last_pubdata_times_ = std::chrono::steady_clock::now();
  return status;
}

bool LDLidarDriver::GetLidarSpinFreq(double& spin_hz) {
  if (!is_start_flag_) {
    return false;
  }
  spin_hz = comm_pkg_->GetSpeed();
  return true;
}

void LDLidarDriver::RegisterGetTimestampFunctional(
    std::function<uint64_t(void)> get_timestamp_handle) {
  register_get_timestamp_handle_ = get_timestamp_handle;
}

void LDLidarDriver::EnableFilterAlgorithnmProcess(bool is_enable) {
  comm_pkg_->EnableFilter(is_enable);
}

bool LDLidarDriver::Start(LDType product_name,
                          const std::string& port_name,
                          uint32_t baudrate,
                          CommunicationModeTypeDef comm_mode) {
  if (is_start_flag_) {
    return true;
  }

  if (LDType::NO_VERSION == product_name) {
    LD_LOG_ERROR("input <product_name> is abnormal.", "");
    return false;
  }

  if (register_get_timestamp_handle_ == nullptr) {
    LD_LOG_ERROR("get timestamp functional is not registered.", "");
    return false;
  }

  comm_pkg_->ClearDataProcessStatus();
  comm_pkg_->RegisterTimestampGetFunctional(register_get_timestamp_handle_);
  comm_pkg_->SetProductType(product_name);

  // Wire the Win serial read callback straight into the packet parser
  comm_serial_.SetReadCallback(
      [this](const char* data, size_t len) {
        comm_pkg_->CommReadCallback(data, len);
      });

  if (!comm_serial_.Open(port_name, baudrate)) {
    LD_LOG_ERROR("Failed to open serial port %s at %u baud.",
                 port_name.c_str(), baudrate);
    return false;
  }

  is_start_flag_ = true;
  SetIsOkStatus(true);

  return true;
}

} // namespace ldlidar
/********************* (C) COPYRIGHT SHENZHEN LDROBOT CO., LTD *******END OF
 * FILE ********/