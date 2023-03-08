//
// Platform Camera Enumerator
//

#include "camera_enumerator.hpp"

#ifdef _WIN32
#include "Camera_MSMF.hpp"
#endif

#ifdef __linux__
#include "Camera_V4L.hpp"
#endif

#ifdef __APPLE__
#include "Camera_MacOS.hpp"
#endif

void Camera_Enumerator::RefreshCameraList()
{

#ifdef _WIN32
    msmf::DeviceEnumerator de;
    std::map<int, msmf::Device> devices;
    devices = de.getVideoDevicesMap();
#endif

#ifdef __linux__
    camv4l::Camera_V4L_Enumerator ce;
    std::map<int, camv4l::Device> devices = ce.getVideoDevicesMap();
#endif

#ifdef __APPLE__
    cam_macos::Camera_MacOS_Enumerator ce;
    std::map<int, cam_macos::Device> devices = ce.getVideoDevicesMap();
#endif

    camera_list_.clear();
    camera_list_.emplace_back(std::make_pair(0, "None"));

    for (const auto &device : devices) {
        uint32_t id = device.first + 1;
        std::string name = device.second.deviceName;
        camera_list_.emplace_back(std::make_pair(id, name));
    }
}

int Camera_Enumerator::GetCameraCount()
{
    return (int)camera_list_.size();
}

std::string Camera_Enumerator::GetCameraName(uint32_t index)
{
    if (index < camera_list_.size())
        return camera_list_.at(index).second;

    return "";
}

int Camera_Enumerator::GetCameraIndex(uint32_t index)
{
    if (index < camera_list_.size())
        return camera_list_.at(index).first;

    return -1;
}