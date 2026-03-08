/**
 * @file serial_interface_win.h
 * @brief Windows serial port interface for ldlidar — mirrors serial_interface_linux
 */

#ifndef __WIN_SERIAL_PORT_H__
#define __WIN_SERIAL_PORT_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>

#include "log_module.h"

namespace ldlidar {

class SerialInterfaceWin {
 public:
  SerialInterfaceWin();
  ~SerialInterfaceWin();

  bool Open(const std::string& port_name, uint32_t baudrate);
  bool Close();
  bool ReadFromIO(uint8_t* rx_buf, uint32_t rx_buf_len, uint32_t* rx_len);
  bool WriteToIo(const uint8_t* tx_buf, uint32_t tx_buf_len, uint32_t* tx_len);

  void SetReadCallback(std::function<void(const char*, size_t)> callback) {
    read_callback_ = callback;
  }

  bool IsOpened() const { return is_opened_.load(); }

 private:
  static void RxThreadProc(void* param);

  HANDLE                                          com_handle_;
  uint32_t                                        com_baudrate_;
  std::atomic<bool>                               is_opened_;
  std::atomic<bool>                               rx_thread_exit_flag_;
  std::thread*                                    rx_thread_;
  long long                                       rx_count_;
  std::function<void(const char*, size_t)>        read_callback_;
};

}  // namespace ldlidar

#endif  // __WIN_SERIAL_PORT_H__
