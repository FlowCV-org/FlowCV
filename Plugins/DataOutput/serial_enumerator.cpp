//
// Created by Richard on 3/26/2022.
//

#include "serial_enumerator.hpp"

void Serial_Enumerator::RefreshSerialList()
{
    auto spe = SerialDeviceEnum::DeviceEnumerator();

    serial_list_.clear();
    serial_list_ = spe.GetSerialPortList();
    SerialDeviceEnum::SerialPortInfo spi;
    spi.description = "None";
    spi.port = "None";
    if (!serial_list_.empty())
        serial_list_.insert(serial_list_.begin(), spi);
    else
        serial_list_.emplace_back(spi);

}

int Serial_Enumerator::GetSerialCount()
{
    return serial_list_.size();
}

std::string Serial_Enumerator::GetSerialName(uint32_t index)
{
    if (index < serial_list_.size())
        return serial_list_.at(index).port;
}

std::string Serial_Enumerator::GetSerialDesc(uint32_t index)
{
    if (index < serial_list_.size())
        return serial_list_.at(index).description;
}

std::string Serial_Enumerator::GetSerialID(uint32_t index)
{
    if (index < serial_list_.size())
        return serial_list_.at(index).hardware_id;
}
