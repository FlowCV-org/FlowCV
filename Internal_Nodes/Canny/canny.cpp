//
// Plugin Canny
//

#include "canny.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

CannyFilter::CannyFilter()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Canny");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {DSPatch::IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {DSPatch::IoType::Io_Type_CvMat} );

    thresh_mode_ = 0;
    norm_type_ = 0;
    low_thresh_ = 50;
    high_thresh_ = 150;
    kernel_size_ = 3;

    SetEnabled(true);

}

void CannyFilter::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            // Process Image
            cv::Mat tmp, frame;
            if (in1->channels() > 1)
                cv::cvtColor(*in1, tmp, cv::COLOR_BGR2GRAY);
            else
                tmp = *in1;

            if (thresh_mode_ == 1) {
                cv::Scalar mean, dev;
                cv::meanStdDev(tmp, mean, dev);
                low_thresh_ = (float)(mean[0] - dev[0]);
                high_thresh_ = (float)(mean[0] + dev[0]);
            }
            cv::Canny(tmp, frame, low_thresh_, high_thresh_, kernel_size_, (bool)norm_type_);
            if (!frame.empty())
                outputs.SetValue(0, frame);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool CannyFilter::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void CannyFilter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt(CreateControlString("Sobel Apt Size", GetInstanceName()).c_str(), &kernel_size_, 2)) {
            if (kernel_size_ < 3)
                kernel_size_ = 3;
            if ((kernel_size_ % 2) == 0)
                kernel_size_++;
            if (kernel_size_ > 7)
                kernel_size_ = 7;
        }
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Threshold Mode", GetInstanceName()).c_str(), &thresh_mode_, "Fixed\0Automatic\0\0");
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Gradient Mode", GetInstanceName()).c_str(), &norm_type_, "L1 Norm\0L2 Norm\0\0");
        ImGui::SetNextItemWidth(120);
        ImGui::DragFloat(CreateControlString("Low Threshold", GetInstanceName()).c_str(), &low_thresh_, 0.1f, 1.0f, 5000.0f, "%.2f" );
        ImGui::SetNextItemWidth(120);
        ImGui::DragFloat(CreateControlString("High Threshold", GetInstanceName()).c_str(), &high_thresh_, 0.1f, 1.0f, 5000.0f, "%.2f" );

    }

}

std::string CannyFilter::GetState()
{
    using namespace nlohmann;

    json state;

    state["kernel_size"] = kernel_size_;
    state["thresh_mode"] = thresh_mode_;
    state["norm_type"] = norm_type_;
    state["low_thresh"] = low_thresh_;
    state["high_thresh"] = high_thresh_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void CannyFilter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("kernel_size")) {
        if (state["kernel_size"].is_number()) {
            kernel_size_ = state["kernel_size"].get<int>();
        }
    }
    if (state.contains("thresh_mode")) {
        if (state["thresh_mode"].is_number()) {
            thresh_mode_ = state["thresh_mode"].get<int>();
        }
    }
    if (state.contains("norm_type")) {
        if (state["norm_type"].is_number()) {
            norm_type_ = state["norm_type"].get<int>();
        }
    }
    if (state.contains("low_thresh")) {
        if (state["low_thresh"].is_number()) {
            low_thresh_ = state["low_thresh"].get<float>();
        }
    }
    if (state.contains("high_thresh")) {
        if (state["high_thresh"].is_number()) {
            high_thresh_ = state["high_thresh"].get<float>();
        }
    }

}

} // End Namespace DSPatch::DSPatchables