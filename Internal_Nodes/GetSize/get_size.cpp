//
// Plugin GetSize
//

#include "get_size.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

GetSize::GetSize()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Get_Size");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"size"}, {IoType::Io_Type_Int_Array} );

    SetEnabled(true);

}

void GetSize::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            std::vector<int> image_size{in1->cols, in1->rows};
            outputs.SetValue(0, image_size);
        }
    }

}

bool GetSize::HasGui(int interface)
{
    return false;
}

void GetSize::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);


}

std::string GetSize::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void GetSize::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);


}

} // End Namespace DSPatch::DSPatchables