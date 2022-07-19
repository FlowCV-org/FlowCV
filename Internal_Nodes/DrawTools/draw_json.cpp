//
// Plugin DrawJSON
//

#include "draw_json.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

DrawJson::DrawJson()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Draw_JSON");
    SetComponentCategory_(DSPatch::Category::Category_Draw);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"in", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    // 1 outputs
    SetOutputCount_( 1, {"out"}, {IoType::Io_Type_CvMat} );

    text_scale_ = 1.0f;
    text_pos_ = {10, 30};
    text_color_ = {1.0f, 1.0f, 1.0f, 1.0f};
    text_thickness_ = 2;
    text_font_ = 0;
    json_data_index_ = 0;
    float_draw_precision_ = 3;

    SetEnabled(true);

}

std::string DrawJson::JsonTreeToStringList_(const nlohmann::json::iterator &it, std::string &curDepth)
{
    std::string outStr;
    if (it->is_object()) {
        outStr = curDepth;
        outStr += it.key();
        outStr += "/";
        // Recursively Iterate Down Until Finding Last Level
        for (nlohmann::json::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
            JsonTreeToStringList_(it2, outStr);
        }
    }
    else {
        // Last Level Add to List
        outStr += it.key();
        std::string newPath = curDepth;
        newPath += outStr;
        json_tree_list_.emplace_back(newPath);
    }
    return outStr;
}

bool DrawJson::IsInList_(const std::string &key_path)
{
    for (const auto &item : json_out_list_) {
        if (item.out_key_path == key_path)
            return true;
    }
    return false;
}

void DrawJson::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    auto in2 = inputs.GetValue<nlohmann::json>( 1 );
    if ( !in1 ) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            in1->copyTo(frame);
            cv::Point2i start_pos = text_pos_;
            int txt_offset = 14;
            if (in2) {
                if (!in2->empty()) {
                    std::lock_guard<std::mutex> lck(io_mutex_);
                    json_data_ = *in2;
                    if (json_data_.contains("data")) {
                        if (!json_data_["data"].empty()) {
                            if (json_data_index_ < json_data_["data"].size()) {
                                for (const auto &item : json_out_list_) {
                                    nlohmann::json::json_pointer jPtr(item.out_key_path);
                                    if (json_data_["data"].at(json_data_index_).contains(jPtr)) {
                                        auto j = json_data_["data"].at(json_data_index_).at(jPtr);
                                        std::string outStr = item.out_key_path;

                                        if (j.is_array()) {
                                            outStr += " [";
                                            outStr += std::to_string(item.out_item_index);
                                            outStr += "]: ";
                                            if (item.out_item_index < j.size()) {
                                                if (j.at(item.out_item_index).is_number_float()) {
                                                    std::ostringstream out;
                                                    out.precision(float_draw_precision_);
                                                    out << std::fixed << j.at(item.out_item_index).get<float>();
                                                    outStr += out.str();
                                                } else {
                                                    outStr += to_string(j.at(item.out_item_index));
                                                }
                                            }
                                        }
                                        else {
                                            outStr += ": ";
                                            if (j.is_number_float()) {
                                                std::ostringstream out;
                                                out.precision(float_draw_precision_);
                                                out << std::fixed << j.get<float>();
                                                outStr += out.str();
                                            } else {
                                                outStr += to_string(j);
                                            }
                                        }

                                        cv::putText(frame, outStr, start_pos, text_font_, text_scale_,
                                                    cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                                        int baseline = 0;
                                        cv::Size txtSize = cv::getTextSize(outStr, text_font_, text_scale_, text_thickness_, &baseline);
                                        start_pos.y += txtSize.height + txt_offset;
                                    }
                                }
                            }
                        }
                    }
                    if (!frame.empty())
                        outputs.SetValue(0, frame);
                }
            }
        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool DrawJson::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void DrawJson::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceName()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        std::lock_guard<std::mutex> lck (io_mutex_);
        ImGui::Combo(CreateControlString("Font", GetInstanceName()).c_str(), &text_font_,
                     "Simplex\0Plain\0Duplex\0Complex\0Triplex\0Complex Small\0Script Simplex\0Script Complex\0\0");
        ImGui::Separator();
        ImGui::Text("Position:");
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("X", GetInstanceName()).c_str(), &text_pos_.x, 0.5f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Y", GetInstanceName()).c_str(), &text_pos_.y, 0.5f);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &text_scale_, 0.1f);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.1f, 1, 100);
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Color", GetInstanceName()).c_str(), (float*)&text_color_);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Decimal Places", GetInstanceName()).c_str(), &float_draw_precision_, 0.5f, 1, 10);
        ImGui::Separator();
        if (!json_data_.empty()) {
            if (json_data_.contains("data")) {
                if (!json_data_["data"].empty()) {
                    uint32_t dataSize = json_data_["data"].size();
                    ImGui::SetNextItemWidth(100);
                    if (ImGui::InputInt(CreateControlString("Data Index", GetInstanceName()).c_str(), &json_data_index_, 1, 10)) {
                        if (json_data_index_ < 0)
                            json_data_index_ = 0;
                        if (json_data_index_ > dataSize - 1)
                            json_data_index_ = (int)dataSize - 1;
                    }
                    ImGui::Text("JSON Available Fields:");
                    ImGui::SameLine();
                    HelpMarker("Double Click to Add Item to Draw List");
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 57, 77, 100));
                    // Create Tree List
                    json_tree_list_.clear();
                    for (nlohmann::json::iterator it = json_data_["data"].at(json_data_index_).begin(); it != json_data_["data"].at(json_data_index_).end(); ++it) {
                        std::string curDepth = "/";
                        std::string strPath = JsonTreeToStringList_(it, curDepth);
                    }
                    int dataHeight = 22 * json_tree_list_.size();
                    if (dataHeight < 66)
                        dataHeight = 66;
                    if (dataHeight > 200)
                        dataHeight = 200;
                    ImGui::BeginChild(CreateControlString("Data", GetInstanceName()).c_str(), ImVec2(0, dataHeight), true, ImGuiWindowFlags_None);
                    // Create Available List
                    for (const auto &s : json_tree_list_) {
                        nlohmann::json::json_pointer jPtr(s);
                        auto j = json_data_["data"].at(json_data_index_).at(jPtr);
                        if (j.is_array()) {
                            const std::string& label = s;
                            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    JsonOutItem tmp;
                                    tmp.out_key_path = s;
                                    tmp.is_array = true;
                                    json_out_list_.emplace_back(tmp);
                                }
                            }
                        }
                        else if (!j.is_array() && !j.is_object()) {
                            const std::string& label = s;
                            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    if (!IsInList_(s)) {
                                        JsonOutItem tmp;
                                        tmp.out_key_path = s;
                                        tmp.is_array = false;
                                        json_out_list_.emplace_back(tmp);
                                    }
                                }
                            }
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                    ImGui::Separator();
                    ImGui::Text("Draw List:");
                    ImGui::SameLine();
                    HelpMarker("Double Click to Remove Item");
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 77, 50, 100));
                    int outHeight = 22 * json_out_list_.size();
                    if (outHeight < 66)
                        outHeight = 66;
                    if (outHeight > 200)
                        outHeight = 200;
                    ImGui::BeginChild(CreateControlString("List", GetInstanceName()).c_str(), ImVec2(0, outHeight), true, ImGuiWindowFlags_None);
                    // Create Out List
                    for (int i = 0; i < json_out_list_.size(); i++) {
                        std::string label = json_out_list_.at(i).out_key_path;
                        if (json_out_list_.at(i).is_array) {
                            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(180.0f, 0.0f))) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    json_out_list_.erase(json_out_list_.begin() + i);
                                    continue;
                                }
                            }
                            ImGui::SameLine();
                            std::string ctlName = "idx##";
                            ctlName += std::to_string(i);
                            ctlName += GetInstanceName();
                            ImGui::SetNextItemWidth(80);
                            if (ImGui::InputInt(ctlName.c_str(), &json_out_list_.at(i).out_item_index, 1, 10)) {
                                if (json_out_list_.at(i).out_item_index < 0)
                                    json_out_list_.at(i).out_item_index = 0;
                            }
                        }
                        else {
                            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    json_out_list_.erase(json_out_list_.begin() + i);
                                    continue;
                                }
                            }
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "JSON Contains No Data");
            }
        }
    }

}

std::string DrawJson::GetState()
{
    using namespace nlohmann;

    json state;

    json pos;
    pos["x"] = text_pos_.x;
    pos["y"] = text_pos_.y;
    state["position"] = pos;
    state["scale"] = text_scale_;
    state["font"] = text_font_;
    state["thickness"] = text_thickness_;
    state["decimal_places"] = float_draw_precision_;
    json tColor;
    tColor["R"] = text_color_.x;
    tColor["G"] = text_color_.y;
    tColor["B"] = text_color_.z;
    state["color"] = tColor;
    state["json_data_index"] = json_data_index_;
    json outList;
    for (const auto &item : json_out_list_) {
        json listItem;
        listItem["is_array"] = item.is_array;
        if (item.is_array)
            listItem["item_index"] = item.out_item_index;
        listItem["item_path"] = item.out_key_path;
        outList.emplace_back(listItem);
    }
    state["out_list"] = outList;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DrawJson::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("color")) {
        text_color_.x = state["color"]["R"].get<float>();
        text_color_.y = state["color"]["G"].get<float>();
        text_color_.z = state["color"]["B"].get<float>();
    }
    if (state.contains("position")) {
        text_pos_.x = state["position"]["x"].get<int>();
        text_pos_.y = state["position"]["y"].get<int>();
    }
    if (state.contains("scale"))
        text_scale_ = state["scale"].get<float>();
    if (state.contains("thickness"))
        text_thickness_ = state["thickness"].get<int>();
    if (state.contains("decimal_places"))
        float_draw_precision_ = state["decimal_places"].get<int>();
    if (state.contains("font"))
        text_font_ = state["font"].get<int>();
    if (state.contains("json_data_index"))
        json_data_index_ = state["json_data_index"].get<int>();
    if (state.contains("out_list")) {
        json_out_list_.clear();
        for(const auto &item : state["out_list"]) {
            JsonOutItem tmp;
            if (item.contains("is_array"))
                tmp.is_array = item["is_array"].get<bool>();
            if (tmp.is_array) {
                if (item.contains("item_index"))
                    tmp.out_item_index = item["item_index"].get<int>();
            }
            else
                tmp.out_item_index = 0;
            if (item.contains("item_path"))
                tmp.out_key_path = item["item_path"].get<std::string>();
            json_out_list_.emplace_back(tmp);
        }
    }
}

} // End Namespace DSPatch::DSPatchables