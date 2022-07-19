//
// Plugin OscSend
//

#include "OSC_Send.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class OscSend
{
};
}  // namespace DSPatch

OscSend::OscSend()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::OscSend() )
{
    // Name and Category
    SetComponentName_("OSC_Send");
    SetComponentCategory_(Category::Category_Output);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 5, {"bool", "int", "float", "str", "json"}, {IoType::Io_Type_Bool, IoType::Io_Type_Int, IoType::Io_Type_Float, IoType::Io_Type_String, IoType::Io_Type_JSON} );

    // 0 outputs
    SetOutputCount_(0);

    port_ = 0;
    is_multicast_ = false;
    is_valid_ip = false;
    transmit_rate_ = 0;
    current_time_ = std::chrono::steady_clock::now();
    last_time_ = current_time_;

    SetEnabled(true);

}

void HandleNetworkWrite(const std::error_code& error, std::size_t bytes_transferred)
{
    std::cout << "Wrote " << bytes_transferred << " bytes with " << error.message() << std::endl;
}

void OscSend::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input Handler
    auto in1 = inputs.GetValue<bool>( 0 );
    auto in2 = inputs.GetValue<int>( 1 );
    auto in3 = inputs.GetValue<float>( 2 );
    auto in4 = inputs.GetValue<std::string>( 3 );
    auto in5 = inputs.GetValue<nlohmann::json>( 4 );

    if (IsEnabled()) {
        if (socket_ != nullptr) {
            if (socket_->is_open()) {
                current_time_ = std::chrono::steady_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
                bool readyToSend = false;
                if (transmit_rate_ > 0) {
                    if (delta >= (long long)rate_val_) {
                        readyToSend = true;
                    }
                } else
                    readyToSend = true;

                if (in1) {
                    if (readyToSend) {
                        char buf[1024] = {'\0'};
                        osc::OutboundPacketStream oscStream(buf, 1024);
                        oscStream << osc::BeginMessage(osc_addr_prefix_);
                        oscStream << *in1;
                        oscStream << osc::EndMessage;
                        socket_->async_send_to(asio::buffer(oscStream.Data(), oscStream.Size()), *remote_endpoint_, HandleNetworkWrite);
                    }
                }
                else if (in2) {
                    if (readyToSend) {
                        char buf[1024] = {'\0'};
                        osc::OutboundPacketStream oscStream(buf, 1024);
                        oscStream << osc::BeginMessage(osc_addr_prefix_);
                        oscStream << *in2;
                        oscStream << osc::EndMessage;
                        socket_->async_send_to(asio::buffer(oscStream.Data(), oscStream.Size()), *remote_endpoint_, HandleNetworkWrite);
                    }
                }
                else if (in3) {
                    if (readyToSend) {
                        char buf[1024] = {'\0'};
                        osc::OutboundPacketStream oscStream(buf, 1024);
                        oscStream << osc::BeginMessage(osc_addr_prefix_);
                        oscStream << *in3;
                        oscStream << osc::EndMessage;
                        socket_->async_send_to(asio::buffer(oscStream.Data(), oscStream.Size()), *remote_endpoint_, HandleNetworkWrite);
                    }
                }
                else if (in4) {
                    if (readyToSend) {
                        char buf[1024] = {'\0'};
                        osc::OutboundPacketStream oscStream(buf, 1024);
                        std::string oscAddr = osc_addr_prefix_;
                        if (oscAddr.at(oscAddr.size() - 1) != '/')
                            oscAddr += '/';
                        oscAddr += *in4;
                        oscStream << osc::BeginMessage(oscAddr.c_str());
                        //oscStream << 0;
                        oscStream << osc::EndMessage;
                        socket_->async_send_to(asio::buffer(oscStream.Data(), oscStream.Size()), *remote_endpoint_, HandleNetworkWrite);
                    }
                }
                else if (in5) {
                    if (!in5->empty()) {
                        nlohmann::json json_in_ = *in5;
                        if (readyToSend) {
                            std::string oscBaseAddr;
                            if (strlen(osc_addr_prefix_) > 0) {
                                if (osc_addr_prefix_[0] == '/')
                                    oscBaseAddr += osc_addr_prefix_;
                                else {
                                    oscBaseAddr += '/';
                                    oscBaseAddr += osc_addr_prefix_;
                                }
                            }
                            if (!oscBaseAddr.empty()) {
                                if (oscBaseAddr.at(oscBaseAddr.size() - 1) != '/')
                                    oscBaseAddr += '/';
                            }
                            else
                                oscBaseAddr += '/';
                            char buf[8192] = {'\0'};
                            osc::OutboundPacketStream oscStream(buf, 8192);
                            oscStream << osc::BeginBundleImmediate;
                            if (json_in_.contains("ref_frame")) {
                                std::string oscAddr = oscBaseAddr;
                                oscAddr += "ref_frame";
                                oscStream << osc::BeginMessage(oscAddr.c_str());
                                oscStream << json_in_["ref_frame"]["w"].get<int>();
                                oscStream << json_in_["ref_frame"]["h"].get<int>();
                                oscStream << osc::EndMessage;
                            }
                            std::string dataType = "data";
                            if (json_in_.contains("data_type")) {
                                dataType += '/';
                                dataType += json_in_["data_type"].get<std::string>();
                            }
                            if (json_in_.contains("data")) {
                                if (!json_in_["data"].empty()) {
                                    int idx = 0;
                                    for (auto &d: json_in_["data"]) {
                                        for (nlohmann::json::iterator it = d.begin(); it != d.end(); ++it) {
                                            std::string oscAddr = oscBaseAddr;
                                            oscAddr += dataType;
                                            oscAddr += '/';
                                            oscAddr += it.key();
                                            oscAddr += '/';
                                            oscAddr += std::to_string(idx);
                                            if (it->is_string()) {
                                                oscAddr += '/';
                                                oscAddr += it->get<std::string>();
                                            }
                                            oscStream << osc::BeginMessage(oscAddr.c_str());
                                            if (it->is_number_float())
                                                oscStream << it->get<float>();
                                            else if (it->is_number_integer())
                                                oscStream << it->get<int>();
                                            else if (it->is_boolean()) {
                                                oscStream << it->get<bool>();
                                            }
                                            else if (it->is_array()) {
                                                for (int i = 0; i < it->size(); i++) {
                                                    if (it->at(i).is_number_float())
                                                        oscStream << it->at(i).get<float>();
                                                    else if (it->is_number_integer())
                                                        oscStream << it->at(i).get<int>();
                                                }
                                            }
                                            else if (it->is_object()) { // TODO: Handle Deeper Level JSON Objects

                                            }
                                            oscStream << osc::EndMessage;
                                        }
                                        idx++;
                                    }
                                }
                            }
                            oscStream << osc::EndBundle;
                            socket_->async_send_to(asio::buffer(oscStream.Data(), oscStream.Size()), *remote_endpoint_, HandleNetworkWrite);
                        }
                    }
                }
                if (readyToSend) {
                    float curRate = 1.0f / ((float) delta * 0.001f);
                    //std::cout << curRate << std::endl;
                    last_time_ = current_time_;
                }
            }
        }
    }
}

bool OscSend::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void OscSend::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        bool multicastIndicator = is_multicast_;
        ImGui::Checkbox(CreateControlString("Is Multicast", GetInstanceName()).c_str(), &multicastIndicator);
        if (!is_valid_ip)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(128, 0, 0, 255));
        else
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(29, 47, 73, 255));
        ImGui::SetNextItemWidth(140);
        if (ImGui::InputText(CreateControlString("IP Address", GetInstanceName()).c_str(), tmp_ip_buf, 64)) {
            ip_addr_ = tmp_ip_buf;
            if (IsValidIP_() && port_ != 0) {
                OpenSocket_();
            }
        }
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt(CreateControlString("Port", GetInstanceName()).c_str(), &port_)) {
            if (IsValidIP_() && port_ != 0) {
                OpenSocket_();
            }
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(120);
        if (ImGui::Combo(CreateControlString("Rate (Hz)", GetInstanceName()).c_str(), &transmit_rate_, "Fastest\0 60\0 30\0 20\0 15\0 10\0 5\0 2\0 1\0\0")) {
            if (transmit_rate_ > 0)
                rate_val_ = (1.0f / (float)rate_selection_[transmit_rate_]) * 1000.0f;
            else
                rate_val_ = 0;
        }
        ImGui::Separator();
        ImGui::TextUnformatted("OSC Options");
        ImGui::SetNextItemWidth(140);
        ImGui::InputText(CreateControlString("Address Prefix", GetInstanceName()).c_str(), osc_addr_prefix_, 64);
    }
}

std::string OscSend::GetState()
{
    using namespace nlohmann;

    json state;

    state["ip_addr"] = ip_addr_;
    state["port"] = port_;
    state["data_rate"] = transmit_rate_;
    state["osc_addr"] = osc_addr_prefix_;
    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void OscSend::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("ip_addr")) {
        ip_addr_ = state["ip_addr"].get<std::string>();
        strcpy(tmp_ip_buf, ip_addr_.c_str());
    }
    if (state.contains("port"))
        port_ = state["port"].get<int>();
    if (state.contains("osc_addr")) {
        sprintf(osc_addr_prefix_, "%s", state["osc_addr"].get<std::string>().c_str());
    }
    if (state.contains("data_rate")) {
        transmit_rate_ = state["data_rate"].get<int>();
        if (transmit_rate_ > 0)
            rate_val_ = (1.0f / (float)rate_selection_[transmit_rate_]) * 1000.0f;
        else
            rate_val_ = 0;
    }

    if (IsValidIP_() && port_ != 0)
        OpenSocket_();

}

void OscSend::OpenSocket_()
{
    if (IsValidIP_()) {
        auto address = asio::ip::address::from_string(ip_addr_);
        if (socket_ == nullptr)
            socket_ = std::make_unique<asio::ip::udp::socket>(io_service_);
        else {
            if (socket_->is_open())
                socket_->close();
        }
        remote_endpoint_ = std::make_unique<asio::ip::udp::endpoint>(address, port_);
        is_multicast_ = false;
        if (address.is_v4()) {
            socket_->open(asio::ip::udp::v4());
            if (address.is_multicast())
                is_multicast_ = true;
        } else if (address.is_v6()) {
            socket_->open(asio::ip::udp::v6());
            if (address.is_multicast())
                is_multicast_ = true;
        }
    }
}

void OscSend::CloseSocket_()
{
    if (socket_->is_open())
        socket_->close();
}

bool OscSend::IsValidIP_()
{
    if (!ip_addr_.empty()) {
        std::error_code ec;
        auto address = asio::ip::address::from_string(ip_addr_, ec);
        if (ec) {
            is_valid_ip = false;
            std::cerr << ec.message() << std::endl;
        } else {
            if (address.is_v4()) {
                int dotCount = 0;
                for (auto &c: ip_addr_) {
                    if (c == '.')
                        dotCount++;
                }
                if (dotCount == 3) {
                    is_valid_ip = true;
                    return true;
                }
            } else if (address.is_v6()) {
                int dotCount = 0;
                for (auto c: ip_addr_) {
                    if (c == ':')
                        dotCount++;
                }
                if (dotCount == 7) {
                    is_valid_ip = true;
                    return true;
                }
            }
        }
    }

    is_valid_ip = false;
    return false;
}
