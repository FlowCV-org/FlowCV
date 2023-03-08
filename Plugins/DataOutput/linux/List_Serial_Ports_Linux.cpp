//
// Platform Serial Device Enumerator
// Linux
//

#include "List_Serial_Ports_Linux.hpp"

namespace SerialDeviceEnum
{

std::vector<std::string> DeviceEnumerator::glob(const std::vector<std::string> &patterns)
{
    std::vector<std::string> paths_found;

    if (patterns.empty())
        return paths_found;

    glob_t glob_results;

    int glob_retval = ::glob(patterns[0].c_str(), 0, nullptr, &glob_results);

    auto iter = patterns.begin();

    while (++iter != patterns.end()) {
        glob_retval = ::glob(iter->c_str(), GLOB_APPEND, nullptr, &glob_results);
    }

    for (int path_index = 0; path_index < glob_results.gl_pathc; path_index++) {
        paths_found.emplace_back(glob_results.gl_pathv[path_index]);
    }

    globfree(&glob_results);

    return paths_found;
}

std::string DeviceEnumerator::basename(const std::string &path)
{
    size_t pos = path.rfind('/');

    if (pos == std::string::npos)
        return path;

    return std::string(path, pos + 1, std::string::npos);
}

std::string DeviceEnumerator::dirname(const std::string &path)
{
    size_t pos = path.rfind('/');

    if (pos == std::string::npos)
        return path;
    else if (pos == 0)
        return "/";

    return std::string(path, 0, pos);
}

bool DeviceEnumerator::path_exists(const std::string &path)
{
    struct stat sb
    {
    };

    if (stat(path.c_str(), &sb) == 0)
        return true;

    return false;
}

std::string DeviceEnumerator::realpath(const std::string &path)
{
    char *real_path = ::realpath(path.c_str(), nullptr);

    std::string result;

    if (real_path != nullptr) {
        result = real_path;

        free(real_path);
    }

    return result;
}

std::string DeviceEnumerator::usb_sysfs_friendly_name(const std::string &sys_usb_path)
{
    unsigned int device_number = 0;

    std::istringstream(read_line(sys_usb_path + "/devnum")) >> device_number;

    std::string manufacturer = read_line(sys_usb_path + "/manufacturer");

    std::string product = read_line(sys_usb_path + "/product");

    std::string serial = read_line(sys_usb_path + "/serial");

    if (manufacturer.empty() && product.empty() && serial.empty())
        return "";

    return format("%s %s %s", manufacturer.c_str(), product.c_str(), serial.c_str());
}

std::vector<std::string> DeviceEnumerator::get_sysfs_info(const std::string &device_path)
{
    std::string device_name = basename(device_path);

    std::string friendly_name;

    std::string hardware_id;

    std::string sys_device_path = format("/sys/class/tty/%s/device", device_name.c_str());

    if (device_name.compare(0, 6, "ttyUSB") == 0) {
        sys_device_path = dirname(dirname(realpath(sys_device_path)));

        if (path_exists(sys_device_path)) {
            friendly_name = usb_sysfs_friendly_name(sys_device_path);

            hardware_id = usb_sysfs_hw_string(sys_device_path);
        }
    }
    else if (device_name.compare(0, 6, "ttyACM") == 0) {
        sys_device_path = dirname(realpath(sys_device_path));

        if (path_exists(sys_device_path)) {
            friendly_name = usb_sysfs_friendly_name(sys_device_path);

            hardware_id = usb_sysfs_hw_string(sys_device_path);
        }
    }
    else {
        // Try to read ID string of PCI device

        std::string sys_id_path = sys_device_path + "/id";

        if (path_exists(sys_id_path))
            hardware_id = read_line(sys_id_path);
    }

    if (friendly_name.empty())
        friendly_name = device_name;

    if (hardware_id.empty())
        hardware_id = "n/a";

    std::vector<std::string> result;
    result.push_back(friendly_name);
    result.push_back(hardware_id);

    return result;
}

std::string DeviceEnumerator::read_line(const std::string &file)
{
    std::ifstream ifs(file.c_str(), std::ifstream::in);

    std::string line;

    if (ifs) {
        getline(ifs, line);
    }

    return line;
}

std::string DeviceEnumerator::format(const char *format, ...)
{
    va_list ap;

    size_t buffer_size_bytes = 256;

    std::string result;

    char *buffer = (char *)malloc(buffer_size_bytes);

    if (buffer == nullptr)
        return result;

    bool done = false;

    unsigned int loop_count = 0;

    while (!done) {
        va_start(ap, format);

        int return_value = vsnprintf(buffer, buffer_size_bytes, format, ap);

        if (return_value < 0) {
            done = true;
        }
        else if (return_value >= buffer_size_bytes) {
            // Realloc and try again.
            buffer_size_bytes = return_value + 1;

            char *new_buffer_ptr = (char *)realloc(buffer, buffer_size_bytes);

            if (new_buffer_ptr == nullptr) {
                done = true;
            }
            else {
                buffer = new_buffer_ptr;
            }
        }
        else {
            result = buffer;
            done = true;
        }

        va_end(ap);

        if (++loop_count > 5)
            done = true;
    }

    free(buffer);

    return result;
}

std::string DeviceEnumerator::usb_sysfs_hw_string(const std::string &sysfs_path)
{
    std::string serial_number = read_line(sysfs_path + "/serial");

    if (serial_number.length() > 0) {
        serial_number = format("SNR=%s", serial_number.c_str());
    }

    std::string vid = read_line(sysfs_path + "/idVendor");

    std::string pid = read_line(sysfs_path + "/idProduct");

    return format("USB VID:PID=%s:%s %s", vid.c_str(), pid.c_str(), serial_number.c_str());
}

std::vector<SerialPortInfo> DeviceEnumerator::GetSerialPortList()
{
    std::vector<SerialPortInfo> results;

    std::vector<std::string> search_globs;
    search_globs.emplace_back("/dev/ttyACM*");
    // search_globs.emplace_back("/dev/ttyS*");
    search_globs.emplace_back("/dev/ttyUSB*");
    // search_globs.emplace_back("/dev/tty.*");
    search_globs.emplace_back("/dev/cu.*");

    std::vector<std::string> devices_found = glob(search_globs);

    auto iter = devices_found.begin();

    while (iter != devices_found.end()) {
        std::string device = *iter++;
        std::vector<std::string> sysfs_info = get_sysfs_info(device);
        std::string friendly_name = sysfs_info[0];
        std::string hardware_id = sysfs_info[1];

        SerialPortInfo device_entry;
        device_entry.port = device;
        device_entry.description = friendly_name;
        device_entry.hardware_id = hardware_id;

        results.push_back(device_entry);
    }

    return results;
}

}  // End Namespace SerialDeviceEnum