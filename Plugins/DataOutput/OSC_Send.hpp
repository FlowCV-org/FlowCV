//
// Plugin OscSend
//

#ifndef FLOWCV_PLUGIN_OSC_SEND_HPP_
#define FLOWCV_PLUGIN_OSC_SEND_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <asio2/asio2.hpp>
#include "OscOutboundPacketStream.h"

namespace DSPatch::DSPatchables
{
namespace internal
{
class OscSend;
}

class DLLEXPORT OscSend final : public Component
{
 public:
    OscSend();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

 protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    bool IsValidIP_();
    void OpenSocket_();
    void CloseSocket_();

 private:
    std::unique_ptr<internal::OscSend> p;
    std::string ip_addr_;
    char osc_addr_prefix_[64] = {'\0'};
    char tmp_ip_buf[64] = {'\0'};
    int port_;
    int transmit_rate_;
    bool is_multicast_;
    bool is_valid_ip;
    float rate_val_{};
    int rate_selection_[9] = {0,60,30,20,15,10,5,2,1};
    std::chrono::steady_clock::time_point current_time_;
    std::chrono::steady_clock::time_point last_time_;
    asio::io_service io_service_;
    std::unique_ptr<asio::ip::udp::socket> socket_;
    std::unique_ptr<asio::ip::udp::endpoint> remote_endpoint_;

};

EXPORT_PLUGIN( OscSend )

}  // namespace DSPatch::DSPatchables
#endif //FLOWCV_PLUGIN_OSC_SEND_HPP_
