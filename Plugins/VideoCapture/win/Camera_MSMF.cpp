#include "Camera_MSMF.hpp"

std::string ToNarrow(const wchar_t* s, char dfault = '?',
    const std::locale& loc = std::locale())
{
    std::ostringstream stm;

    while (*s != L'\0') {
        stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
    }
    return stm.str();
}

namespace msmf {

    std::map<int, Device> DeviceEnumerator::getVideoDevicesMap() {
        return getDevicesMap(CLSID_VideoInputDeviceCategory);
    }

    std::map<int, Device> DeviceEnumerator::getAudioDevicesMap() {
        return getDevicesMap(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    }

// Returns a map of id and devices that can be used
	std::map<int, Device> DeviceEnumerator::getDevicesMap(REFGUID category)
	{
        std::map<int, Device> deviceMap;

        HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(res)) {

            HRESULT hr;
            IEnumMoniker *pEnum;
            ICreateDevEnum *pDevEnum = nullptr;
            hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, (void **) &pDevEnum);

            if (SUCCEEDED(hr)) {
                // Create an enumerator for the category.
                hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
                if (hr == S_FALSE) {
                    hr = VFW_E_NOT_FOUND; // The category is empty. Treat as an error.
                }
                pDevEnum->Release();

                IMoniker *pMoniker = nullptr;

                int deviceIndex = 0;
                if (pEnum) {
                    while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
                        IPropertyBag *pPropBag;
                        HRESULT hr2 = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
                        if (FAILED(hr2)) {
                            pMoniker->Release();
                            continue;
                        }

                        VARIANT var;
                        VariantInit(&var);

                        Device curDevice;
                        curDevice.id = deviceIndex;

                        // Get description or friendly name.
                        hr2 = pPropBag->Read(L"Description", &var, nullptr);
                        if (FAILED(hr2)) {
                            hr2 = pPropBag->Read(L"FriendlyName", &var, nullptr);
                        }
                        if (SUCCEEDED(hr2)) {
                            curDevice.deviceName = ConvertBSTRToMBS(var.bstrVal);
                            VariantClear(&var);
                        }

                        hr2 = pPropBag->Write(L"FriendlyName", &var);

                        hr2 = pPropBag->Read(L"DevicePath", &var, nullptr);
                        if (SUCCEEDED(hr2)) {
                            curDevice.devicePath = ConvertBSTRToMBS(var.bstrVal);
                            VariantClear(&var);
                        }
                        if (curDevice.deviceName != "NewTek NDI Video") {
                            deviceMap[deviceIndex] = curDevice;
                            deviceIndex++;
                        }
                        pPropBag->Release();
                        pMoniker->Release();
                    }
                    pEnum->Release();
                }
            }
            CoUninitialize();
        }
        return deviceMap;
	}

    std::string DeviceEnumerator::ConvertBSTRToMBS(BSTR bstr)
    {
        int wslen = ::SysStringLen(bstr);
        return ConvertWCSToMBS((wchar_t*)bstr, wslen);
    }

    std::string DeviceEnumerator::ConvertWCSToMBS(const wchar_t* pstr, long wslen)
    {
        int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, nullptr, 0, nullptr, nullptr);

        std::string dblstr(len, '\0');
        len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
            pstr, wslen /* not necessary NULL-terminated */,
            &dblstr[0], len,
            nullptr, nullptr /* no default char */);

        return dblstr;
    }

    std::string DeviceEnumerator::ConvertWCHhartoMBS(BSTR bstr)
    {
        return ToNarrow(bstr);
    }


}




