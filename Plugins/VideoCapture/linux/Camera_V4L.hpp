//
// Created by Richard on 2/26/2022.
//

#ifndef FLOWCV_CAMERA_V4L_HPP_
#define FLOWCV_CAMERA_V4L_HPP_
#include <iostream>
#include <map>
#include <fstream>
#include <filesystem>

namespace camv4l
{

struct Device
{
    int id;                  // This can be used to open the device in OpenCV
    std::string deviceName;  // This can be used to show the devices to the user
};

class Camera_V4L_Enumerator
{
  public:
    std::map<int, Device> getVideoDevicesMap();

  private:
};
}  // namespace camv4l
#endif  // FLOWCV_CAMERA_V4L_HPP_
