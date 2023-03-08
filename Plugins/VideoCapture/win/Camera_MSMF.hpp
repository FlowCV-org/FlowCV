#pragma once
#include <mfapi.h>
#include <mfobjects.h>
#include <Mfidl.h>
#include <map>
#include <string>
#include <locale>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <dshow.h>

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "strmiids.lib")

namespace msmf
{

struct Device
{
    int id;  // This can be used to open the device in OpenCV
    std::string devicePath;
    std::string deviceName;  // This can be used to show the devices to the user
};

class DeviceEnumerator
{

  public:
    DeviceEnumerator() = default;
    std::map<int, Device> getDevicesMap(REFGUID category);
    std::map<int, Device> getVideoDevicesMap();
    std::map<int, Device> getAudioDevicesMap();

  private:
    std::string ConvertBSTRToMBS(BSTR bstr);
    std::string ConvertWCSToMBS(const wchar_t *pstr, long wslen);
    std::string ConvertWCHhartoMBS(BSTR bstr);
};
}  // namespace msmf
