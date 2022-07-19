//
// Various OpenCV Blur Filters
//

#include "blur.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Blur::Blur()
    : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Blur");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {DSPatch::IoType::Io_Type_CvMat});

    // 1 output
    SetOutputCount_(1, {"out"}, {DSPatch::IoType::Io_Type_CvMat});

    blur_amt_h_ = 1.0f;
    blur_amt_v_ = 1.0f;
    lock_h_v_ = false;
    blur_mode_ = 0;

    SetEnabled(true);
}

void Blur::Process_(SignalBus const &inputs, SignalBus &outputs) {

    if (lock_h_v_)
        blur_amt_v_ = blur_amt_h_;

    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    // Do something with Input
    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            if (blur_mode_ == 0) {
                if ((int)blur_amt_h_ <= 0)
                    blur_amt_h_ = 1.0f;
                if ((int)blur_amt_v_ <= 0)
                    blur_amt_v_ = 1.0f;
                cv::blur( *in1, frame, cv::Size( (int)blur_amt_h_, (int)blur_amt_v_ ), cv::Point(-1,-1) );
            }
            else if (blur_mode_ == 1) {
                cv::GaussianBlur(*in1, frame, cv::Size(0, 0), blur_amt_h_, blur_amt_v_);
            }
            else if (blur_mode_ == 2) {
                int kSize = (int)blur_amt_h_;
                if ((kSize % 2) == 0)
                    kSize++;
                cv::medianBlur(*in1, frame, kSize );
            }
            else if (blur_mode_ == 3) {
                bilateralFilter(*in1, frame, (int)blur_amt_h_, blur_amt_h_*2, blur_amt_v_/2 );
            }
            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else
            outputs.SetValue(0, *in1);


    }

}

bool Blur::HasGui(int interface) {
    if (interface == (int) FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Blur::UpdateGui(void *context, int interface) {
    auto *imCurContext = (ImGuiContext *) context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int) FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(150);
        ImGui::Combo(CreateControlString("Blur Type", GetInstanceName()).c_str(), &blur_mode_, "Box\0Gaussian\0Median\0Bilateral\0\0");
        ImGui::Checkbox(CreateControlString("Lock H & V", GetInstanceName()).c_str(), &lock_h_v_);
        if (blur_mode_ != 3) {
            if (blur_mode_ != 2)
                ImGui::Text("Amt H:");
            else
                ImGui::Text("Amt:");
        }
        else
            ImGui::Text("Color:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat(CreateControlString("H", GetInstanceName()).c_str(), &blur_amt_h_, 0.001f, 100.0f);
        if (blur_mode_ != 2) {
            if (blur_mode_ != 3)
                ImGui::Text("Amt V:");
            else
                ImGui::Text("Space:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat(CreateControlString("V", GetInstanceName()).c_str(), &blur_amt_v_, 0.001f, 100.0f);
        }
    }

}

std::string Blur::GetState() {
    using namespace nlohmann;

    json state;

    state["blur_amt_h"] = blur_amt_h_;
    state["blur_amt_v"] = blur_amt_v_;
    state["blur_mode"] = blur_mode_;
    state["lock_h_v"] = lock_h_v_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Blur::SetState(std::string &&json_serialized) {
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("blur_amt_h")) {
        if (state["blur_amt_h"].is_number_float()) {
            blur_amt_h_ = state["blur_amt_h"].get<float>();
        }
    }
    if (state.contains("blur_amt_v")) {
        if (state["blur_amt_v"].is_number_float()) {
            blur_amt_v_ = state["blur_amt_v"].get<float>();
        }
    }
    if (state.contains("blur_mode")) {
        if (state["blur_mode"].is_number()) {
            blur_mode_ = state["blur_mode"].get<int>();
        }
    }
    if (state.contains("lock_h_v")) {
        if (state["lock_h_v"].is_boolean()) {
            lock_h_v_ = state["lock_h_v"].get<bool>();
        }
    }

}

} // End Namespace DSPatch::DSPatchables
