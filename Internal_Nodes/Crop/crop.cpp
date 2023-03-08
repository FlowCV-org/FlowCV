//
// Plugin Crop
//

#include "crop.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Crop::Crop() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Crop");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "bbox"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    crop_area_.x = 0;
    crop_area_.y = 0;
    crop_area_.width = 100;
    crop_area_.height = 100;
    has_json_data_ = false;
    calc_total_bbox_ = false;
    json_bbox_index_ = 0;
    adjust_size_x_ = 0;
    adjust_size_y_ = 0;

    SetEnabled(true);
}

void Crop::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<nlohmann::json>(1);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            nlohmann::json json_data;
            in1->copyTo(frame);

            if (in2) {
                json_data = *in2;
                if (!json_data.empty()) {
                    if (json_data.contains("data")) {
                        has_json_data_ = true;

                        if (calc_total_bbox_) {
                            cv::Rect totalRect = cv::Rect(frame.cols, frame.rows, 0, 0);
                            for (auto &d : json_data["data"]) {
                                if (d.contains("bbox")) {
                                    cv::Rect tmpRect;
                                    tmpRect.x = d["bbox"]["x"].get<int>();
                                    tmpRect.y = d["bbox"]["y"].get<int>();
                                    tmpRect.width = d["bbox"]["w"].get<int>();
                                    tmpRect.height = d["bbox"]["h"].get<int>();
                                    if (tmpRect.x < totalRect.x)
                                        totalRect.x = tmpRect.x;
                                    if (tmpRect.y < totalRect.y)
                                        totalRect.y = tmpRect.y;
                                    if ((tmpRect.x + tmpRect.width) > totalRect.width)
                                        totalRect.width = tmpRect.x + tmpRect.width;
                                    if ((tmpRect.y + tmpRect.height) > totalRect.height)
                                        totalRect.height = tmpRect.y + tmpRect.height;
                                }
                            }
                            if (totalRect.x < totalRect.width && totalRect.y < totalRect.height) {
                                crop_area_.x = totalRect.x - (adjust_size_x_ / 2);
                                crop_area_.y = totalRect.y - (adjust_size_y_ / 2);
                                crop_area_.width = (totalRect.width - crop_area_.x) + adjust_size_x_;
                                crop_area_.height = (totalRect.height - crop_area_.y) + adjust_size_y_;
                            }
                        }
                        else {
                            if (json_bbox_index_ < json_data["data"].size()) {
                                if (json_data["data"].at(json_bbox_index_).contains("bbox")) {
                                    crop_area_.x = json_data["data"].at(json_bbox_index_)["bbox"]["x"].get<int>();
                                    crop_area_.y = json_data["data"].at(json_bbox_index_)["bbox"]["y"].get<int>();
                                    crop_area_.width = json_data["data"].at(json_bbox_index_)["bbox"]["w"].get<int>();
                                    crop_area_.height = json_data["data"].at(json_bbox_index_)["bbox"]["h"].get<int>();
                                    crop_area_.x -= (adjust_size_x_ / 2);
                                    crop_area_.y -= (adjust_size_y_ / 2);
                                    crop_area_.width += adjust_size_x_;
                                    crop_area_.height += adjust_size_y_;
                                }
                            }
                        }
                    }
                    else
                        has_json_data_ = false;
                }
                else
                    has_json_data_ = false;
            }
            else
                has_json_data_ = false;

            // Process Image
            if (crop_area_.x < 0)
                crop_area_.x = 0;
            if (crop_area_.x > frame.cols - 1)
                crop_area_.x = frame.cols - 1;
            if (crop_area_.y < 0)
                crop_area_.y = 0;
            if (crop_area_.y > frame.rows - 1)
                crop_area_.y = frame.rows - 1;

            if (crop_area_.height < 1)
                crop_area_.height = 1;
            if (crop_area_.width < 1)
                crop_area_.width = 1;

            if (crop_area_.x + crop_area_.width > frame.cols)
                crop_area_.width = (frame.cols - crop_area_.x);

            if (crop_area_.y + crop_area_.height > frame.rows)
                crop_area_.height = (frame.rows - crop_area_.y);

            cv::Mat crop_frame = frame(crop_area_);
            if (!crop_frame.empty())
                outputs.SetValue(0, crop_frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Crop::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Crop::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!has_json_data_) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Start X", GetInstanceName()).c_str(), &crop_area_.x, 0.5f, 0, 1000);
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Start Y", GetInstanceName()).c_str(), &crop_area_.y, 0.5f, 0, 1000);
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &crop_area_.width, 0.5f, 0, 8000);
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &crop_area_.height, 0.5f, 0, 8000);
        }
        else {
            ImGui::Checkbox(CreateControlString("Calculate Total BBOX from All", GetInstanceName()).c_str(), &calc_total_bbox_);
            if (!calc_total_bbox_) {
                ImGui::SetNextItemWidth(100);
                ImGui::DragInt(CreateControlString("BBOX Index", GetInstanceName()).c_str(), &json_bbox_index_, 0.25f, 0, 2000);
                if (json_bbox_index_ < 0)
                    json_bbox_index_ = 0;
            }
            ImGui::Separator();
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Padding X", GetInstanceName()).c_str(), &adjust_size_x_, 0.25f);
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Padding Y", GetInstanceName()).c_str(), &adjust_size_y_, 0.25f);
        }
    }
}

std::string Crop::GetState()
{
    using namespace nlohmann;

    json state;

    json bboxRect;
    bboxRect["X"] = crop_area_.x;
    bboxRect["Y"] = crop_area_.y;
    bboxRect["W"] = crop_area_.width;
    bboxRect["H"] = crop_area_.height;
    state["bbox"] = bboxRect;
    state["calc_total_bbox"] = calc_total_bbox_;
    state["json_bbox_index"] = json_bbox_index_;
    state["adjust_size_x"] = adjust_size_x_;
    state["adjust_size_y"] = adjust_size_y_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Crop::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("bbox")) {
        crop_area_.x = state["bbox"]["X"].get<int>();
        crop_area_.y = state["bbox"]["Y"].get<int>();
        crop_area_.width = state["bbox"]["W"].get<int>();
        crop_area_.height = state["bbox"]["H"].get<int>();
    }
    if (state.contains("calc_total_bbox"))
        calc_total_bbox_ = state["calc_total_bbox"].get<bool>();
    if (state.contains("json_bbox_index"))
        json_bbox_index_ = state["json_bbox_index"].get<int>();
    if (state.contains("adjust_size_x"))
        adjust_size_x_ = state["adjust_size_x"].get<int>();
    if (state.contains("adjust_size_y"))
        adjust_size_y_ = state["adjust_size_y"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables