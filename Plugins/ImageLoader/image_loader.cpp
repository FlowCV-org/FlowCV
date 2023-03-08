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
}  // namespace DSPatch::DSPatchables::internal

ImageLoader::ImageLoader() : Component(ProcessOrder::InOrder), p(new internal::ImageLoader())
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
    SetInputCount_(0);

    // 1 output
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    fps_ = 30;
    fps_index_ = 7;
    last_time_ = std::chrono::steady_clock::now();
    fps_time_ = (1.0f / (float)fps_) * 1000.0f;

    SetEnabled(true);
}

void ImageLoader::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    if (!IsEnabled())
        SetEnabled(true);

    if (io_mutex_.try_lock()) {  // Try lock so other threads will skip if locked instead of waiting
        if (!frame_.empty()) {
            bool should_wait = true;
            while (should_wait) {
                std::chrono::steady_clock::time_point current_time_ = std::chrono::steady_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
                if (delta >= (uint32_t)fps_time_)
                    should_wait = false;
            }
            outputs.SetValue(0, frame_);
        }
        last_time_ = std::chrono::steady_clock::now();
        io_mutex_.unlock();
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

        if (show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Load Image", GetInstanceName()).c_str());

        if (file_dialog_.showFileDialog(CreateControlString("Load Image", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                ImVec2(700, 310), ".png,.tga,.jpg,.tif", &show_file_dialog_)) {
            std::lock_guard<std::mutex> lk(io_mutex_);
            image_file_ = file_dialog_.selected_path;
            frame_ = cv::imread(image_file_, true);
            show_file_dialog_ = false;
        }
        ImGui::SetNextItemWidth(80);
        const int fpsValues[] = {1, 3, 5, 10, 15, 20, 25, 30, 60, 120};
        if (ImGui::Combo(CreateControlString("Output FPS", GetInstanceName()).c_str(), &fps_index_, " 1\0 3\0 5\0 10\0 15\0 20\0 25\0 30\0 60\0 120\0\0")) {
            fps_ = fpsValues[fps_index_];
            fps_time_ = (1.0f / (float)fps_) * 1000.0f;
        }
    }
}

std::string ImageLoader::GetState()
{
    using namespace nlohmann;

    json state;

    state["image_path"] = image_file_;
    state["fps"] = fps_;
    state["fps_index"] = fps_index_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ImageLoader::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("image_path")) {
        if (!state["image_path"].empty()) {
            std::lock_guard<std::mutex> lk(io_mutex_);
            image_file_ = state["image_path"].get<std::string>();
            frame_ = cv::imread(image_file_, true);
        }
    }
    if (state.contains("fps"))
        fps_ = state["fps"].get<int>();
    if (state.contains("fps_index"))
        fps_index_ = state["fps_index"].get<int>();
}
