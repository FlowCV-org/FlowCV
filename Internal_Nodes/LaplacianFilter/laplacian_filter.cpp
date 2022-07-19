//
// Plugin LaplacianFilter
//

#include "laplacian_filter.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

LaplacianFilter::LaplacianFilter()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Laplacian");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    out_depth_ = 2; // -1/CV_16S/CV_32F/CV_64F
    scale_ = 1.0f;
    delta_ = 0.0f;
    ksize_ = 3;

    SetEnabled(true);

}

void LaplacianFilter::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            // Process Image
            cv::Mat lapFrame, frame;
            if (in1->channels() > 1) {
                cv::cvtColor(*in1, lapFrame, cv::COLOR_BGR2GRAY);
            }
            else {
                lapFrame = *in1;
            }

            int depth_mode = -1;

            // -1/CV_16S/CV_32F/CV_64F
            if (out_depth_ == 0)
                depth_mode = -1;
            else if (out_depth_ == 1)
                depth_mode = CV_16S;
            else if (out_depth_ == 2)
                depth_mode = CV_32F;
            else if (out_depth_ == 3)
                depth_mode = CV_64F;

            cv::Laplacian(lapFrame, frame, depth_mode,ksize_, scale_, delta_, cv::BORDER_DEFAULT);
            if (!frame.empty())
                outputs.SetValue(0, frame);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool LaplacianFilter::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void LaplacianFilter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Depth Out", GetInstanceName()).c_str(), &out_depth_,
                     "Auto\0CV_16S\0CV_32F\0CV_64F\0\0");
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputInt(CreateControlString("Kernel Size", GetInstanceName()).c_str(), &ksize_, 2, 2)) {
            if ((ksize_ % 2) == 0) {
                ksize_ += 1;
            }
            if (ksize_ < 1)
                ksize_ = 1;
            if (ksize_ > 7)
                ksize_ = 7;
        }
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &scale_, 0.1f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Delta", GetInstanceName()).c_str(), &delta_, 0.1f);
    }

}

std::string LaplacianFilter::GetState()
{
    using namespace nlohmann;

    json state;

    state["depth_out"] = out_depth_;
    state["scale"] = scale_;
    state["delta"] = delta_;
    state["ksize"] = ksize_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void LaplacianFilter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("depth_out"))
        out_depth_ = state["depth_out"].get<int>();
    if (state.contains("scale"))
        scale_ = state["scale"].get<float>();
    if (state.contains("delta"))
        delta_ = state["delta"].get<float>();
    if (state.contains("ksize"))
        ksize_ = state["ksize"].get<int>();

}

} // End Namespace DSPatch::DSPatchables