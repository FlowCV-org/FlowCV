//
// Plugin BlobDetector
//

#include "blob_detector.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

BlobDetector::BlobDetector()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Blob_Detector");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {DSPatch::IoType::Io_Type_CvMat} );

    // 2 outputs
    SetOutputCount_( 2, {"viz", "blobs"}, {DSPatch::IoType::Io_Type_CvMat, DSPatch::IoType::Io_Type_JSON} );

    blob_param_color_ = 255;
    blob_params_.blobColor = blob_param_color_;
    blob_params_.filterByColor = true;
    blob_params_.filterByArea = true;
    blob_params_.filterByCircularity = false;
    blob_params_.filterByConvexity = false;
    blob_params_.filterByInertia = false;
    blob_params_.minArea = 0;
    blob_params_.maxArea = 5000;
    blob_params_.minCircularity = 0;
    blob_params_.maxCircularity = 1;
    blob_params_.minConvexity = 0;
    blob_params_.maxConvexity = 1;
    blob_params_.minInertiaRatio = 0;
    blob_params_.maxInertiaRatio = 1;
    blob_params_.minThreshold = 10;
    blob_params_.maxThreshold = 200;
    blob_params_.minDistBetweenBlobs = 10;
    blob_params_.thresholdStep = 10;
    blob_param_repeat_ = 2;
    blob_params_.minRepeatability = 2;

    SetEnabled(true);
}

void BlobDetector::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            in1->copyTo(frame);
            // Process Image
            cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(blob_params_);
            std::vector<cv::KeyPoint> keypoints;
            detector->detect(frame, keypoints);

            cv::drawKeypoints(frame, keypoints, frame,
                              cv::Scalar(blob_viz_color_.z * 255, blob_viz_color_.y * 255, blob_viz_color_.x * 255),
                              cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

            nlohmann::json json_out;
            nlohmann::json jBlobs;
            json_out["data_type"] = "keypoints";
            nlohmann::json ref;
            ref["w"] = frame.cols;
            ref["h"] = frame.rows;
            json_out["ref_frame"] = ref;
            if (!keypoints.empty()) {
                for (auto &k : keypoints) {
                    nlohmann::json cTmp;
                    cTmp["x"] = k.pt.x;
                    cTmp["y"] = k.pt.y;
                    cTmp["size"] = k.size;
                    jBlobs.emplace_back(cTmp);
                }
                json_out["data"] = jBlobs;
                outputs.SetValue(1, json_out);
            }

            if (!frame.empty())
                outputs.SetValue(0, frame);

            detector.release();
        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool BlobDetector::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void BlobDetector::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Text("Threshold:");
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Min##Thresh", GetInstanceName()).c_str(), &blob_params_.minThreshold, 0.25f, 0.0f, 255.0f, "%.0f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Max##Thresh", GetInstanceName()).c_str(), &blob_params_.maxThreshold, 0.25f, 0.0f, 255.0f, "%.0f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(CreateControlString("Step##Thresh", GetInstanceName()).c_str(), &blob_params_.thresholdStep, 0.01f, 0.0f, 255.0f, "%.2f");
        if (blob_params_.thresholdStep < 1)
            blob_params_.thresholdStep = 1;
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Min Distance Between Blobs", GetInstanceName()).c_str(), &blob_params_.minDistBetweenBlobs, 0.1f, 0.0f, 5000.0f, "%.1f");
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Min Repeatability", GetInstanceName()).c_str(), &blob_param_repeat_, 0.25f, 0, 255);
        if (blob_param_repeat_ < 1)
            blob_param_repeat_ = 1;
        blob_params_.minRepeatability = blob_param_repeat_;

        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter By Color", GetInstanceName()).c_str(), &blob_params_.filterByColor);
        if (blob_params_.filterByColor) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Color", GetInstanceName()).c_str(), &blob_param_color_, 0.25f, 0, 255);
            blob_params_.blobColor = blob_param_color_;
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter By Area", GetInstanceName()).c_str(), &blob_params_.filterByArea);
        if (blob_params_.filterByArea) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Min##Area", GetInstanceName()).c_str(), &blob_params_.minArea, 0.1f, 0.0f, 5000.0f, "%.1f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Max##Area", GetInstanceName()).c_str(), &blob_params_.maxArea, 0.1f, 0.0f, 5000.0f, "%.1f");
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter By Circularity", GetInstanceName()).c_str(), &blob_params_.filterByCircularity);
        if (blob_params_.filterByCircularity) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Min##Circularity", GetInstanceName()).c_str(), &blob_params_.minCircularity, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Max##Circularity", GetInstanceName()).c_str(), &blob_params_.maxCircularity, 0.001f, 0.0f, 1.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter By Convexity", GetInstanceName()).c_str(), &blob_params_.filterByConvexity);
        if (blob_params_.filterByConvexity) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Min##Convexity", GetInstanceName()).c_str(), &blob_params_.minConvexity, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Max##Convexity", GetInstanceName()).c_str(), &blob_params_.maxConvexity, 0.001f, 0.0f, 1.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter By Inertia", GetInstanceName()).c_str(), &blob_params_.filterByInertia);
        if (blob_params_.filterByInertia) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Min##Inertia", GetInstanceName()).c_str(), &blob_params_.minInertiaRatio, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat(CreateControlString("Max##Inertia", GetInstanceName()).c_str(), &blob_params_.maxInertiaRatio, 0.001f, 0.0f, 1.0f);
        }
        ImGui::Separator();
        ImGui::Text("Visualization Settings");
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Blob Color", GetInstanceName()).c_str(), (float*)&blob_viz_color_);
    }

}

std::string BlobDetector::GetState()
{
    using namespace nlohmann;

    json state;

    json bColor;
    bColor["R"] = blob_viz_color_.x;
    bColor["G"] = blob_viz_color_.y;
    bColor["B"] = blob_viz_color_.z;
    state["blob_viz_color"] = bColor;
    state["filter_by_area"] = blob_params_.filterByArea;
    state["filter_by_circularity"] = blob_params_.filterByCircularity;
    state["filter_by_color"] = blob_params_.filterByColor;
    state["filter_by_convexity"] = blob_params_.filterByConvexity;
    state["filter_by_inertia"] = blob_params_.filterByInertia;
    state["color"] = blob_params_.blobColor;
    state["max_area"] = blob_params_.maxArea;
    state["max_circularity"] = blob_params_.maxCircularity;
    state["max_convexity"] = blob_params_.maxConvexity;
    state["max_inertia"] = blob_params_.maxInertiaRatio;
    state["max_threshold"] = blob_params_.maxThreshold;
    state["min_area"] = blob_params_.minArea;
    state["min_circularity"] = blob_params_.minCircularity;
    state["min_convexity"] = blob_params_.minConvexity;
    state["min_dist"] = blob_params_.minDistBetweenBlobs;
    state["min_inertia"] = blob_params_.minInertiaRatio;
    state["min_repeat"] = blob_params_.minRepeatability;
    state["min_threshold"] = blob_params_.minThreshold;
    state["threshold_step"] = blob_params_.thresholdStep;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void BlobDetector::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("blob_viz_color")) {
        blob_viz_color_.x = state["blob_viz_color"]["R"].get<float>();
        blob_viz_color_.y = state["blob_viz_color"]["G"].get<float>();
        blob_viz_color_.z = state["blob_viz_color"]["B"].get<float>();
    }

    if (state.contains("filter_by_area"))
        blob_params_.filterByArea = state["filter_by_area"].get<bool>();
    if (state.contains("filter_by_circularity"))
        blob_params_.filterByCircularity = state["filter_by_circularity"].get<bool>();
    if (state.contains("filter_by_color"))
        blob_params_.filterByColor = state["filter_by_color"].get<bool>();
    if (state.contains("filter_by_convexity"))
        blob_params_.filterByConvexity = state["filter_by_convexity"].get<bool>();
    if (state.contains("filter_by_inertia"))
        blob_params_.filterByInertia = state["filter_by_inertia"].get<bool>();
    if (state.contains("color")) {
        blob_params_.blobColor = state["color"].get<unsigned char>();
        blob_param_color_ = (int)blob_params_.blobColor;
    }
    if (state.contains("max_area"))
        blob_params_.maxArea = state["max_area"].get<float>();
    if (state.contains("max_circularity"))
        blob_params_.maxCircularity = state["max_circularity"].get<float>();
    if (state.contains("max_convexity"))
        blob_params_.maxConvexity = state["max_convexity"].get<float>();
    if (state.contains("max_inertia"))
        blob_params_.maxInertiaRatio = state["max_inertia"].get<float>();
    if (state.contains("max_threshold"))
        blob_params_.maxThreshold = state["max_threshold"].get<float>();
    if (state.contains("min_area"))
        blob_params_.minArea = state["min_area"].get<float>();
    if (state.contains("min_circularity"))
        blob_params_.minCircularity = state["min_circularity"].get<float>();
    if (state.contains("min_convexity"))
        blob_params_.minConvexity = state["min_convexity"].get<float>();
    if (state.contains("min_dist"))
        blob_params_.minDistBetweenBlobs = state["min_dist"].get<float>();
    if (state.contains("min_inertia"))
        blob_params_.minInertiaRatio = state["min_inertia"].get<float>();
    if (state.contains("min_repeat")) {
        blob_params_.minRepeatability = state["min_repeat"].get<float>();
        blob_param_repeat_ = blob_params_.minRepeatability;
    }
    if (state.contains("min_threshold"))
        blob_params_.minThreshold = state["min_threshold"].get<float>();
    if (state.contains("threshold_step"))
        blob_params_.thresholdStep = state["threshold_step"].get<float>();
}

} // End Namespace DSPatch::DSPatchables