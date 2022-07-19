//
// Platform Serial Device Enumerator
// Windows
//

#ifndef FLOWCV_SERIAL_PORTS_WIN_HPP_
#define FLOWCV_SERIAL_PORTS_WIN_HPP_

#include <iostream>
#include <string>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

namespace SerialDeviceEnum {

struct SerialPortInfo {
    std::string port;
    std::string description;
    std::string hardware_id;
};

class DeviceEnumerator {
 public:
    std::vector<SerialPortInfo> GetSerialPortList();

};
} // End Namespace SerialDeviceEnum
#endif //FLOWCV_SERIAL_PORTS_WIN_HPP_
