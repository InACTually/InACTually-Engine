/**
 * Windows serial port interface for ldlidar — mirrors serial_interface_linux
 * Lars Engeln - 2026
 */

#include "serial_interface_win.h"

#define MAX_ACK_BUF_LEN 4096

namespace ldlidar {

SerialInterfaceWin::SerialInterfaceWin()
    : com_handle_(INVALID_HANDLE_VALUE),
      com_baudrate_(0),
      is_opened_(false),
      rx_thread_exit_flag_(false),
      rx_thread_(nullptr),
      rx_count_(0),
      read_callback_(nullptr) {
}

SerialInterfaceWin::~SerialInterfaceWin() {
  Close();
}

bool SerialInterfaceWin::Open(const std::string& port_name, uint32_t baudrate) {
  // Support both "COM3" and "\\.\COM3" style names; ports >= COM10 need the \\.\\ prefix
  std::string device_path = (port_name.rfind("\\\\.\\", 0) == 0)
                                ? port_name
                                : "\\\\.\\" + port_name;

  com_handle_ = CreateFileA(
      device_path.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0,        // exclusive access
      nullptr,  // no security attributes
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

  if (com_handle_ == INVALID_HANDLE_VALUE) {
    LD_LOG_ERROR("SerialInterfaceWin: CreateFile failed for %s, error %lu",
                 port_name.c_str(), GetLastError());
    return false;
  }

  com_baudrate_ = baudrate;

  // --- comm timeouts: non-blocking read, return immediately with whatever is available ---
  COMMTIMEOUTS timeouts{};
  timeouts.ReadIntervalTimeout         = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier  = 0;
  timeouts.ReadTotalTimeoutConstant    = 0;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant   = 0;
  if (!SetCommTimeouts(com_handle_, &timeouts)) {
    LD_LOG_ERROR("SerialInterfaceWin: SetCommTimeouts failed, error %lu", GetLastError());
    CloseHandle(com_handle_);
    com_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }

  // --- DCB: baud rate, 8N1, no flow control ---
  DCB dcb{};
  dcb.DCBlength = sizeof(DCB);
  if (!GetCommState(com_handle_, &dcb)) {
    LD_LOG_ERROR("SerialInterfaceWin: GetCommState failed, error %lu", GetLastError());
    CloseHandle(com_handle_);
    com_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }

  dcb.BaudRate        = baudrate;
  dcb.ByteSize        = 8;
  dcb.StopBits        = ONESTOPBIT;
  dcb.Parity          = NOPARITY;
  dcb.fBinary         = TRUE;
  dcb.fParity         = FALSE;
  dcb.fOutxCtsFlow    = FALSE;
  dcb.fOutxDsrFlow    = FALSE;
  dcb.fDtrControl     = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fOutX           = FALSE;
  dcb.fInX            = FALSE;
  dcb.fRtsControl     = RTS_CONTROL_DISABLE;
  dcb.fAbortOnError   = FALSE;

  if (!SetCommState(com_handle_, &dcb)) {
    LD_LOG_ERROR("SerialInterfaceWin: SetCommState failed, error %lu", GetLastError());
    CloseHandle(com_handle_);
    com_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }

  // Verify actual baud rate was accepted
  DCB dcb_verify{};
  dcb_verify.DCBlength = sizeof(DCB);
  if (GetCommState(com_handle_, &dcb_verify)) {
    LDS_LOG_INFO("SerialInterfaceWin: Actual BaudRate reported: %lu", dcb_verify.BaudRate);
  }

  PurgeComm(com_handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);

  rx_thread_exit_flag_ = false;
  is_opened_ = true;
  rx_thread_ = new std::thread(RxThreadProc, this);

  return true;
}

bool SerialInterfaceWin::Close() {
  if (!is_opened_.load()) {
    return true;
  }

  rx_thread_exit_flag_ = true;
  is_opened_ = false;

  if (com_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(com_handle_);
    com_handle_ = INVALID_HANDLE_VALUE;
  }

  if (rx_thread_ != nullptr && rx_thread_->joinable()) {
    rx_thread_->join();
    delete rx_thread_;
    rx_thread_ = nullptr;
  }

  return true;
}

bool SerialInterfaceWin::ReadFromIO(uint8_t* rx_buf, uint32_t rx_buf_len, uint32_t* rx_len) {
  if (!IsOpened() || com_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD bytes_read = 0;
  if (!ReadFile(com_handle_, rx_buf, rx_buf_len, &bytes_read, nullptr)) {
    return false;
  }

  if (rx_len) {
    *rx_len = static_cast<uint32_t>(bytes_read);
  }
  return bytes_read > 0;
}

bool SerialInterfaceWin::WriteToIo(const uint8_t* tx_buf, uint32_t tx_buf_len, uint32_t* tx_len) {
  if (!IsOpened() || com_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD bytes_written = 0;
  if (!WriteFile(com_handle_, tx_buf, tx_buf_len, &bytes_written, nullptr)) {
    return false;
  }

  if (tx_len) {
    *tx_len = static_cast<uint32_t>(bytes_written);
  }
  return bytes_written > 0;
}

void SerialInterfaceWin::RxThreadProc(void* param) {
  SerialInterfaceWin* self = static_cast<SerialInterfaceWin*>(param);
  char* rx_buf = new char[MAX_ACK_BUF_LEN + 1];

  while (!self->rx_thread_exit_flag_.load()) {
    uint32_t bytes_read = 0;
    bool ok = self->ReadFromIO(reinterpret_cast<uint8_t*>(rx_buf), MAX_ACK_BUF_LEN, &bytes_read);
    if (ok && bytes_read > 0) {
      self->rx_count_ += bytes_read;
      if (self->read_callback_ != nullptr) {
        self->read_callback_(rx_buf, bytes_read);
      }
    } else {
      // Nothing available — yield briefly to avoid busy-spin
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  delete[] rx_buf;
}

}  // namespace ldlidar
