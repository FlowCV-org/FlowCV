//
// Created by Richard on 6/17/22.
//

#include "List_Serial_Ports_Mac.hpp"

namespace SerialDeviceEnum
{
std::vector<SerialPortInfo> DeviceEnumerator::GetSerialPortList()
{
    std::vector<SerialPortInfo> serPorts;
    std::vector<serial::PortInfo> devices_found = serial::list_ports();

    auto iter = devices_found.begin();

    while (iter != devices_found.end()) {
        serial::PortInfo device = *iter++;

        SerialPortInfo si;
        si.description = device.description;
        si.port = device.port;
        si.hardware_id = device.hardware_id;
        serPorts.emplace_back(si);

        // printf( "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(), device.hardware_id.c_str() );
    }

    return serPorts;
}
}  // namespace SerialDeviceEnum