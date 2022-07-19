//
// Plugin DiscreteCosineTransform
//

#include "discrete_cosine_transform.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

DiscreteCosineTransform::DiscreteCosineTransform()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("DCT");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    mode_ = 0;

    SetEnabled(true);

}

void DiscreteCosineTransform::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat dctFrame, frame;
            // Process Image
            if (in1->channels() > 1) {
                cv::cvtColor(*in1, dctFrame, cv::COLOR_BGR2GRAY);
            }
            else {
                in1->copyTo(dctFrame);
            }

            int dft_mode = mode_;

            if (mode_ > 1) {
                if (mode_ == 2)
                    dft_mode = cv::DFT_ROWS;
            }

            if (dctFrame.type() != CV_32F)
                dctFrame.convertTo(dctFrame, CV_32F, 1.0/255);

            cv::dct(dctFrame, frame, dft_mode);

            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
    }
}

bool DiscreteCosineTransform::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void DiscreteCosineTransform::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Operation", GetInstanceName()).c_str(), &mode_,
                     "Default\0Inverse\0Rows\0\0");
    }

}

std::string DiscreteCosineTransform::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DiscreteCosineTransform::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);


}

} // End Namespace DSPatch::DSPatchables