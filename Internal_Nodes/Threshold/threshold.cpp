//
// Plugin Threshold
//

#include "threshold.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Threshold::Threshold() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Threshold");
    SetComponentCategory_(DSPatch::Category::Category_Color);
    SetComponentAuthor_("richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    thresh_method_ = 0;
    thresh_type_ = 0;
    thresh_amt_ = 127;
    adapt_thresh_ = 2;
    adapt_block_ = 11;
    adapt_type_ = 0;
    hsv_low_ = ImVec4(0, 0, 0, 0);
    hsv_high_ = ImVec4(1, 1, 1, 0);

    SetEnabled(true);
}

void Threshold::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            // Process Image
            if (thresh_method_ == 0) {  // Simple
                cv::threshold(*in1, frame, thresh_amt_, 255, thresh_type_);
            }
            else if (thresh_method_ == 1) {  // Color Range
                ImVec4 hsv1, hsv2;
                ImGui::ColorConvertRGBtoHSV(hsv_low_.x, hsv_low_.y, hsv_low_.z, hsv1.x, hsv1.y, hsv1.z);
                ImGui::ColorConvertRGBtoHSV(hsv_high_.x, hsv_high_.y, hsv_high_.z, hsv2.x, hsv2.y, hsv2.z);
                if (in1->channels() > 1)
                    cv::cvtColor(*in1, frame, cv::COLOR_BGR2HSV);
                else {
                    cv::cvtColor(*in1, frame, cv::COLOR_GRAY2BGR);
                    cv::cvtColor(frame, frame, cv::COLOR_BGR2HSV);
                }
                // OpenCV Hue is scaled to half 360 range (0 - 179)
                cv::inRange(frame, cv::Scalar(hsv1.x * 179, hsv1.y * 255, hsv1.z * 255), cv::Scalar(hsv2.x * 179, hsv2.y * 255, hsv2.z * 255), frame);
            }
            else if (thresh_method_ == 2) {  // Adaptive
                if (in1->channels() > 1)
                    cv::cvtColor(*in1, frame, cv::COLOR_BGR2GRAY);
                else
                    in1->copyTo(frame);

                int aThreshType = thresh_type_;
                if (aThreshType > 1)
                    aThreshType = 1;
                cv::adaptiveThreshold(frame, frame, 255, adapt_type_, aThreshType, adapt_block_, adapt_thresh_);
            }
            else if (thresh_method_ == 3) {  // Otsu
                if (in1->channels() > 1)
                    cv::cvtColor(*in1, frame, cv::COLOR_BGR2GRAY);
                else
                    in1->copyTo(frame);
                cv::threshold(frame, frame, 0, 255, thresh_type_ + cv::THRESH_OTSU);
            }

            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Threshold::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Threshold::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Method", GetInstanceName()).c_str(), &thresh_method_, "Simple\0Color Range\0Adaptive\0Otsu\0\0");
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Type", GetInstanceName()).c_str(), &thresh_type_, "Binary\0Binary Inv\0Trunc\0ToZero\0ToZero Inv\0\0");
        if (thresh_method_ == 0) {
            ImGui::Separator();
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Thresh", GetInstanceName()).c_str(), &thresh_amt_, 0.5f, 1, 255);
        }
        else if (thresh_method_ == 1) {
            ImGui::Separator();
            ImGui::ColorEdit3(CreateControlString("Low", GetInstanceName()).c_str(), (float *)&hsv_low_);
            ImGui::ColorEdit3(CreateControlString("High", GetInstanceName()).c_str(), (float *)&hsv_high_);
        }
        else if (thresh_method_ == 2) {
            ImGui::Separator();
            ImGui::SetNextItemWidth(120);
            ImGui::Combo(CreateControlString("Adapt Type", GetInstanceName()).c_str(), &adapt_type_, "Mean\0Gaussian\0\0");
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Adapt Thresh", GetInstanceName()).c_str(), &adapt_thresh_, 0.5f, 0, 255);
            ImGui::SetNextItemWidth(80);
            if (ImGui::InputInt(CreateControlString("Block Size", GetInstanceName()).c_str(), &adapt_block_, 2, 2)) {
                if ((adapt_block_ % 2) == 0)
                    adapt_block_++;
                if (adapt_block_ <= 1)
                    adapt_block_ = 3;
            }
        }
    }
}

std::string Threshold::GetState()
{
    using namespace nlohmann;

    json state;

    state["thresh_method"] = thresh_method_;
    state["thresh_amt"] = thresh_amt_;
    state["thresh_type"] = thresh_type_;
    state["adapt_type"] = adapt_type_;
    state["adapt_thresh"] = adapt_thresh_;
    state["adapt_block"] = adapt_block_;
    json lowColor;
    lowColor["R"] = hsv_low_.x;
    lowColor["G"] = hsv_low_.y;
    lowColor["B"] = hsv_low_.z;
    state["low_range"] = lowColor;
    json highColor;
    highColor["R"] = hsv_high_.x;
    highColor["G"] = hsv_high_.y;
    highColor["B"] = hsv_high_.z;
    state["high_range"] = highColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Threshold::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("low_range")) {
        hsv_low_.x = state["low_range"]["R"].get<float>();
        hsv_low_.y = state["low_range"]["G"].get<float>();
        hsv_low_.z = state["low_range"]["B"].get<float>();
    }
    if (state.contains("high_range")) {
        hsv_high_.x = state["high_range"]["R"].get<float>();
        hsv_high_.y = state["high_range"]["G"].get<float>();
        hsv_high_.z = state["high_range"]["B"].get<float>();
    }
    if (state.contains("thresh_method"))
        thresh_method_ = state["thresh_method"].get<int>();
    if (state.contains("thresh_type"))
        thresh_type_ = state["thresh_type"].get<int>();
    if (state.contains("thresh_amt"))
        thresh_amt_ = state["thresh_amt"].get<int>();
    if (state.contains("adapt_type"))
        adapt_type_ = state["adapt_type"].get<int>();
    if (state.contains("adapt_thresh"))
        adapt_thresh_ = state["adapt_thresh"].get<int>();
    if (state.contains("adapt_block"))
        adapt_block_ = state["adapt_block"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables