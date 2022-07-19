//
// Platform Serial Device Enumerator
// Linux
//

#ifndef FLOWCV_SERIAL_PORTS_LINUX_HPP_
#define FLOWCV_SERIAL_PORTS_LINUX_HPP_
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace SerialDeviceEnum {

struct SerialPortInfo {
    std::string port;
    std::string description;
    std::string hardware_id;
};

class DeviceEnumerator {
 public:
    std::vector<SerialPortInfo> GetSerialPortList();
    static std::vector<std::string> glob(const std::vector<std::string>& patterns);
    static std::string basename(const std::string& path);
    static std::string dirname(const std::string& path);
    static bool path_exists(const std::string& path);
    static std::string realpath(const std::string& path);
    static std::string usb_sysfs_friendly_name(const std::string& sys_usb_path);
    static std::vector<std::string> get_sysfs_info(const std::string& device_path);
    static std::string read_line(const std::string& file);
    static std::string usb_sysfs_hw_string(const std::string& sysfs_path);
    static std::string format(const char* format, ...);

};

} // End Namespace SerialDeviceEnum
#endif //FLOWCV_SERIAL_PORTS_LINUX_HPP_
