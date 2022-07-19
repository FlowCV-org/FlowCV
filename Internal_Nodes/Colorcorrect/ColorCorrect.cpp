//
// Plugin Colorcorrect
//

#include "ColorCorrect.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Colorcorrect::Colorcorrect()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Color_Correct");
    SetComponentCategory_(DSPatch::Category::Category_Color);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_( 2, {"in", "mask"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    brightness_ = 0.0f;
    contrast_ = 1.0f;
    gamma_ = 1.0f;
    gain_ = 1.0f;
    saturation_ = 1.0f;
    color_range_low_.x = 0.0f;
    color_range_low_.y = 0.0f;
    color_range_low_.z = 0.0f;
    color_range_high_.x = 1.0f;
    color_range_high_.y = 1.0f;
    color_range_high_.z = 1.0f;
    UpdateLUT();

    SetEnabled(true);

}

void Colorcorrect::UpdateLUT()
{
    for( int i = 0; i < 256; ++i)
        lut_cache_.at(i) = cv::saturate_cast<uchar>(pow((float)i / 255.0f, 1.0f / gamma_) * 255.0f);
}

void Colorcorrect::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto inMask = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {

            if (in1->channels() >= 3) {
                // Check Mask
                cv::Mat matMask;
                if (!inMask) {
                    matMask = cv::Mat(in1->rows, in1->cols, CV_8UC1, 255);
                } else {
                    if (in1->size() != inMask->size()) {
                        cv::resize(*inMask, matMask, in1->size());
                    } else {
                        inMask->copyTo(matMask);
                    }
                }

                // Process Image
                cv::Mat matFrame = cv::Mat(in1->rows, in1->cols, in1->type());
                float low[3] = {color_range_low_.z, color_range_low_.y, color_range_low_.x};
                float high[3] = {color_range_high_.z, color_range_high_.y, color_range_high_.x};
                for (int y = 0; y < in1->rows; y++) {
                    for (int x = 0; x < in1->cols; x++) {
                        cv::Vec3b rgb = in1->at<cv::Vec3b>(y, x);
                        float lum = ((float)(rgb[0] + rgb[1] + rgb[2]) / 3.0f) / 255.0f;
                        for (int c = 0; c < 3; c++) {
                            float val = (float)in1->at<cv::Vec3b>(y, x)[c] / 255.0f;
                            if (matMask.at<uchar>(y, x) == 255) {

                                if (val >= low[c] && val <= high[c]) {
                                    // Saturation
                                    if (saturation_ > 1.0f || saturation_ < 1.0f)
                                        val = (1.0f - saturation_) * lum + saturation_ * val;

                                    // Gamma
                                    if (gamma_ > 1.0f || gamma_ < 1.0f) {
                                        // Use Cached LUT for faster calculation
                                        uint8_t tmpVal = cv::saturate_cast<uchar>(val * 255.0f);
                                        val = (float) lut_cache_[tmpVal] / 255.0f;
                                    }

                                    // Contrast
                                    if (contrast_ > 1.0f || contrast_ < 1.0f)
                                        val = contrast_ * (val - 0.5f) + 0.5f;

                                    // Gain
                                    if (gain_ > 1.0f || gain_ < 1.0f)
                                        val *= gain_;

                                    // Brightness
                                    if (brightness_ > 0.0f || brightness_ < 0.0f)
                                        val += brightness_;
                                }
                            }
                            matFrame.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(val * 255.0f);
                        }
                    }
                }
                if (!matFrame.empty())
                    outputs.SetValue(0, matFrame);
            }
        } else {
            outputs.SetValue(0, *in1);
        }

    }

}

bool Colorcorrect::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Colorcorrect::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Reset", GetInstanceName()).c_str())) {
            brightness_ = 0.0f;
            contrast_ = 1.0f;
            gamma_ = 1.0f;
            gain_ = 1.0f;
            saturation_ = 1.0f;
            color_range_low_.x = 0.0f;
            color_range_low_.y = 0.0f;
            color_range_low_.z = 0.0f;
            color_range_high_.x = 1.0f;
            color_range_high_.y = 1.0f;
            color_range_high_.z = 1.0f;
            UpdateLUT();
        }
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Low Range", GetInstanceName()).c_str(), (float*)&color_range_low_);
        ImGui::ColorEdit3(CreateControlString("High Range", GetInstanceName()).c_str(), (float*)&color_range_high_);
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Saturation", GetInstanceName()).c_str(), &saturation_, 0.01f, 0.0f, 4.0f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Brightness", GetInstanceName()).c_str(), &brightness_, 0.01f, -3.0f, 3.0f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Contrast", GetInstanceName()).c_str(), &contrast_, 0.01f, 0.0f, 5.0f);
        ImGui::SetNextItemWidth(100);
        if (ImGui::DragFloat(CreateControlString("Gamma", GetInstanceName()).c_str(), &gamma_, 0.01f, 0.0f, 5.0f)){
            UpdateLUT();
        }
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Gain", GetInstanceName()).c_str(), &gain_, 0.01f, 0.0f, 5.0f);
    }

}

std::string Colorcorrect::GetState()
{
    using namespace nlohmann;

    json state;

    state["brightness"] = brightness_;
    state["contrast"] = contrast_;
    state["gamma"] = gamma_;
    state["gain"] = gain_;
    state["saturation"] = saturation_;
    json lowColor;
    lowColor["R"] = color_range_low_.x;
    lowColor["G"] = color_range_low_.y;
    lowColor["B"] = color_range_low_.z;
    state["low_color"] = lowColor;
    json highColor;
    highColor["R"] = color_range_high_.x;
    highColor["G"] = color_range_high_.y;
    highColor["B"] = color_range_high_.z;
    state["high_color"] = highColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Colorcorrect::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("brightness"))
        brightness_ = state["brightness"].get<float>();

    if (state.contains("contrast"))
        contrast_ = state["contrast"].get<float>();

    if (state.contains("gamma"))
        gamma_ = state["gamma"].get<float>();

    if (state.contains("gain"))
        gain_ = state["gain"].get<float>();

    if (state.contains("saturation"))
        saturation_ = state["saturation"].get<float>();

    if (state.contains("low_color")) {
        color_range_low_.x = state["low_color"]["R"].get<float>();
        color_range_low_.y = state["low_color"]["G"].get<float>();
        color_range_low_.z = state["low_color"]["B"].get<float>();
    }
    if (state.contains("high_color")) {
        color_range_high_.x = state["high_color"]["R"].get<float>();
        color_range_high_.y = state["high_color"]["G"].get<float>();
        color_range_high_.z = state["high_color"]["B"].get<float>();
    }

    UpdateLUT();

}


} // End Namespace DSPatch::DSPatchables