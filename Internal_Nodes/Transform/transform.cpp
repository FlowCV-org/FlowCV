//
// Plugin Transform
//

#include "transform.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Transform::Transform()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Transform");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {DSPatch::IoType::Io_Type_CvMat} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {DSPatch::IoType::Io_Type_CvMat} );

    trans_.x = 0;
    trans_.y = 0;
    flip_mode_ = 0;
    frame_res_.x = 640;
    frame_res_.y = 480;
    aspect_ratio_ = (float)frame_res_.y / (float)frame_res_.x;
    aspect_mode_ = 0;
    rotate_mode_ = 0;
    rotate_amt_ = 0;
    interp_mode_ = 0;
    scale_mode_ = 0;
    scale_.x = 100.0f;
    scale_.y = 100.0f;
    scale_max_.x = 1000.0f;
    scale_max_.y = 0.1f;

    SetEnabled(true);

}

void Transform::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {

        if (IsEnabled()) {
            // Process Image
            cv::Mat frame_;
            in1->copyTo(frame_);
            frame_res_.x = frame_.cols;
            frame_res_.y = frame_.rows;
            aspect_ratio_ = (float) frame_res_.y / (float) frame_res_.x;

            // Flip
            if (flip_mode_ == 1)
                cv::flip(frame_, frame_, 0);
            else if (flip_mode_ == 2)
                cv::flip(frame_, frame_, 1);
            else if (flip_mode_ == 3)
                cv::flip(frame_, frame_, -1);


            // Rotate
            if (rotate_mode_ == 1) {
                cv::rotate(frame_, frame_, cv::ROTATE_90_CLOCKWISE);
            }
            else if (rotate_mode_ == 2) {
                cv::rotate(frame_, frame_, cv::ROTATE_90_COUNTERCLOCKWISE);
            }
            else if (rotate_mode_ == 3) {
                cv::rotate(frame_, frame_, cv::ROTATE_180);
            }
            else if (rotate_mode_ == 4) {
                if (rotate_amt_ > 0 || rotate_amt_ < 0) {
                    cv::Point2f center((float) (frame_.cols - 1) / 2.0f, (float) (frame_.rows - 1) / 2.0f);
                    cv::Mat rotation_matix = getRotationMatrix2D(center, rotate_amt_, 1.0);
                    warpAffine(frame_, frame_, rotation_matix, frame_.size());
                }
            }

            // Translate
            if (trans_.x > 0 || trans_.x < 0 || trans_.y > 0 || trans_.y < 0) {
                cv::Mat trans_mat = (cv::Mat_<double>(2, 3) << 1, 0, trans_.x, 0, 1, trans_.y);
                cv::warpAffine(frame_, frame_, trans_mat, frame_.size());
            }

            // Scale
            bool applyScale = false;
            cv::Point2f scaleVal;
            if (scale_mode_ == 0) {
                scaleVal.x = (float)frame_.cols * (scale_.x / 100.0f);
                scaleVal.y = (float)frame_.rows * (scale_.y / 100.0f);

                if (scale_.x > 100.0f || scale_.x < 100.0f)
                    applyScale = true;

                if (scale_.y > 100.0f || scale_.y < 100.0f)
                    applyScale = true;
            }
            else {
                scaleVal.x = scale_.x;
                scaleVal.y = scale_.y;

                if (scale_.x > (float)frame_.cols || scale_.x < (float)frame_.cols)
                    applyScale = true;

                if (scale_.y > (float)frame_.rows || scale_.y < (float)frame_.rows)
                    applyScale = true;
            }

            if (applyScale) {
                if (interp_mode_ == 0)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_NEAREST);
                else if (interp_mode_ == 1)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LINEAR);
                else if (interp_mode_ == 2)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_CUBIC);
                else if (interp_mode_ == 3)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_AREA);
                else if (interp_mode_ == 4)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LANCZOS4);
                else if (interp_mode_ == 5)
                    cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LINEAR_EXACT);
            }
            if (!frame_.empty())
                outputs.SetValue(0, frame_);

        } else {
            // Copy Original to Output (pass thru)
            outputs.SetValue(0, *in1);
        }
    }
}

bool Transform::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Transform::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Text("Transate:");
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("X", GetInstanceName()).c_str(), &trans_.x, 0.5f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Y", GetInstanceName()).c_str(), &trans_.y, 0.5f);
        ImGui::Combo(CreateControlString("Flip", GetInstanceName()).c_str(), &flip_mode_, "None\0Horizontal\0Vertical\0Both\0\0");
        ImGui::Separator();
        ImGui::Text("Rotate:");
        const char *angles[5] = {"0°", "90° CW", "90° CCW", "180°", "Free"};
        ImGui::Combo(CreateControlString("Amt", GetInstanceName()).c_str(), &rotate_mode_, angles, 5);
        if (rotate_mode_ == 4) {
            ImGui::SetNextItemWidth(200);
            ImGui::DragFloat(CreateControlString("Angle", GetInstanceName()).c_str(), &rotate_amt_, 0.01f, -180.0f, 180.0f, "%.2f°" );
        }
        ImGui::Separator();
        ImGui::Text("Scale:");
        if (ImGui::Combo(CreateControlString("Mode", GetInstanceName()).c_str(), &scale_mode_, "Percentage\0Pixels\0\0")) {
            if (scale_mode_ == 0) {
                scale_.x = 100.0f;
                scale_.y = 100.0f;
                scale_max_.x = 1000.0f;
                scale_max_.y = 0.1f;
                aspect_mode_ = 0;
            }
            else {
                scale_.x = (float)frame_res_.x;
                scale_.y = (float)frame_res_.y;
                scale_max_.x = 8000.0f;
                scale_max_.y = 1.0f;
            }
        }
        ImGui::Combo(CreateControlString("Interpolation", GetInstanceName()).c_str(), &interp_mode_, "Nearest\0Bilinear\0BiCubic\0Area\0Lanczos\0Bilinear Exact\0\0");
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Width", GetInstanceName()).c_str(), &scale_.x, scale_max_.y, 1.0f, scale_max_.x );
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Height", GetInstanceName()).c_str(), &scale_.y, scale_max_.y, 1.0f, scale_max_.x );
        if (scale_mode_ != 0) {
            if (ImGui::RadioButton(CreateControlString("Free", GetInstanceName()).c_str(), aspect_mode_ == 0)) { aspect_mode_ = 0; }
            ImGui::SameLine();
            if (ImGui::RadioButton(CreateControlString("Lock Aspect W", GetInstanceName()).c_str(), aspect_mode_ == 1)) { aspect_mode_ = 1; }
            ImGui::SameLine();
            if (ImGui::RadioButton(CreateControlString("Lock Aspect H", GetInstanceName()).c_str(), aspect_mode_ == 2)) { aspect_mode_ = 2; }
        }
        if (aspect_mode_ == 1) {
            scale_.y = scale_.x * aspect_ratio_;
        }
        else if (aspect_mode_ == 2) {
            if (aspect_ratio_ > 0.0f) {
                scale_.x = scale_.y / aspect_ratio_;
            }
        }
    }
}

std::string Transform::GetState()
{
    using namespace nlohmann;

    json state;

    json trans;
    trans["x"] = trans_.x;
    trans["y"] = trans_.y;
    state["translate"] = trans;
    json scale;
    scale["x"] = scale_.x;
    scale["y"] = scale_.y;
    state["scale"] = scale;
    state["scale_mode"] = scale_mode_;
    state["aspect_mode"] = aspect_mode_;
    state["flip_mode"] = flip_mode_;
    state["interp_mode"] = interp_mode_;
    state["rotate_mode"] = rotate_mode_;
    state["rotate_amt"] = rotate_amt_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Transform::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("translate")) {
        trans_.x = state["translate"]["x"].get<int>();
        trans_.y = state["translate"]["y"].get<int>();
    }
    if (state.contains("scale")) {
        scale_.x = state["scale"]["x"].get<float>();
        scale_.y = state["scale"]["y"].get<float>();
    }
    if (state.contains("aspect_mode")) {
        if (state["aspect_mode"].is_number()) {
            aspect_mode_ = state["aspect_mode"].get<int>();
        }
    }
    if (state.contains("flip_mode")) {
        if (state["flip_mode"].is_number()) {
            flip_mode_ = state["flip_mode"].get<int>();
        }
    }
    if (state.contains("scale_mode")) {
        if (state["scale_mode"].is_number()) {
            scale_mode_ = state["scale_mode"].get<int>();
            if (scale_mode_ == 0) {
                scale_max_.x = 1000.0f;
                scale_max_.y = 0.1f;
            }
            else {
                scale_max_.x = 8000.0f;
                scale_max_.y = 1.0f;
            }
        }
    }
    if (state.contains("interp_mode")) {
        if (state["interp_mode"].is_number()) {
            interp_mode_ = state["interp_mode"].get<int>();
        }
    }
    if (state.contains("rotate_mode")) {
        if (state["rotate_mode"].is_number()) {
            rotate_mode_ = state["rotate_mode"].get<int>();
        }
    }
    if (state.contains("rotate_amt")) {
        if (state["rotate_amt"].is_number()) {
            rotate_amt_ = state["rotate_amt"].get<float>();
        }
    }

}

} // End Namespace DSPatch::DSPatchables