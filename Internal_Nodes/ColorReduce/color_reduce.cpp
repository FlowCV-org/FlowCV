//
// Plugin ColorReduce
//

#include "color_reduce.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

ColorReduce::ColorReduce()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Color_Reduce");
    SetComponentCategory_(DSPatch::Category::Category_Color);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    div_ = 64;

    SetEnabled(true);

}

void ColorReduce::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            // Process Image
            in1->copyTo(frame);

            int nl = frame.rows;
            int nc = frame.cols * frame.channels();

            if (div_ < 1)
                div_ = 1;
            for (int j = 0; j < nl; j++) {
                uchar* data = frame.ptr<uchar>(j);
                for (int i = 0; i < nc; i++) {
                    data[i] = data[i] / div_ * div_ + div_ / 2;
                }
            }
            if (!frame.empty())
                outputs.SetValue(0, frame);

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool ColorReduce::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ColorReduce::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("Reduce", GetInstanceName()).c_str(), &div_, 0.5f, 1, 255)) {
            if (div_ < 1)
                div_ = 1;
        }
    }

}

std::string ColorReduce::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ColorReduce::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);


}

} // End Namespace DSPatch::DSPatchables