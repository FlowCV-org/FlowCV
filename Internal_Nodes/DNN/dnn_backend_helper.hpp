//
// Helper for getting DNN Backend and Target Lists
//

#ifndef DNN_BACKEND_HELPER_HPP_
#define DNN_BACKEND_HELPER_HPP_
#include <iostream>
#include <vector>
#include <opencv2/dnn.hpp>

class DnnBackendListHelper
{
  public:
    DnnBackendListHelper();
    void UpdateTargetList(cv::dnn::Backend backend);
    std::vector<std::pair<std::string, cv::dnn::Backend>> GetBackEndList();
    std::vector<std::pair<std::string, cv::dnn::Target>> GetTargetList();

  private:
    std::vector<std::pair<std::string, cv::dnn::Backend>> backend_list_;
    std::vector<std::pair<std::string, cv::dnn::Target>> target_list_;
};

#endif  // DNN_BACKEND_HELPER_HPP_
