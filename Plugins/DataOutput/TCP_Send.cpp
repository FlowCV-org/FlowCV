//
// Plugin UdpSend
//

#include "TCP_Send.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class TcpSend
{
};
}  // namespace DSPatch

TcpSend::TcpSend()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::TcpSend() )
{
    // Name and Category
    SetComponentName_("TCP_Send");
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
    data_pack_mode_ = 3;
    is_valid_ip = false;
    send_as_binary_ = true;
    transmit_rate_ = 0;
    eol_seq_index_ = 0;
    current_time_ = std::chrono::steady_clock::now();
    last_time_ = current_time_;
    client_ = std::make_unique<asio2::tcp_client>();
    client_->auto_reconnect(true, std::chrono::milliseconds(1000));
    client_->bind_connect([&]() {
        if (asio2::get_last_error())
            printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
        else
            printf("connect success : %s %u\n", client_->local_address().c_str(), client_->local_port());
    }).bind_disconnect([&]()
       {
           printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
       });
    client_->reuse_address(true);

    SetEnabled(true);

}

void TcpSend::SetEOLSeq_()
{
    memset(eol_seq_, '\0', 3);

    switch (eol_seq_index_) {
        case 1:
            eol_seq_[0] = '\r';
            break;
        case 2:
            eol_seq_[0] = '\n';
            break;
        case 3:
            eol_seq_[0] = '\r';
            eol_seq_[1] = '\n';
            break;
        case 4:
            eol_seq_[0] = ' ';
            break;
        case 5:
            eol_seq_[0] = '\t';
            break;
        case 6:
            eol_seq_[0] = ',';
            break;
        default:
            eol_seq_[0] = '\0';
            break;
    }
}

template <typename T>
std::vector<uint8_t> TcpSend::GenerateOutBuffer_(T data)
{
    std::vector<uint8_t> outBuffer;
    if (send_as_binary_) {
        auto *d = reinterpret_cast<uint8_t *>(&data);
        for (int i = 0; i < sizeof(T); i++) {
            outBuffer.emplace_back(d[i]);
        }
    }
    else {
        std::string buf;
        if (typeid(T).name() == typeid(char).name())
            buf = data;
        else
            buf = std::to_string(data);
        for (char &i : buf) {
            outBuffer.emplace_back(i);
        }
    }
    for (int i = 0; i < 2; i++) {
        if (eol_seq_[i] != '\0')
            outBuffer.emplace_back(eol_seq_[i]);
    }
    return outBuffer;
}

void TcpSend::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input Handler
    auto in1 = inputs.GetValue<bool>( 0 );
    auto in2 = inputs.GetValue<int>( 1 );
    auto in3 = inputs.GetValue<float>( 2 );
    auto in4 = inputs.GetValue<std::string>( 3 );
    auto in5 = inputs.GetValue<nlohmann::json>( 4 );

    if (IsEnabled()) {
        if (client_ != nullptr) {
            if (client_->is_started()) {
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
                        std::vector<uint8_t> buf = GenerateOutBuffer_<bool>(*in1);
                        client_->send(asio::buffer(buf.data(), buf.size()));
                    }
                }
                else if (in2) {
                    if (readyToSend) {
                        std::vector<uint8_t> buf = GenerateOutBuffer_<int>(*in2);
                        client_->send(asio::buffer(buf.data(), buf.size()));
                    }
                }
                else if (in3) {
                    if (readyToSend) {
                        std::vector<uint8_t> buf = GenerateOutBuffer_<float>(*in3);
                        client_->send(asio::buffer(buf.data(), buf.size()));
                    }
                }
                else if (in4) {
                    if (readyToSend) {
                        std::vector<uint8_t> buf = GenerateOutBuffer_<char>(*in4->c_str());
                        client_->send(asio::buffer(buf.data(), buf.size()));
                    }
                }
                else if (in5) {
                    if (!in5->empty()) {
                        nlohmann::json json_in_ = *in5;
                        if (readyToSend) {
                            switch (data_pack_mode_) {
                                case 1: // BSON
                                {
                                    std::vector<uint8_t> msg = nlohmann::json::to_bson(json_in_);
                                    client_->send(asio::buffer(msg.data(), msg.size()));
                                    break;
                                }
                                case 2: // CBOR
                                {
                                    std::vector<uint8_t> msg = nlohmann::json::to_cbor(json_in_);
                                    client_->send(asio::buffer(msg.data(), msg.size()));
                                    break;
                                }
                                case 3: // MessagePack
                                {
                                    std::vector<uint8_t> msg = nlohmann::json::to_msgpack(json_in_);
                                    client_->send(asio::buffer(msg.data(), msg.size()));
                                    break;
                                }
                                case 4: // UBJSON
                                {
                                    std::vector<uint8_t> msg = nlohmann::json::to_ubjson(json_in_);
                                    client_->send(asio::buffer(msg.data(), msg.size()));
                                    break;
                                }
                                default: // None (0)
                                    std::string jsonStr = json_in_.dump();
                                    for (int i = 0; i < 2; i++) {
                                        if (eol_seq_[i] != '\0')
                                            jsonStr += eol_seq_[i];
                                    }
                                    client_->send(asio::buffer(jsonStr));
                            }
                        }
                    }
                }
                if (readyToSend)
                    last_time_ = current_time_;
            }
        }
    }
}

bool TcpSend::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void TcpSend::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!is_valid_ip)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(128, 0, 0, 255));
        else
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(29, 47, 73, 255));
        ImGui::SetNextItemWidth(140);
        if (ImGui::InputText(CreateControlString("IP Address", GetInstanceName()).c_str(), tmp_ip_buf, 64)) {
            ip_addr_ = tmp_ip_buf;
            if (IsValidIP_() && port_ != 0) {
                OpenTcpConn_();
            }
        }
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt(CreateControlString("Port", GetInstanceName()).c_str(), &port_)) {
            if (IsValidIP_() && port_ != 0) {
                OpenTcpConn_();
            }
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("JSON Data Mode", GetInstanceName()).c_str(), &data_pack_mode_, "Text\0BSON\0CBOR\0MessagePack\0UBJSON\0\0");
        ImGui::Separator();
        ImGui::SetNextItemWidth(120);
        if (ImGui::Combo(CreateControlString("Rate (Hz)", GetInstanceName()).c_str(), &transmit_rate_, "Fastest\0 60\0 30\0 20\0 15\0 10\0 5\0 2\0 1\0\0")) {
            if (transmit_rate_ > 0)
                rate_val_ = (1.0f / (float)rate_selection_[transmit_rate_]) * 1000.0f;
            else
                rate_val_ = 0;
        }
        ImGui::Separator();
        if (ImGui::Combo(CreateControlString("EOL Sequence", GetInstanceName()).c_str(), &eol_seq_index_, "None\0<CR>\0<LF>\0<CR><LF>\0<SPACE>\0<TAB>\0<COMMA>\0\0")) {
            SetEOLSeq_();
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Non-JSON Data Type Sending Options");
        ImGui::Checkbox(CreateControlString("Send As Binary", GetInstanceName()).c_str(), &send_as_binary_);

    }
}

std::string TcpSend::GetState()
{
    using namespace nlohmann;

    json state;

    state["ip_addr"] = ip_addr_;
    state["port"] = port_;
    state["data_mode"] = data_pack_mode_;
    state["data_rate"] = transmit_rate_;
    state["eol_seq"] = eol_seq_index_;
    state["send_binary"] = send_as_binary_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void TcpSend::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("ip_addr")) {
        ip_addr_ = state["ip_addr"].get<std::string>();
        strcpy(tmp_ip_buf, ip_addr_.c_str());
    }
    if (state.contains("port"))
        port_ = state["port"].get<int>();
    if (state.contains("data_mode"))
        data_pack_mode_ = state["data_mode"].get<int>();
    if (state.contains("data_rate")) {
        transmit_rate_ = state["data_rate"].get<int>();
        if (transmit_rate_ > 0)
            rate_val_ = (1.0f / (float)rate_selection_[transmit_rate_]) * 1000.0f;
        else
            rate_val_ = 0;
    }
    if (state.contains("send_binary"))
        send_as_binary_ = state["send_binary"].get<bool>();
    if (state.contains("eol_seq")) {
        eol_seq_index_ = state["eol_seq"].get<int>();
        SetEOLSeq_();
    }

    if (IsValidIP_() && port_ != 0)
        OpenTcpConn_();

}

void TcpSend::OpenTcpConn_()
{
    if (IsValidIP_()) {
        if (client_ != nullptr) {
            if (client_->is_started())
                client_->stop();
            client_->start(ip_addr_, port_);
        }
    }
}

void TcpSend::CloseTcpConn_()
{
    if (client_->is_started())
        client_->stop();
}

bool TcpSend::IsValidIP_()
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


