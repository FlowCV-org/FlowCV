//
// Helper for getting DNN Backend and Target Lists
//

#include "dnn_backend_helper.hpp"
#include <utility>

struct backendListCompare
{
    explicit backendListCompare(std::string  s) : _s(std::move(s)) { }
    bool operator () (std::pair<std::string, cv::dnn::Backend> const& p) const
    {
        return (p.first == _s);
    }
    std::string _s;
};

DnnBackendListHelper::DnnBackendListHelper()
{
    const std::vector<const char *> backends = {"Default", "Halide", "Inference Engine", "OpenCV", "VkCom", "Cuda"};

    auto backendList = cv::dnn::getAvailableBackends();
    backend_list_.emplace_back(backends[0], cv::dnn::DNN_BACKEND_DEFAULT);
    for (const auto &b : backendList) {
        if ((int)b.first == 1000000 || b.first == cv::dnn::DNN_BACKEND_INFERENCE_ENGINE) {
            auto i = std::find_if(backend_list_.begin(), backend_list_.end(), backendListCompare(backends[2]));
            if (i == backend_list_.end()) {
                backend_list_.emplace_back(backends[2], cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
            }
        }
        else {
            if (b.first < backends.size()) {
                auto i = std::find_if(backend_list_.begin(), backend_list_.end(), backendListCompare(backends[b.first]));
                if (i == backend_list_.end()) {
                    backend_list_.emplace_back(backends.at(b.first), b.first);
                }
            }
        }
    }
}

void DnnBackendListHelper::UpdateTargetList(cv::dnn::Backend backend)
{
    const std::vector<const char *> targets = {"CPU", "OpenCL", "OpenCL FP16", "Myriad", "Vulkan", "FPGA", "CUDA", "CUDA FP16", "HDDL"};

    target_list_.clear();
    auto targetList = cv::dnn::getAvailableTargets(backend);
    for (const auto &t : targetList) {
        if ((int)t < targets.size()) {
            target_list_.emplace_back(targets.at((int)t), t);
        }
    }
    std::sort(target_list_.begin(), target_list_.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
}

std::vector<std::pair<std::string, cv::dnn::Backend>> DnnBackendListHelper::GetBackEndList()
{
    return backend_list_;
}

std::vector<std::pair<std::string, cv::dnn::Target>> DnnBackendListHelper::GetTargetList()
{
    return target_list_;
}
