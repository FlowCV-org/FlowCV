//
// Created by Richard on 3/26/2022.
//

#ifndef FLOWCV_SERIAL_ENUMERATOR_HPP_
#define FLOWCV_SERIAL_ENUMERATOR_HPP_
#include <iostream>
#include <vector>

#ifdef _WIN32
#include "List_Serial_Ports_Win.hpp"
#endif

#ifdef __linux__
#include "List_Serial_Ports_Linux.hpp"
#endif

#ifdef __APPLE__
#include "List_Serial_Ports_Mac.hpp"
#endif

class Serial_Enumerator
{

  public:
    void RefreshSerialList();
    int GetSerialCount();
    std::string GetSerialName(uint32_t index);
    std::string GetSerialDesc(uint32_t index);
    std::string GetSerialID(uint32_t index);

  private:
    std::vector<SerialDeviceEnum::SerialPortInfo> serial_list_;
};
#endif  // FLOWCV_SERIAL_ENUMERATOR_HPP_
