//
// Plugin ImageWriter
//

#include "image_writer.hpp"
#include "FlowLogger.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class ImageWriter
{
};
}  // namespace DSPatch::DSPatchables::internal

ImageWriter::ImageWriter() : Component(ProcessOrder::OutOfOrder), p(new internal::ImageWriter())
{
    // Name and Category
    SetComponentName_("Image_Writer");
    SetComponentCategory_(Category::Category_Output);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    show_file_dialog_ = false;
    save_image_now_ = false;

    // 0 outputs
    SetOutputCount_(0);

    SetEnabled(true);
}

void ImageWriter::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {

            if (save_image_now_) {
                if (!image_file_.empty()) {
                    cv::imwrite(image_file_, *in1);
                }
                save_image_now_ = false;
            }
        }
    }
}

bool ImageWriter::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ImageWriter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Write Image", GetInstanceName()).c_str())) {
            save_image_now_ = true;
        }
        ImGui::Separator();
        if (ImGui::Button(CreateControlString("Set Output Path", GetInstanceName()).c_str())) {
            show_file_dialog_ = true;
        }
        ImGui::Text("Save Image Path:");
        if (image_file_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", image_file_.c_str());

        if (show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Save Image", GetInstanceName()).c_str());

        if (file_dialog_.showFileDialog(CreateControlString("Save Image", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::SAVE,
                ImVec2(700, 310), "*.*", &show_file_dialog_)) {

            LOG_INFO("{}", file_dialog_.selected_path);
            LOG_INFO("{}", file_dialog_.selected_fn);
            LOG_INFO("{}", file_dialog_.ext);

            image_file_ = file_dialog_.selected_path;
            show_file_dialog_ = false;
        }
    }
}

std::string ImageWriter::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ImageWriter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
}
