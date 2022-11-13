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

    props_.AddOption("blur_mode", "Blur Type", 0, {"Box", "Gaussian", "Median", "Bilateral"});
    props_.AddBool("lock_h_v", "Lock H & V", false);
    props_.AddFloat("blur_amt_h", "Blur Amount H", 1.0f, 1.0f, 100.0f, 0.1f);
    props_.AddFloat("blur_amt_v", "Blur Amount V", 1.0f, 1.0f, 100.0f, 0.1f);

    SetEnabled(true);
}

void Blur::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Blur is multithreaded, it's better to wait and not process more than one call at a time
    std::lock_guard<std::mutex> lck(mutex_lock_);

    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            props_.Sync();

            cv::Mat frame;
            auto bh = props_.Get<float>("blur_amt_h");
            auto bv = props_.Get<float>("blur_amt_v");
            auto bm = props_.Get<int>("blur_mode");
            if (props_.Get<bool>("lock_h_v"))
                bv = bh;
            if (bm == 0) {
                cv::blur(*in1, frame, cv::Size((int)bh, (int)bv), cv::Point(-1, -1));
            } else if (bm == 1) {
                cv::GaussianBlur(*in1, frame, cv::Size(0, 0), bh, bv);
            } else if (bm == 2) {
                int kSize = (int)bh;
                if ((kSize % 2) == 0)
                    kSize++;
                cv::medianBlur(*in1, frame, kSize);
            } else if (bm == 3) {
                bilateralFilter(*in1, frame, (int)bh, bh * 2, bv / 2);
            }
            if (!frame.empty())
                outputs.SetValue(0, frame);
        } else
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
        // Draw properties (in order added)
        props_.DrawUi(GetInstanceName());

        // Additional UI logic
        if (props_.GetW<int>("blur_mode") < 2) {
            props_.SetDescription("blur_amt_h", "Blur Amount H");
            props_.SetDescription("blur_amt_v", "Blur Amount V");
            props_.SetVisibility("lock_h_v", true);
            if (props_.GetW<bool>("lock_h_v"))
                props_.SetVisibility("blur_amt_v", false);
            else
                props_.SetVisibility("blur_amt_v", true);
        } else if (props_.GetW<int>("blur_mode") == 2) {
            props_.SetDescription("blur_amt_h", "Blur Amount");
            props_.SetVisibility("blur_amt_v", false);
            props_.Set("lock_h_v", false);
            props_.SetVisibility("lock_h_v", false);
        } else if (props_.GetW<int>("blur_mode") == 3) {
            props_.SetDescription("blur_amt_h", "Color");
            props_.SetDescription("blur_amt_v", "Space");
            props_.Set("lock_h_v", false);
            props_.SetVisibility("lock_h_v", false);
            props_.SetVisibility("blur_amt_v", true);
        }
        if (props_.GetW<int>("blur_mode") != 2) {
            if (props_.GetW<bool>("lock_h_v"))
                props_.SetVisibility("blur_amt_v", false);
            else
                props_.SetVisibility("blur_amt_v", true);
        }
    }
}

std::string Blur::GetState() {
    using namespace nlohmann;

    json state;

    props_.ToJson(state);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Blur::SetState(std::string &&json_serialized) {
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    props_.FromJson(state);

}

} // End Namespace DSPatch::DSPatchables
