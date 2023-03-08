//
// Plugin Mean
//

#include "mean.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Mean::Mean() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Mean");
    SetComponentCategory_(DSPatch::Category::Category_Utility);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"json"}, {IoType::Io_Type_JSON});

    scale_ = 1.0f;
    calc_mean_ = true;
    calc_std_dev_ = false;

    SetEnabled(true);
}

void Mean::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {

            nlohmann::json json_out;
            nlohmann::json data;
            json_out["data_type"] = "mean";
            nlohmann::json ref;
            ref["w"] = in1->cols;
            ref["h"] = in1->rows;
            json_out["ref_frame"] = ref;

            if (calc_std_dev_) {
                cv::Mat mean;
                cv::Mat stdDev;
                cv::meanStdDev(*in1, mean, stdDev);
                mean *= scale_;
                stdDev *= scale_;
                for (int i = 0; i < stdDev.rows; i++) {
                    data["std_dev"].emplace_back((float)stdDev.at<double>(i, 0));
                }
                if (calc_mean_) {
                    for (int i = 0; i < mean.rows; i++) {
                        data["mean"].emplace_back((float)mean.at<double>(i, 0));
                    }
                }
            }

            if (calc_mean_ && !calc_std_dev_) {
                cv::Scalar mn = cv::mean(*in1);
                mn *= scale_;
                for (int i = 0; i < in1->channels(); i++) {
                    data["mean"].emplace_back(mn.val[i]);
                }
            }

            data["scale"] = scale_;
            json_out["data"].emplace_back(data);
            outputs.SetValue(0, json_out);
        }
    }
}

bool Mean::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Mean::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Checkbox(CreateControlString("Calc Mean", GetInstanceName()).c_str(), &calc_mean_);
        ImGui::Checkbox(CreateControlString("Calc Std Dev", GetInstanceName()).c_str(), &calc_std_dev_);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &scale_, 0.001f, 0.001f, 10000.0f);
    }
}

std::string Mean::GetState()
{
    using namespace nlohmann;

    json state;

    state["scale"] = scale_;
    state["calc_mean"] = calc_mean_;
    state["calc_std_dev"] = calc_std_dev_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Mean::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("scale"))
        scale_ = state["scale"].get<float>();
    if (state.contains("calc_mean"))
        calc_mean_ = state["calc_mean"].get<bool>();
    if (state.contains("calc_std_dev"))
        calc_std_dev_ = state["calc_std_dev"].get<bool>();
}

}  // End Namespace DSPatch::DSPatchables