//
// Plugin GetOptimalDft
//

#include "get_optimal_dft.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

GetOptimalDft::GetOptimalDft() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Get_Optimal_DFT_Size");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "size"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Int_Array});

    // 1 outputs
    SetOutputCount_(1, {"size"}, {IoType::Io_Type_Int_Array});

    has_size_input_ = false;
    has_frame_input_ = false;
    width_ = 1;
    height_ = 1;

    SetEnabled(true);
}

void GetOptimalDft::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<std::vector<int>>(1);

    if (!in1) {
        has_frame_input_ = false;
    }
    else {
        if (!in1->empty())
            has_frame_input_ = true;
        else
            has_frame_input_ = false;
    }
    if (!in2) {
        has_size_input_ = false;
    }
    else {
        if (in2->size() == 2)
            has_size_input_ = true;
        else
            has_size_input_ = false;
    }

    if (IsEnabled()) {
        if (has_frame_input_ || has_size_input_) {
            std::vector<int> dft_size;

            if (has_frame_input_) {
                width_ = cv::getOptimalDFTSize(in1->cols);
                height_ = cv::getOptimalDFTSize(in1->rows);
            }
            else {
                width_ = cv::getOptimalDFTSize(in2->at(0));
                height_ = cv::getOptimalDFTSize(in2->at(1));
            }

            if (dft_size.empty()) {
                dft_size.emplace_back(width_);
                dft_size.emplace_back(height_);
            }
            else {
                dft_size.at(0) = width_;
                dft_size.at(1) = height_;
            }

            outputs.SetValue(0, dft_size);
        }
    }
}

bool GetOptimalDft::HasGui(int interface)
{

    return false;
}

void GetOptimalDft::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);
}

std::string GetOptimalDft::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void GetOptimalDft::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
}

}  // End Namespace DSPatch::DSPatchables