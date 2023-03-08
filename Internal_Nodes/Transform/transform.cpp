//
// Plugin Transform
//

#include "transform.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Transform::Transform() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Transform");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {DSPatch::IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {DSPatch::IoType::Io_Type_CvMat});

    // Add Node Properties
    props_.AddInt("trans_x", "Translate X", 0, -4000, 4000, 0.5f);
    props_.AddInt("trans_y", "Translate Y", 0, -4000, 4000, 0.5f);
    props_.AddInt("res_x", "Frame Res X", 640, 0, 4000, 1.0f, false);
    props_.AddInt("res_y", "Frame Res Y", 480, 0, 4000, 1.0f, false);
    props_.AddFloat("aspect_ratio", "Aspect Ratio", 1.33333f, 0.0f, 100.0f, 0.1f, false);
    props_.AddOption("flip_mode", "Flip", 0, {"None", "Horizontal", "Vertical", "Both"});
    props_.AddOption("rot_mode", "Rotate", 0, {"0°", "90° CW", "90° CCW", "180°", "Free"});
    props_.AddFloat("angle", "Angle", 0.0f, -180.0f, 180.0f, 0.01f);
    props_.AddOption("scale_mode", "Scale Mode", 0, {"Percentage", "Pixels"});
    props_.AddFloat("scale_x", "Width", 100.0f, 2.0f, 1000.0f, 0.1f);
    props_.AddFloat("scale_y", "Height", 100.0f, 2.0f, 1000.0f, 0.1f);
    props_.AddOption("interp", "Interpolation", 0, {"Nearest", "Bilinear", "BiCubic", "Area", "Lanczos", "Bilinear Exact"});
    props_.AddOption("aspect_mode", "Aspect Mode", 0, {"Free", "Lock Width", "Lock Height"}, false);

    // Enable Node
    SetEnabled(true);
}

void Transform::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            // Thread safe sync properties from UI
            props_.Sync();

            // Process Image
            cv::Mat frame_;
            in1->copyTo(frame_);
            props_.Set("res_x", frame_.cols);
            props_.Set("res_y", frame_.rows);
            props_.Set("aspect_ratio", (float)frame_.rows / (float)frame_.cols);

            // Flip
            switch (props_.Get<int>("flip_mode")) {
                case 1:  // Horizontal
                    cv::flip(frame_, frame_, 0);
                    break;
                case 2:  // Vertical
                    cv::flip(frame_, frame_, 1);
                    break;
                case 3:  // Both
                    cv::flip(frame_, frame_, -1);
                    break;
            }

            // Rotate
            switch (props_.Get<int>("rot_mode")) {
                case 1:
                    cv::rotate(frame_, frame_, cv::ROTATE_90_CLOCKWISE);
                    break;
                case 2:
                    cv::rotate(frame_, frame_, cv::ROTATE_90_COUNTERCLOCKWISE);
                    break;
                case 3:
                    cv::rotate(frame_, frame_, cv::ROTATE_180);
                    break;
                case 4:
                    auto ang = props_.Get<float>("angle");
                    if (ang > 0 || ang < 0) {
                        cv::Point2f center((float)(frame_.cols - 1) / 2.0f, (float)(frame_.rows - 1) / 2.0f);
                        cv::Mat rotation_matix = getRotationMatrix2D(center, ang, 1.0);
                        warpAffine(frame_, frame_, rotation_matix, frame_.size());
                    }
            }

            // Translate
            cv::Point2f trans((float)props_.Get<int>("trans_x"), (float)props_.Get<int>("trans_y"));
            if (trans.x > 0 || trans.x < 0 || trans.y > 0 || trans.y < 0) {
                cv::Mat trans_mat = (cv::Mat_<double>(2, 3) << 1, 0, trans.x, 0, 1, trans.y);
                cv::warpAffine(frame_, frame_, trans_mat, frame_.size());
            }

            // Scale
            bool applyScale = false;
            cv::Point2f scale(props_.Get<float>("scale_x"), props_.Get<float>("scale_y"));
            cv::Point2f scaleVal;
            if (props_.Get<int>("scale_mode") == 0) {
                scaleVal.x = (float)frame_.cols * (scale.x / 100.0f);
                scaleVal.y = (float)frame_.rows * (scale.y / 100.0f);

                if (scale.x > 100.0f || scale.x < 100.0f)
                    applyScale = true;

                if (scale.y > 100.0f || scale.y < 100.0f)
                    applyScale = true;
            }
            else {
                scaleVal.x = scale.x;
                scaleVal.y = scale.y;

                if (scale.x > (float)frame_.cols || scale.x < (float)frame_.cols)
                    applyScale = true;

                if (scale.y > (float)frame_.rows || scale.y < (float)frame_.rows)
                    applyScale = true;
            }

            if (applyScale) {
                switch (props_.Get<int>("interp")) {
                    case 0:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_NEAREST);
                        break;
                    case 1:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LINEAR);
                        break;
                    case 2:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_CUBIC);
                        break;
                    case 3:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_AREA);
                        break;
                    case 4:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LANCZOS4);
                        break;
                    case 5:
                        cv::resize(frame_, frame_, cv::Size((int)scaleVal.x, (int)scaleVal.y), 0, 0, cv::INTER_LINEAR_EXACT);
                        break;
                }
            }
            if (!frame_.empty())
                outputs.SetValue(0, frame_);
        }
        else {
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
        // Draw Properties (will be drawn in order added)
        props_.DrawUi(GetInstanceName());

        // Additional UI Logic
        if (props_.GetW<int>("rot_mode") == 4)
            props_.SetVisibility("angle", true);
        else
            props_.SetVisibility("angle", false);

        if (props_.Changed("scale_mode")) {
            if (props_.GetW<int>("scale_mode") == 0) {
                props_.Set("scale_x", 100.0f);
                props_.Set("scale_y", 100.0f);
                props_.SetMax("scale_x", 1000.0f);
                props_.SetStep("scale_x", 0.1f);
                props_.SetMax("scale_y", 1000.0f);
                props_.SetStep("scale_y", 0.1f);
                props_.Set("aspect_mode", 0);
                props_.SetVisibility("aspect_mode", false);
            }
            else {
                props_.Set("scale_x", (float)props_.GetW<int>("res_x"));
                props_.Set("scale_y", (float)props_.GetW<int>("res_y"));
                props_.SetMax("scale_x", 8000.0f);
                props_.SetStep("scale_x", 1.0f);
                props_.SetMax("scale_y", 8000.0f);
                props_.SetStep("scale_y", 1.0f);
                props_.SetVisibility("aspect_mode", true);
            }
        }
        if (props_.GetW<int>("aspect_mode") == 1) {
            props_.Set("scale_y", props_.GetW<float>("scale_x") * props_.GetW<float>("aspect_ratio"));
        }
        else if (props_.GetW<int>("aspect_mode") == 2) {
            if (props_.GetW<float>("aspect_ratio") > 0.0f) {
                props_.Set("scale_x", props_.GetW<float>("scale_y") / props_.GetW<float>("aspect_ratio"));
            }
        }
    }
}

std::string Transform::GetState()
{
    using namespace nlohmann;

    json state;

    props_.ToJson(state);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Transform::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    props_.FromJson(state);

    if (props_.Get<int>("scale_mode") == 0) {
        props_.SetMax("scale_x", 1000.0f);
        props_.SetStep("scale_x", 0.1f);
        props_.SetMax("scale_y", 1000.0f);
        props_.SetStep("scale_y", 0.1f);
        props_.SetVisibility("aspect_mode", false);
    }
    else {
        props_.SetMax("scale_x", 8000.0f);
        props_.SetStep("scale_x", 1.0f);
        props_.SetMax("scale_y", 8000.0f);
        props_.SetStep("scale_y", 1.0f);
        props_.SetVisibility("aspect_mode", true);
    }
}

}  // End Namespace DSPatch::DSPatchables