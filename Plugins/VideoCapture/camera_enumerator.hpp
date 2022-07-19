//
// Platform Camera Enumerator
//

#ifndef FLOWCV_CAMERA_ENUMERATOR_HPP_
#define FLOWCV_CAMERA_ENUMERATOR_HPP_

#include <iostream>
#include <vector>

class Camera_Enumerator {
  public:
    void RefreshCameraList();
    int GetCameraCount();
    std::string GetCameraName(uint32_t index);
    int GetCameraIndex(uint32_t index);

  private:
    std::vector<std::pair<int, std::string>> camera_list_;

};

#endif //FLOWCV_CAMERA_ENUMERATOR_HPP_
