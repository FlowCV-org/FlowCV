//
// Created by Richard Wardlow
//

#include "Camera_MacOS.hpp"

namespace cam_macos {

    std::map<int, Device> Camera_MacOS_Enumerator::getVideoDevicesMap()
    {
        std::map<int, Device> deviceMap;

        CFMutableDictionaryRef matchingDict;
        io_iterator_t iter;
        kern_return_t kr;
        io_service_t device;

        /* set up a matching dictionary for the class */
        matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
        if (matchingDict == NULL)
        {
            return deviceMap; // fail
        }

        /* Now we have a dictionary, get an iterator.*/
        kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
        if (kr != KERN_SUCCESS)
        {
            return deviceMap;
        }

        int camIndex = -1;
        int camCount = 0;

        while ((device = IOIteratorNext(iter)))
        {
            char devName[255];
            IORegistryEntryGetName(device,devName);
            std::string cameraName = devName;
            std::transform(cameraName.begin(), cameraName.end(), cameraName.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            if (cameraName.find("cam") != std::string::npos) {
                camCount++;
            }
            IOObjectRelease(device);
        }
        //std::cout << camCount << std::endl;

        matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
        kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
        /* iterate */
        while ((device = IOIteratorNext(iter)))
        {
            char devName[255];
            IORegistryEntryGetName(device,devName);
            std::string cameraName = devName;
            std::transform(cameraName.begin(), cameraName.end(), cameraName.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            if (cameraName.find("cam") != std::string::npos) {
                camCount--;
                Device currentDevice;
                currentDevice.id = camCount;
                currentDevice.deviceName = devName;
                deviceMap[camCount] = currentDevice;
            }
            IOObjectRelease(device);
        }

        /* Done, release the iterator */
        IOObjectRelease(iter);

        return deviceMap;

    }

}// End cam_macos namespace
