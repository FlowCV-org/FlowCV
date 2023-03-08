//
// Plugin Solid
//

#include "solid.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Solid::Solid() : Component(ProcessOrder::InOrder)
{
    // Name and Category
    SetComponentName_("Solid");
    SetComponentCategory_(DSPatch::Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 0 inputs
    SetInputCount_(0);

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    width_ = 640;
    height_ = 480;
    color_ = {0.0f, 0.0f, 0.0f, 1.0f};
    is_color_ = true;
    has_alpha_ = true;
    grey_value_ = 0;
    fps_ = 30;
    fps_index_ = 7;
    last_time_ = std::chrono::steady_clock::now();
    fps_time_ = (1.0f / (float)fps_) * 1000.0f;

    SetEnabled(true);
}

void Solid::Process_(SignalBus const &inputs, SignalBus &outputs)
{

    if (io_mutex_.try_lock()) {  // Try lock so other threads will skip if locked instead of waiting
        bool should_wait = true;
        while (should_wait) {
            std::chrono::steady_clock::time_point current_time_ = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_time_).count();
            if (delta >= (uint32_t)fps_time_)
                should_wait = false;
        }
        cv::Mat frame;
        if (is_color_) {
            if (has_alpha_)
                frame = cv::Mat(height_, width_, CV_8UC4, cv::Scalar(color_.z * 255, color_.y * 255, color_.x * 255, color_.w * 255));
            else
                frame = cv::Mat(height_, width_, CV_8UC3, cv::Scalar(color_.z * 255, color_.y * 255, color_.x * 255));
        }
        else {
            frame = cv::Mat(height_, width_, CV_8UC1, cv::Scalar(grey_value_));
        }
        outputs.SetValue(0, frame);
        last_time_ = std::chrono::steady_clock::now();

        io_mutex_.unlock();
    }
}

bool Solid::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Solid::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::Checkbox(CreateControlString("Color", GetInstanceName()).c_str(), &is_color_);
        if (is_color_) {
            ImGui::SetNextItemWidth(100);
            ImGui::Checkbox(CreateControlString("Has Alpha", GetInstanceName()).c_str(), &has_alpha_);
            // ImGui::SetNextItemWidth(300);
            if (has_alpha_)
                ImGui::ColorEdit4(CreateControlString("Solid Color", GetInstanceName()).c_str(), (float *)&color_);
            else
                ImGui::ColorEdit3(CreateControlString("Solid Color", GetInstanceName()).c_str(), (float *)&color_);
        }
        else {
            // ImGui::SetNextItemWidth(100);
            ImGui::SliderInt(CreateControlString("Solid Value", GetInstanceName()).c_str(), &grey_value_, 0, 255);
        }
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &width_, 0.5, 1, 10000)) {
            if (width_ < 2)
                width_ = 2;
        }
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &height_, 0.5, 1, 10000)) {
            if (height_ < 2)
                height_ = 2;
        }
        ImGui::SetNextItemWidth(80);
        const int fpsValues[] = {1, 3, 5, 10, 15, 20, 25, 30, 60, 120};
        if (ImGui::Combo(CreateControlString("Output FPS", GetInstanceName()).c_str(), &fps_index_, " 1\0 3\0 5\0 10\0 15\0 20\0 25\0 30\0 60\0 120\0\0")) {
            fps_ = fpsValues[fps_index_];
            fps_time_ = (1.0f / (float)fps_) * 1000.0f;
        }
    }
}

std::string Solid::GetState()
{
    using namespace nlohmann;

    json state;

    state["width"] = width_;
    state["height"] = height_;
    json solidColor;
    solidColor["R"] = color_.x;
    solidColor["G"] = color_.y;
    solidColor["B"] = color_.z;
    solidColor["A"] = color_.w;
    state["solid_color"] = solidColor;
    state["value"] = grey_value_;
    state["fps"] = fps_;
    state["fps_index"] = fps_index_;
    state["is_color"] = is_color_;
    state["has_alpha"] = has_alpha_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Solid::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("width"))
        width_ = state["width"].get<int>();
    if (state.contains("height"))
        height_ = state["height"].get<int>();
    if (state.contains("solid_color")) {
        color_.x = state["solid_color"]["R"].get<float>();
        color_.y = state["solid_color"]["G"].get<float>();
        color_.z = state["solid_color"]["B"].get<float>();
        color_.w = state["solid_color"]["A"].get<float>();
    }
    if (state.contains("value"))
        grey_value_ = state["value"].get<int>();
    if (state.contains("fps"))
        fps_ = state["fps"].get<int>();
    if (state.contains("fps_index"))
        fps_index_ = state["fps_index"].get<int>();
    if (state.contains("is_color"))
        is_color_ = state["is_color"].get<int>();
    if (state.contains("has_alpha"))
        has_alpha_ = state["has_alpha"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables