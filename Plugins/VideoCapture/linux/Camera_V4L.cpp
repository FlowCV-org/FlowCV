//
// Created by Richard on 2/26/2022.
//

#include "Camera_V4L.hpp"

namespace camv4l {

std::map<int, Device> Camera_V4L_Enumerator::getVideoDevicesMap()
{
    std::map<int, Device> deviceMap;

    namespace fs = std::filesystem;

    if (fs::exists("/sys/class/video4linux")) {
        for (int i = 0; i < 64; i++) {
            std::string vidFile = "/sys/class/video4linux/video";
            vidFile += std::to_string(i);
            if (fs::exists(vidFile)) {
                std::string nameFile = vidFile + "/name";

                std::string camName;
                int camIndex = -1;

                std::fstream f1(nameFile, std::fstream::in);
                std::string s1;
                getline(f1, s1, '\n');
                f1.close();
                camName = s1;

                if (camName.find("Metadata") == std::string::npos) {
                    camIndex = i - 1;
                    Device currentDevice;
                    currentDevice.id = camIndex;
                    currentDevice.deviceName = camName;
                    deviceMap[camIndex] = currentDevice;
                }
            }
        }
    }

    return deviceMap;

}

}// End camv4l namespace
