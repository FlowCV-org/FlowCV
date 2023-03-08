//
// Plugin Sharpen
//

#include "sharpen.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Sharpen::Sharpen() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Sharpen");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    props_.AddOption("sharpen_mode", "Sharpen Mode", 0, {"Mode 1", "Mode 2"});
    props_.AddInt("sharpen_amt", "Sharpen", 0, 0, 3, 0.25f);

    SetEnabled(true);
}

void Sharpen::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        cv::Mat frame;

        if (IsEnabled()) {
            props_.Sync();

            // Process Image
            int amt = props_.Get<int>("sharpen_amt");
            if (props_.Get<int>("sharpen_mode") == 0) {
                cv::Mat kernel3 = cv::Mat_<double>(3, 3);
                if (amt >= 1) {
                    if (amt == 1) {
                        kernel3.at<double>(0, 0) = 0.0;
                        kernel3.at<double>(0, 1) = -1.0;
                        kernel3.at<double>(0, 2) = 0.0;
                        kernel3.at<double>(1, 0) = -1.0;
                        kernel3.at<double>(1, 1) = 5.0;
                        kernel3.at<double>(1, 2) = -1.0;
                        kernel3.at<double>(2, 0) = 0.0;
                        kernel3.at<double>(2, 1) = -1.0;
                        kernel3.at<double>(2, 2) = 0.0;
                    }
                    if (amt == 2) {
                        kernel3.at<double>(0, 0) = -1.0;
                        kernel3.at<double>(0, 1) = -1.0;
                        kernel3.at<double>(0, 2) = -1.0;
                        kernel3.at<double>(1, 0) = -1.0;
                        kernel3.at<double>(1, 1) = 9.0;
                        kernel3.at<double>(1, 2) = -1.0;
                        kernel3.at<double>(2, 0) = -1.0;
                        kernel3.at<double>(2, 1) = -1.0;
                        kernel3.at<double>(2, 2) = -1.0;
                    }
                    if (amt == 3) {
                        kernel3.at<double>(0, 0) = -2.0;
                        kernel3.at<double>(0, 1) = -2.0;
                        kernel3.at<double>(0, 2) = -2.0;
                        kernel3.at<double>(1, 0) = -2.0;
                        kernel3.at<double>(1, 1) = 17.0;
                        kernel3.at<double>(1, 2) = -2.0;
                        kernel3.at<double>(2, 0) = -2.0;
                        kernel3.at<double>(2, 1) = -2.0;
                        kernel3.at<double>(2, 2) = -2.0;
                    }
                    filter2D(*in1, frame, -1, kernel3, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
                }
                else {
                    in1->copyTo(frame);
                }
            }
            else {
                cv::Mat tmp_frame;
                cv::Mat lapFrame;
                if (in1->channels() > 1) {
                    cv::cvtColor(*in1, tmp_frame, cv::COLOR_BGR2GRAY);
                    cv::Laplacian(tmp_frame, tmp_frame, -1, 3, (float)-amt, 0.0, cv::BORDER_DEFAULT);
                }
                else {
                    cv::Laplacian(*in1, tmp_frame, -1, 3, (float)-amt, 0.0, cv::BORDER_DEFAULT);
                }
                cv::Mat to_color;
                cv::cvtColor(tmp_frame, to_color, cv::COLOR_GRAY2BGR);
                cv::addWeighted(*in1, 1.0, to_color, (float)amt, 0.0, frame);
            }
        }
        else {
            in1->copyTo(frame);
        }

        if (!frame.empty())
            outputs.SetValue(0, frame);
    }
}

bool Sharpen::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Sharpen::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        props_.DrawUi(GetInstanceName());

        if (props_.GetW<int>("sharpen_mode") == 0) {
            props_.SetMax("sharpen_amt", 3);
            if (props_.GetW<int>("sharpen_amt") > props_.GetMax<int>("sharpen_amt"))
                props_.Set("sharpen_amt", props_.GetMax<int>("sharpen_amt"));
        }
        else
            props_.SetMax("sharpen_amt", 5);
    }
}

std::string Sharpen::GetState()
{
    using namespace nlohmann;

    json state;

    props_.ToJson(state);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Sharpen::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    props_.FromJson(state);
}

}  // End Namespace DSPatch::DSPatchables