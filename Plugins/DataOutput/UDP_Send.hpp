//
// Plugin UdpSend
//

#ifndef FLOWCV_PLUGIN_UDP_SEND_HPP_
#define FLOWCV_PLUGIN_UDP_SEND_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <asio2/udp/udp_client.hpp>

namespace DSPatch::DSPatchables
{
namespace internal
{
class UdpSend;
}

class DLLEXPORT UdpSend final : public Component
{
  public:
    UdpSend();
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    bool IsValidIP_();
    void OpenSocket_();
    void CloseSocket_();
    void SetEOLSeq_();
    template <typename T> std::vector<uint8_t> GenerateOutBuffer_(T data);

  private:
    std::unique_ptr<internal::UdpSend> p;
    std::string ip_addr_;
    char tmp_ip_buf[64] = {'\0'};
    char eol_seq_[3] = {'\0'};
    int eol_seq_index_;
    int port_;
    int data_pack_mode_;
    int transmit_rate_;
    bool is_multicast_;
    bool is_valid_ip;
    bool send_as_binary_;
    float rate_val_{};
    int rate_selection_[9] = {0,60,30,20,15,10,5,2,1};
    std::chrono::steady_clock::time_point current_time_;
    std::chrono::steady_clock::time_point last_time_;
    asio::io_service io_service_;
    std::unique_ptr<asio::ip::udp::socket> socket_;
    std::unique_ptr<asio::ip::udp::endpoint> remote_endpoint_;

};

EXPORT_PLUGIN( UdpSend )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PLUGIN_UDP_SEND_HPP_
