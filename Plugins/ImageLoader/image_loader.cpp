//
// Plugin Image Loader
//

#include "image_loader.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class ImageLoader
{
};
}  // namespace DSPatch

ImageLoader::ImageLoader()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::ImageLoader() )
{
    // Name and Category
    SetComponentName_("Image_Loader");
    SetComponentCategory_(Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;
    show_file_dialog_ = false;

    // 1 inputs
    SetInputCount_( 0 );

    // 1 output
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    SetEnabled(true);
}

void ImageLoader::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    if (!IsEnabled())
        SetEnabled(true);

    if (!frame_.empty()) {
        cv::Mat outFrame;
        frame_.copyTo(outFrame);
        outputs.SetValue(0, outFrame);
    }
}

bool ImageLoader::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ImageLoader::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Load Image", GetInstanceName()).c_str())) {
            show_file_dialog_ = true;
        }
        ImGui::Text("Loaded Image:");
        if (image_file_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", image_file_.c_str());

        if(show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Load Image", GetInstanceName()).c_str());

        if(file_dialog_.showFileDialog(CreateControlString("Load Image", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".png,.tga,.jpg,.tif", &show_file_dialog_))
        {
            image_file_ = file_dialog_.selected_path;
            frame_ = cv::imread(image_file_, true);
            show_file_dialog_ = false;
        }
    }

}

std::string ImageLoader::GetState()
{
    using namespace nlohmann;

    json state;

    state["image_path"] = image_file_;
    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ImageLoader::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("image_path")) {
        if (!state["image_path"].empty()) {
            image_file_ = state["image_path"].get<std::string>();
            frame_ = cv::imread(image_file_, true);
        }
    }

}

