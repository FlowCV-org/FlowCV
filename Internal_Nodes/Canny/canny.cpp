//
// Plugin Canny
//

#include "canny.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

CannyFilter::CannyFilter() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Canny");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {DSPatch::IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {DSPatch::IoType::Io_Type_CvMat});

    props_.AddInt("kernel_size", "Sobel Apt Size", 3, 3, 7, 2.0f);
    props_.AddOption("thresh_mode", "Threshold Mode", 0, {"Fixed", "Automatic"});
    props_.AddOption("norm_type", "Gradient Mode", 0, {"L1 Norm", "L2 Norm"});
    props_.AddFloat("low_thresh", "Low Threshold", 50.0f, 1.0f, 5000.0f, 0.1f);
    props_.AddFloat("high_thresh", "High Threshold", 150.0f, 1.0f, 5000.0f, 0.1f);

    SetEnabled(true);
}

void CannyFilter::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            props_.Sync();

            // Process Image
            cv::Mat tmp, frame;
            if (in1->channels() > 1)
                cv::cvtColor(*in1, tmp, cv::COLOR_BGR2GRAY);
            else
                tmp = *in1;

            auto low_thresh = props_.Get<float>("low_thresh");
            auto high_thresh = props_.Get<float>("high_thresh");
            if (props_.Get<int>("thresh_mode") == 1) {
                cv::Scalar mean, dev;
                cv::meanStdDev(tmp, mean, dev);
                low_thresh = (float)(mean[0] - dev[0]);
                high_thresh = (float)(mean[0] + dev[0]);
            }
            cv::Canny(tmp, frame, low_thresh, high_thresh, props_.Get<int>("kernel_size"), (bool)props_.Get<int>("norm_type"));
            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool CannyFilter::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void CannyFilter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        props_.DrawUi(GetInstanceName());

        // Show/Hide Low, High depending on mode
        if (props_.Get<int>("thresh_mode") == 0) {
            props_.SetVisibility("low_thresh", true);
            props_.SetVisibility("high_thresh", true);
        }
        else {
            props_.SetVisibility("low_thresh", false);
            props_.SetVisibility("high_thresh", false);
        }

        // Ensure odd number kernel size
        if (props_.Changed("kernel_size")) {
            int ks = props_.GetW<int>("kernel_size");
            if ((ks % 2) == 0) {
                ks++;
                if (ks > props_.GetMax<int>("kernel_size"))
                    ks = props_.GetMax<int>("kernel_size");
            }
            props_.Set("kernel_size", ks);
        }
    }
}

std::string CannyFilter::GetState()
{
    using namespace nlohmann;

    json state;

    props_.ToJson(state);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void CannyFilter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    props_.FromJson(state);
}

}  // End Namespace DSPatch::DSPatchables