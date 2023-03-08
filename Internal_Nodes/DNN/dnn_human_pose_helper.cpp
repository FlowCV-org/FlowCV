//
// Plugin DNN Human Pose Helper Functions
//

#include "dnn_human_pose_helper.hpp"

namespace dnn_human_pose_helper
{
const std::vector<std::vector<std::pair<int, int>>> POSE_PAIRS = {
    {// COCO body
        {1, 2}, {1, 5}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {1, 8}, {8, 9}, {9, 10}, {1, 11}, {11, 12}, {12, 13}, {1, 0}, {0, 14}, {14, 16}, {0, 15}, {15, 17}},
    {// MPII body
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {1, 5}, {5, 6}, {6, 7}, {1, 14}, {14, 8}, {8, 9}, {9, 10}, {14, 11}, {11, 12}, {12, 13}},
    {
        // hand
        {0, 1}, {1, 2}, {2, 3}, {3, 4},         // thumb
        {0, 5}, {5, 6}, {6, 7}, {7, 8},         // pinky
        {0, 9}, {9, 10}, {10, 11}, {11, 12},    // middle
        {0, 13}, {13, 14}, {14, 15}, {15, 16},  // ring
        {0, 17}, {17, 18}, {18, 19}, {19, 20}   // index
    }};

const std::vector<std::vector<std::string>> KEYPOINT_NAMES = {{"Nose", "Neck", "R_Shoulder", "R_Elbow", "R_Wrist", "L_Shoulder", "L_Elbow", "L_Wrist", "R_Hip",
                                                                  "R_Knee", "R_Ankle", "L_Hip", "L_Knee", "L_Ankle", "R_Eye", "L_Eye", "R_Ear", "L_Ear"},
    {"Head", "Neck", "R_Shoulder", "R_Elbow", "R_Wrist", "L_Shoulder", "L_Elbow", "L_Wrist", "R_Hip", "R_Knee", "R_Ankle", "L_Hip", "L_Knee", "L_Ankle",
        "Chest", "BKG"},
    {"Wrist", "Thumb_1", "Thumb_2", "Thumb_3", "Thumb_4", "Pinky_1", "Pinky_2", "Pinky_3", "Pinky_4", "Middle_1", "Middle_2", "Middle_3", "Middle_4", "Ring_1",
        "Ring_2", "Ring_3", "Ring_4", "Index_1", "Index_2", "Index_3", "Index_4"}};

const std::vector<cv::Scalar> JOINT_COLORS = {cv::Scalar(255, 0, 0), cv::Scalar(255, 85, 0), cv::Scalar(255, 170, 0), cv::Scalar(255, 255, 0),
    cv::Scalar(170, 255, 0), cv::Scalar(85, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 85), cv::Scalar(0, 255, 170), cv::Scalar(0, 255, 255),
    cv::Scalar(0, 170, 255), cv::Scalar(0, 85, 255), cv::Scalar(0, 0, 255), cv::Scalar(85, 0, 255), cv::Scalar(170, 0, 255), cv::Scalar(255, 0, 255),
    cv::Scalar(255, 0, 170), cv::Scalar(255, 0, 85), cv::Scalar(128, 32, 64), cv::Scalar(64, 128, 32), cv::Scalar(32, 64, 128), cv::Scalar(10, 75, 111)};

const std::vector<std::pair<int, int>> MAPPING_IDX = {{31, 32}, {39, 40}, {33, 34}, {35, 36}, {41, 42}, {43, 44}, {19, 20}, {21, 22}, {23, 24}, {25, 26},
    {27, 28}, {29, 30}, {47, 48}, {49, 50}, {53, 54}, {51, 52}, {55, 56}, {37, 38}, {45, 46}};

PoseSizeInfo getPoseInfo(int index)
{
    PoseSizeInfo poseInfo{};
    poseInfo.midx = -1;

    if (index < KEYPOINT_NAMES.size()) {
        poseInfo.midx = index;
        poseInfo.nparts = (int)KEYPOINT_NAMES.at(index).size();
        poseInfo.npairs = (int)POSE_PAIRS.at(index).size();
    }

    return poseInfo;
}

void getKeyPoints(cv::Mat &probMap, double threshold, std::vector<KeyJoint> &keyPoints)
{
    cv::Mat smoothProbMap;
    cv::GaussianBlur(probMap, smoothProbMap, cv::Size(3, 3), 0, 0);

    cv::Mat maskedProbMap;
    cv::threshold(smoothProbMap, maskedProbMap, threshold, 255, cv::THRESH_BINARY);

    maskedProbMap.convertTo(maskedProbMap, CV_8U, 1);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(maskedProbMap, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (const auto &contour : contours) {
        cv::Mat blobMask = cv::Mat::zeros(smoothProbMap.rows, smoothProbMap.cols, smoothProbMap.type());
        cv::fillConvexPoly(blobMask, contour, cv::Scalar(1));

        double maxVal;
        cv::Point maxLoc;

        cv::minMaxLoc(smoothProbMap.mul(blobMask), nullptr, &maxVal, nullptr, &maxLoc);
        keyPoints.emplace_back(maxLoc, probMap.at<float>(maxLoc.y, maxLoc.x));
    }
}

void splitNetOutputBlobToParts(cv::Mat &netOutputBlob, const cv::Size &targetSize, std::vector<cv::Mat> &netOutputParts)
{
    int nParts = netOutputBlob.size[1];
    int h = netOutputBlob.size[2];
    int w = netOutputBlob.size[3];

    for (int i = 0; i < nParts; ++i) {
        cv::Mat part(h, w, CV_32F, netOutputBlob.ptr(0, i));
        cv::Mat resizedPart;
        cv::resize(part, resizedPart, targetSize, 0, 0, cv::INTER_LINEAR);
        netOutputParts.emplace_back(resizedPart);
    }
}

void populateInterpPoints(const cv::Point &a, const cv::Point &b, int numPoints, std::vector<cv::Point> &interpCoords)
{
    float xStep = ((float)(b.x - a.x)) / (float)(numPoints - 1);
    float yStep = ((float)(b.y - a.y)) / (float)(numPoints - 1);

    interpCoords.emplace_back(a);
    for (int i = 1; i < numPoints - 1; ++i) {
        interpCoords.emplace_back(a.x + xStep * i, a.y + yStep * i);
    }
    interpCoords.emplace_back(b);
}

void getValidPairs(const std::vector<cv::Mat> &netOutputParts, const std::vector<std::vector<KeyJoint>> &detectedKeypoints,
    std::vector<std::vector<ValidPair>> &validPairs, std::set<int> &invalidPairs, float thresh, int midx)
{

    int nInterpSamples = 10;
    float pafScoreTh = 0.1;
    float confTh = thresh;

    for (int k = 0; k < MAPPING_IDX.size(); ++k) {
        int idx1 = MAPPING_IDX.at(k).first;
        int idx2 = MAPPING_IDX.at(k).second;

        if (idx1 < netOutputParts.size() && idx2 < netOutputParts.size()) {
            // A->B constitute a limb
            cv::Mat pafA = netOutputParts.at(idx1);
            cv::Mat pafB = netOutputParts.at(idx2);

            // Find the keypoints for the first and second limb
            if (k >= POSE_PAIRS.at(midx).size())
                continue;
            const std::vector<KeyJoint> &candA = detectedKeypoints.at(POSE_PAIRS.at(midx).at(k).first);
            const std::vector<KeyJoint> &candB = detectedKeypoints.at(POSE_PAIRS.at(midx).at(k).second);

            int nA = (int)candA.size();
            int nB = (int)candB.size();

            /*
              # If keypoints for the joint-pair is detected
              # check every joint in candA with every joint in candB
              # Calculate the distance vector between the two joints
              # Find the PAF values at a set of interpolated points between the joints
              # Use the above formula to compute a score to mark the connection valid
             */

            if (nA != 0 && nB != 0) {
                std::vector<ValidPair> localValidPairs;

                for (int i = 0; i < nA; ++i) {
                    int maxJ = -1;
                    float maxScore = -1;
                    bool found = false;

                    for (int j = 0; j < nB; ++j) {
                        std::pair<float, float> distance(candB.at(j).point.x - candA.at(i).point.x, candB.at(j).point.y - candA.at(i).point.y);
                        float norm = std::sqrt(distance.first * distance.first + distance.second * distance.second);

                        if (!norm) {
                            continue;
                        }

                        distance.first /= norm;
                        distance.second /= norm;

                        // Find p(u)
                        std::vector<cv::Point> interpCoords;
                        populateInterpPoints(candA.at(i).point, candB.at(j).point, nInterpSamples, interpCoords);
                        // Find L(p(u))
                        std::vector<std::pair<float, float>> pafInterp;
                        for (auto &interpCoord : interpCoords) {
                            pafInterp.emplace_back(pafA.at<float>(interpCoord.y, interpCoord.x), pafB.at<float>(interpCoord.y, interpCoord.x));
                        }

                        std::vector<float> pafScores;
                        float sumOfPafScores = 0;
                        int numOverTh = 0;
                        for (auto &l : pafInterp) {
                            float score = l.first * distance.first + l.second * distance.second;
                            sumOfPafScores += score;
                            if (score > pafScoreTh) {
                                ++numOverTh;
                            }
                            pafScores.push_back(score);
                        }

                        float avgPafScore = sumOfPafScores / ((float)pafInterp.size());
                        if (((float)numOverTh) / ((float)nInterpSamples) > confTh) {
                            if (avgPafScore > maxScore) {
                                maxJ = j;
                                maxScore = avgPafScore;
                                found = true;
                            }
                        }

                    } /* j */

                    if (found) {
                        localValidPairs.emplace_back(candA.at(i).id, candB.at(maxJ).id, maxScore);
                    }

                } /* i */
                if (!localValidPairs.empty())
                    validPairs.emplace_back(localValidPairs);
            }
            else {
                invalidPairs.insert(k);
                validPairs.emplace_back();
            }
        }
    } /* k */
}

void getPersonwiseKeypoints(
    const std::vector<std::vector<ValidPair>> &validPairs, const std::set<int> &invalidPairs, std::vector<std::vector<int>> &personwiseKeypoints, int midx)
{
    for (int k = 0; k < MAPPING_IDX.size(); ++k) {
        if (invalidPairs.find(k) != invalidPairs.end()) {
            continue;
        }
        if (!validPairs.empty() && k < validPairs.size()) {
            const std::vector<ValidPair> &localValidPairs(validPairs[k]);

            int indexA(POSE_PAIRS.at(midx).at(k).first);
            int indexB(POSE_PAIRS.at(midx).at(k).second);

            for (int i = 0; i < localValidPairs.size(); ++i) {
                bool found = false;
                int personIdx = -1;

                for (int j = 0; !found && j < personwiseKeypoints.size(); ++j) {
                    if (indexA < personwiseKeypoints.at(j).size() && personwiseKeypoints.at(j).at(indexA) == localValidPairs.at(i).aId) {
                        personIdx = j;
                        found = true;
                    }
                } /* j */

                if (found) {
                    personwiseKeypoints.at(personIdx).at(indexB) = localValidPairs.at(i).bId;
                }
                else if (k < 17) {
                    std::vector<int> lpkp(std::vector<int>(18, -1));

                    if (!localValidPairs.empty()) {
                        lpkp.at(indexA) = localValidPairs.at(i).aId;
                        lpkp.at(indexB) = localValidPairs.at(i).bId;
                    }
                    personwiseKeypoints.emplace_back(lpkp);
                }

            } /* i */
        }     /* k */
    }
}
};  // namespace dnn_human_pose_helper