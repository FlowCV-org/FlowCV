//
// Plugin BackgroundSubtraction
//

#include "background_subtraction.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

BackgroundSubtraction::BackgroundSubtraction()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Background_Subtraction");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 2 outputs
    SetOutputCount_( 2, {"out", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    detect_shadows_ = false;
    threshold_ = 40;
    history_ = 500;
    bkg_sub_mode_ = 0;
    update_settings_ = true;

    bg_subtractor_mog2_ = cv::createBackgroundSubtractorMOG2();
    bg_subtractor_knn_ = cv::createBackgroundSubtractorKNN();

    SetEnabled(true);

}

void BackgroundSubtraction::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame, mask, masked_frame;
            in1->copyTo(frame);
            // Process Image
            if (update_settings_) {
                // MOG2
                bg_subtractor_mog2_->setHistory(history_);
                bg_subtractor_mog2_->setVarThreshold(threshold_);
                bg_subtractor_mog2_->setDetectShadows(detect_shadows_);

                // KNN
                bg_subtractor_knn_->setHistory(history_);
                bg_subtractor_knn_->setDist2Threshold(threshold_);
                bg_subtractor_knn_->setDetectShadows(detect_shadows_);

                update_settings_ = false;
            }
            if (bkg_sub_mode_ == 0) {
                bg_subtractor_mog2_->apply(frame, mask);
                masked_frame.setTo(cv::Scalar(0,0,0));
                cv::bitwise_and(frame, frame, masked_frame, mask);
            }
            else if (bkg_sub_mode_ == 1) {
                bg_subtractor_knn_->apply(frame, mask);
                masked_frame.setTo(cv::Scalar(0,0,0));
                cv::bitwise_and(frame, frame, masked_frame, mask);
            }
            if (!masked_frame.empty())
                outputs.SetValue(0, masked_frame);
            if (!mask.empty())
                outputs.SetValue(1, mask);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool BackgroundSubtraction::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void BackgroundSubtraction::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::Combo(CreateControlString("Subtraction Mode", GetInstanceName()).c_str(), &bkg_sub_mode_, "MOG2\0KNN\0\0");
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("History", GetInstanceName()).c_str(), &history_, 0.5f, 1, 5000))
            update_settings_ = true;
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragFloat(CreateControlString("Threshold", GetInstanceName()).c_str(), &threshold_, 0.01f, 0.01f, 1000.0f))
            update_settings_ = true;
        if (ImGui::Checkbox(CreateControlString("Detect Shadows", GetInstanceName()).c_str(), &detect_shadows_))
            update_settings_ = true;
    }

}

std::string BackgroundSubtraction::GetState()
{
    using namespace nlohmann;

    json state;

    state["mode"] = bkg_sub_mode_;
    state["history"] = history_;
    state["threshold"] = threshold_;
    state["detect_shadows"] = detect_shadows_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void BackgroundSubtraction::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("mode"))
        bkg_sub_mode_ = state["mode"].get<int>();
    if (state.contains("history"))
        history_ = state["history"].get<int>();
    if (state.contains("threshold"))
        threshold_ = state["threshold"].get<float>();
    if (state.contains("detect_shadows"))
        detect_shadows_ = state["detect_shadows"].get<bool>();

}

} // End Namespace DSPatch::DSPatchables