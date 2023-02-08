//
// Plugin DNN Human Pose Helper Functions
//

#ifndef FLOWCV_PLUGIN_DNN_HUMAN_POSE_HELPER_HPP_
#define FLOWCV_PLUGIN_DNN_HUMAN_POSE_HELPER_HPP_
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include <vector>

namespace dnn_human_pose_helper {

extern const std::vector<std::vector<std::pair<int, int>>> POSE_PAIRS;
extern const std::vector<cv::Scalar> JOINT_COLORS;
extern const std::vector<std::pair<int,int>> MAPPING_IDX;
extern const std::vector<std::vector<std::string>> KEYPOINT_NAMES;

struct KeyJoint{
    KeyJoint(cv::Point point,float probability){
        this->id = -1;
        this->point = point;
        this->probability = probability;
    }
    int id;
    cv::Point point;
    float probability;
};

struct ValidPair{
    ValidPair(int aId,int bId,float score){
        this->aId = aId;
        this->bId = bId;
        this->score = score;
    }
    int aId;
    int bId;
    float score;
};

struct PoseSizeInfo{
    int midx;
    int npairs;
    int nparts;
};

PoseSizeInfo getPoseInfo(int index);

void getKeyPoints(cv::Mat& probMap,double threshold,std::vector<KeyJoint>& keyPoints);

void splitNetOutputBlobToParts(cv::Mat& netOutputBlob,const cv::Size& targetSize,std::vector<cv::Mat>& netOutputParts);

void populateInterpPoints(const cv::Point& a,const cv::Point& b,int numPoints,std::vector<cv::Point>& interpCoords);

void getValidPairs(const std::vector<cv::Mat>& netOutputParts,
                   const std::vector<std::vector<KeyJoint>>& detectedKeypoints,
                   std::vector<std::vector<ValidPair>>& validPairs,
                   std::set<int>& invalidPairs, float thresh, int midx);

void getPersonwiseKeypoints(const std::vector<std::vector<ValidPair>>& validPairs,
                            const std::set<int>& invalidPairs,
                            std::vector<std::vector<int>>& personwiseKeypoints, int midx);
};

#endif // FLOWCV_PLUGIN_DNN_HUMAN_POSE_HELPER_HPP_
