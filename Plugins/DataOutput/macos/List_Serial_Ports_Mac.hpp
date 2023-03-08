//
// Platform Serial Device Enumerator
// MacOS
//

#ifndef FLOWCV_SERIAL_PORTS_MAC_HPP_
#define FLOWCV_SERIAL_PORTS_MAC_HPP_
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <serial/serial.h>

namespace SerialDeviceEnum
{

struct SerialPortInfo
{
    std::string port;
    std::string description;
    std::string hardware_id;
};

class DeviceEnumerator
{

  public:
    std::vector<SerialPortInfo> GetSerialPortList();
};

}  // End Namespace SerialDeviceEnum

#endif  // FLOWCV_SERIAL_PORTS_MAC_HPP_
