//
// Plugin DiscreteFourierTransform
//

#include "discrete_fourier_transform.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

DiscreteFourierTransform::DiscreteFourierTransform()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("DFT");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 2 outputs
    SetOutputCount_( 1, {"dft"}, {IoType::Io_Type_CvMat} );

    log_view_ = false;
    shift_dc_center_ = true;
    mode_ = 0;

    SetEnabled(true);

}

void DiscreteFourierTransform::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat dftFrame, frame;

            // Process Image
            if (in1->channels() > 1) {
                cv::cvtColor(*in1, dftFrame, cv::COLOR_BGR2GRAY);
            }
            else {
                in1->copyTo(dftFrame);
            }

            int dft_mode = mode_;

            if (mode_ > 0) {
                if (mode_ == 1)
                    dft_mode = cv::DFT_INVERSE + cv::DFT_SCALE;
                else if (mode_ == 2)
                    dft_mode = cv::DFT_ROWS;
                else if (mode_ == 3)
                    dft_mode = cv::DFT_COMPLEX_OUTPUT;
                else if (mode_ == 4)
                    dft_mode = cv::DFT_REAL_OUTPUT;
                else if (mode_ == 5)
                    dft_mode = cv::DFT_COMPLEX_INPUT;
            }

            cv::Mat planes[] = {cv::Mat_<float>(dftFrame), cv::Mat::zeros(dftFrame.size(), CV_32F)};
            cv::Mat complexI;
            cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
            cv::dft(complexI, complexI, dft_mode);            // this way the result may fit in the source matrix

            // compute the magnitude and switch to logarithmic scale
            // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
            cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
            cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
            cv::Mat magI = planes[0];

            if (shift_dc_center_) {
                // crop the spectrum, if it has an odd number of rows or columns
                magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

                // rearrange the quadrants of Fourier image  so that the origin is at the image center
                int cx = magI.cols / 2;
                int cy = magI.rows / 2;

                cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
                cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
                cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
                cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

                cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
                q0.copyTo(tmp);
                q3.copyTo(q0);
                tmp.copyTo(q3);

                q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
                q2.copyTo(q1);
                tmp.copyTo(q2);
            }

            magI.copyTo(dftFrame);

            if (log_view_) {
                magI += cv::Scalar::all(1);                    // switch to logarithmic scale
                cv::log(magI, magI);

                cv::normalize(magI, magI, 0, 1, cv::NORM_MINMAX);
                magI.convertTo(frame, CV_8UC1, 255.0);
            }
            if (log_view_) {
                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }
            else {
                if (!dftFrame.empty())
                    outputs.SetValue(0, dftFrame);
            }
        }
    }
}

bool DiscreteFourierTransform::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void DiscreteFourierTransform::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Operation", GetInstanceName()).c_str(), &mode_,
                     "Default\0Inverse\0Rows\0Complex\0Real\0Complex In\0\0");
        ImGui::Checkbox(CreateControlString("Shift DC to Center", GetInstanceName()).c_str(), &shift_dc_center_);
        ImGui::Checkbox(CreateControlString("Logarithmic View", GetInstanceName()).c_str(), &log_view_);
    }

}

std::string DiscreteFourierTransform::GetState()
{
    using namespace nlohmann;

    json state;

    state["log_view"] = log_view_;
    state["shift_center"] = shift_dc_center_;
    state["op_mode"] = mode_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DiscreteFourierTransform::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("log_view"))
        log_view_ = state["log_view"].get<bool>();
    if (state.contains("shift_center"))
        shift_dc_center_ = state["shift_center"].get<bool>();
    if (state.contains("op_mode"))
        mode_ = state["op_mode"].get<int>();

}

} // End Namespace DSPatch::DSPatchables