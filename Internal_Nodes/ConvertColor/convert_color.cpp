//
// Plugin ConvertColor
//

#include "convert_color.hpp"
#include "color_types.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

ConvertColor::ConvertColor()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Convert_Color");
    SetComponentCategory_(DSPatch::Category::Category_Color);
    SetComponentAuthor_("richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    cvt_mode_ = 0;
    channel_error_ = false;

    SetEnabled(true);

}

void ConvertColor::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            // Process Image
            try {
                cv::Mat frame;
                cv::cvtColor(*in1, frame, ConvertColorCodes[cvt_mode_]);
                channel_error_ = false;
                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }
            catch (const std::exception& e)
            {
                std::cout << e.what();
                outputs.SetValue(0, *in1);
                channel_error_ = true;
            }
        } else {
            outputs.SetValue(0, *in1);
            channel_error_ = false;
        }
    }
}

bool ConvertColor::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ConvertColor::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::Combo(CreateControlString("Conversion Type", GetInstanceName()).c_str(), &cvt_mode_, [](void* data, int idx, const char** out_text) {
            *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str();
            return true;
        }, (void*)&ConvertColorNames, (int)ConvertColorNames.size());
        if (channel_error_) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Incompatible Conversion Type");
        }
    }

}

std::string ConvertColor::GetState()
{
    using namespace nlohmann;

    json state;

    state["conv_code"] = cvt_mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ConvertColor::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("conv_code"))
        cvt_mode_ = state["conv_code"].get<int>();

}

} // End Namespace DSPatch::DSPatchables