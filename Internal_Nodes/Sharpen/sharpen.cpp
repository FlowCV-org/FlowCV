//
// Plugin Sharpen
//

#include "sharpen.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Sharpen::Sharpen()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Sharpen");
    SetComponentCategory_(DSPatch::Category::Category_Filter);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    sharpen_mode_ = 0;
    max_amt_ = 3;

    SetEnabled(true);

}

void Sharpen::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        cv::Mat frame;

        if (IsEnabled()) {
            // Process Image
            if (sharpen_mode_ == 0) {
                cv::Mat kernel3 = cv::Mat_<double>(3,3);
                if (sharpen_amt_ >= 1) {
                    if (sharpen_amt_ == 1) {
                        kernel3.at<double>(0, 0) = 0.0; kernel3.at<double>(0, 1) = -1.0; kernel3.at<double>(0, 2) = 0.0;
                        kernel3.at<double>(1, 0) = -1.0; kernel3.at<double>(1, 1) = 5.0; kernel3.at<double>(1, 2) = -1.0;
                        kernel3.at<double>(2, 0) = 0.0; kernel3.at<double>(2, 1) = -1.0; kernel3.at<double>(2, 2) = 0.0;
                    }
                    if (sharpen_amt_ == 2) {
                        kernel3.at<double>(0, 0) = -1.0; kernel3.at<double>(0, 1) = -1.0; kernel3.at<double>(0, 2) = -1.0;
                        kernel3.at<double>(1, 0) = -1.0; kernel3.at<double>(1, 1) = 9.0; kernel3.at<double>(1, 2) = -1.0;
                        kernel3.at<double>(2, 0) = -1.0; kernel3.at<double>(2, 1) = -1.0; kernel3.at<double>(2, 2) = -1.0;
                    }
                    if (sharpen_amt_ == 3) {
                        kernel3.at<double>(0, 0) = -2.0; kernel3.at<double>(0, 1) = -2.0; kernel3.at<double>(0, 2) = -2.0;
                        kernel3.at<double>(1, 0) = -2.0; kernel3.at<double>(1, 1) = 17.0; kernel3.at<double>(1, 2) = -2.0;
                        kernel3.at<double>(2, 0) = -2.0; kernel3.at<double>(2, 1) = -2.0; kernel3.at<double>(2, 2) = -2.0;
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
                    cv::Laplacian(tmp_frame, tmp_frame, -1, 3, (float) -sharpen_amt_, 0.0, cv::BORDER_DEFAULT);
                }
                else {
                    cv::Laplacian(*in1, tmp_frame, -1, 3, (float) -sharpen_amt_, 0.0, cv::BORDER_DEFAULT);
                }
                cv::Mat to_color;
                cv::cvtColor(tmp_frame, to_color, cv::COLOR_GRAY2BGR);
                cv::addWeighted(*in1, 1.0, to_color, (float)sharpen_amt_, 0.0, frame);
            }
        } else {
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
        ImGui::SetNextItemWidth(120);
        if (ImGui::Combo(CreateControlString("Sharpen Mode", GetInstanceName()).c_str(), &sharpen_mode_, "Mode 1\0Mode 2\0\0")) {
            if (sharpen_mode_ == 0)
                max_amt_ = 3;
            else
                max_amt_ = 5;
            if (sharpen_amt_ > max_amt_)
                sharpen_amt_ = max_amt_;
        }
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputInt(CreateControlString("Sharpen", GetInstanceName()).c_str(), &sharpen_amt_, 1)) {
            if (sharpen_amt_ < 0)
                sharpen_amt_ = 0;
            else if (sharpen_amt_ > max_amt_)
                sharpen_amt_ = max_amt_;
        }
    }
}

std::string Sharpen::GetState()
{
    using namespace nlohmann;

    json state;

    state["sharpen_mode"] = sharpen_mode_;
    state["sharpen_amt"] = sharpen_amt_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Sharpen::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("sharpen_mode"))
        sharpen_mode_ = state["sharpen_mode"].get<int>();
    if (state.contains("sharpen_amt"))
        sharpen_amt_ = state["sharpen_amt"].get<int>();

}

} // End Namespace DSPatch::DSPatchables