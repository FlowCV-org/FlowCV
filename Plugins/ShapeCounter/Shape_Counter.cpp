//
// Plugin ShapeCounter
//

#include "Shape_Counter.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class ShapeCounter
{
};
}  // namespace DSPatch::DSPatchables::internal

ShapeCounter::ShapeCounter() : Component(ProcessOrder::OutOfOrder), p(new internal::ShapeCounter())
{
    // Name and Category
    SetComponentName_("Shape_Counter");
    SetComponentCategory_(Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "shapes"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 1 outputs
    SetOutputCount_(2, {"out", "counts"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    size_match_ = 100;
    size_threshold_ = 5;
    counting_mode_ = 0;
    show_overlay_ = true;
    text_scale_ = 0.35f;
    text_pos_ = {20, 30};
    text_color_ = {1.0f, 1.0f, 1.0f, 0.8f};
    text_thickness_ = 1;
    text_font_ = 0;

    SetEnabled(true);
}

void ShapeCounter::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<nlohmann::json>(1);
    if (!in1 || !in2) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            nlohmann::json json_data;

            json_data = *in2;
            // Process Image
            in1->copyTo(frame);

            nlohmann::json json_out;
            nlohmann::json data;
            nlohmann::json entry;
            int sizeCnt = 0;
            std::unordered_map<std::string, int> classCount;
            if (counting_mode_ == 0) {
                entry["size"] = size_match_;
                entry["count"] = 0;
            }
            if (!json_data.empty()) {
                if (json_data.contains("data")) {
                    for (auto &d : json_data["data"]) {
                        if (counting_mode_ == 0) {
                            if (d.contains("size")) {
                                int shapeSize = (int)d["size"].get<float>();
                                if (shapeSize > (size_match_ - size_threshold_) && shapeSize < (size_match_ + size_threshold_)) {
                                    sizeCnt++;
                                    entry["count"] = sizeCnt;
                                }
                            }
                        }
                        else if (counting_mode_ == 1) {
                            if (d.contains("class_id")) {
                                std::string classId = d["class_id"].get<std::string>();
                                if (classCount.find(classId) != classCount.end())
                                    classCount[classId]++;
                                else
                                    classCount[classId] = 1;
                            }
                        }
                    }

                    if (counting_mode_ == 0)
                        data.emplace_back(entry);
                    else if (counting_mode_ == 1) {
                        for (auto &cnt : classCount) {
                            nlohmann::json classEntry;
                            classEntry["class_id"] = cnt.first;
                            classEntry["count"] = cnt.second;
                            data.emplace_back(classEntry);
                        }
                    }

                    // Add Frame Overlays
                    if (show_overlay_) {
                        if (counting_mode_ == 0) {  // By Size
                            int baseline = 0;
                            int txtOffset = 12;
                            std::string countStr = "Count: ";
                            countStr += std::to_string(data.at(0)["count"].get<int>());
                            std::string sizeStr = "Size: ";
                            sizeStr += std::to_string(data.at(0)["size"].get<int>());

                            cv::Size txtSize = cv::getTextSize(countStr, text_font_, text_scale_, text_thickness_, &baseline);
                            int roiHeight = ((txtSize.height + txtOffset) * 2) + 10;
                            int txtPosHeightOffset = (int)txtSize.height + 10;
                            cv::Rect roiRect = cv::Rect(text_pos_.x - 10, text_pos_.y - txtPosHeightOffset, txtSize.width + 20, roiHeight);
                            if (roiRect.x < 0) {
                                roiRect.x = 0;
                                text_pos_.x = 10;
                            }
                            if (roiRect.y < 0) {
                                roiRect.y = 0;
                                text_pos_.y = txtPosHeightOffset;
                            }
                            if (roiRect.x + roiRect.width > frame.cols) {
                                roiRect.x = ((frame.cols - roiRect.width) >= 0) ? (frame.cols - roiRect.width) : 0;
                                roiRect.width = (frame.cols - roiRect.x) - 1;
                                text_pos_.x = roiRect.x + 10;
                            }
                            if (roiRect.y + roiRect.height > frame.rows) {
                                roiRect.y = ((frame.rows - roiRect.height) >= 0) ? (frame.rows - roiRect.height) : 0;
                                roiRect.height = (frame.rows - roiRect.y) - 1;
                                text_pos_.y = roiRect.y + txtPosHeightOffset;
                            }
                            cv::Mat roi = frame(roiRect);
                            cv::Mat color(roi.size(), CV_8UC3, cv::Scalar(80, 80, 80));
                            double alpha = text_color_.w;
                            cv::addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
                            cv::rectangle(frame, roiRect, cv::Scalar(40, 40, 40), 1);
                            cv::putText(frame, sizeStr, cv::Point(text_pos_.x, text_pos_.y), text_font_, text_scale_,
                                cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                            cv::putText(frame, countStr, cv::Point(text_pos_.x, text_pos_.y + txtSize.height + txtOffset), text_font_, text_scale_,
                                cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                        }
                        else if (counting_mode_ == 1) {  // By ID
                            if (!classCount.empty()) {
                                int baseline = 0;
                                int txtOffset = 12;
                                size_t longestClass = 0;
                                std::string longestClassStr;
                                for (auto &cnt : classCount) {
                                    if (cnt.first.size() > longestClass) {
                                        longestClass = cnt.first.size();
                                        longestClassStr = cnt.first;
                                    }
                                }
                                std::string placeholder = "Class: ";
                                placeholder += longestClassStr;
                                placeholder += ", Count: 10000";
                                cv::Size txtSize = cv::getTextSize(placeholder, text_font_, text_scale_, text_thickness_, &baseline);
                                int roiHeight = ((txtSize.height + txtOffset) * (int)classCount.size()) + 15;
                                int txtPosHeightOffset = (int)txtSize.height + 10;
                                cv::Rect roiRect = cv::Rect(text_pos_.x - 10, text_pos_.y - txtPosHeightOffset, txtSize.width, roiHeight);
                                if (roiRect.x < 0) {
                                    roiRect.x = 0;
                                    text_pos_.x = 10;
                                }
                                if (roiRect.y < 0) {
                                    roiRect.y = 0;
                                    text_pos_.y = txtPosHeightOffset;
                                }
                                if (roiRect.x + roiRect.width > frame.cols) {
                                    roiRect.x = ((frame.cols - roiRect.width) >= 0) ? (frame.cols - roiRect.width) : 0;
                                    roiRect.width = (frame.cols - roiRect.x) - 1;
                                    text_pos_.x = roiRect.x + 10;
                                }
                                if (roiRect.y + roiRect.height > frame.rows) {
                                    roiRect.y = ((frame.rows - roiRect.height) >= 0) ? (frame.rows - roiRect.height) : 0;
                                    roiRect.height = (frame.rows - roiRect.y) - 1;
                                    text_pos_.y = roiRect.y + txtPosHeightOffset;
                                }

                                cv::Mat roi = frame(roiRect);
                                cv::Mat color(roi.size(), CV_8UC3, cv::Scalar(80, 80, 80));
                                double alpha = text_color_.w;
                                cv::addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
                                cv::rectangle(frame, roiRect, cv::Scalar(40, 40, 40), 1);
                                int startY = text_pos_.y;
                                for (auto &cnt : classCount) {
                                    std::string sizeStr = "Class: ";
                                    sizeStr += cnt.first;
                                    sizeStr += ", Count: ";
                                    sizeStr += std::to_string(cnt.second);
                                    cv::putText(frame, sizeStr, cv::Point(text_pos_.x, startY), text_font_, text_scale_,
                                        cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                                    startY += (txtSize.height + txtOffset);
                                }
                            }
                        }
                    }
                }
            }
            if (!frame.empty()) {
                outputs.SetValue(0, frame);
                json_out["data"] = data;
                outputs.SetValue(1, json_out);
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool ShapeCounter::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ShapeCounter::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Font", GetInstanceName()).c_str(), &text_font_,
            "Simplex\0Plain\0Duplex\0Complex\0Triplex\0Complex Small\0Script Simplex\0Script Complex\0\0");
        ImGui::Separator();
        ImGui::Text("Position:");
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragInt(CreateControlString("X", GetInstanceName()).c_str(), &text_pos_.x, 0.5f)) {
            if (text_pos_.x < 0)
                text_pos_.x = 0;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragInt(CreateControlString("Y", GetInstanceName()).c_str(), &text_pos_.y, 0.5f)) {
            if (text_pos_.y < 0)
                text_pos_.y = 0;
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(CreateControlString("Scale", GetInstanceName()).c_str(), &text_scale_, 0.01f, 0.1f, 100.0f);
        ImGui::Separator();
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.1f, 1, 100);
        ImGui::Separator();
        ImGui::ColorEdit4(CreateControlString("Color", GetInstanceName()).c_str(), (float *)&text_color_);
        ImGui::Separator();
        ImGui::Combo(CreateControlString("Counting Mode", GetInstanceName()).c_str(), &counting_mode_, "By Size\0By ID\0\0");
        ImGui::Checkbox(CreateControlString("Show Overlay", GetInstanceName()).c_str(), &show_overlay_);
        if (counting_mode_ == 0) {
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Size Match", GetInstanceName()).c_str(), &size_match_, 0.5f, 1, 1000);
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Size Threshold", GetInstanceName()).c_str(), &size_threshold_, 0.5f, 0, 1000);
        }
    }
}

std::string ShapeCounter::GetState()
{
    using namespace nlohmann;

    json state;

    state["size_match"] = size_match_;
    state["size_thresh"] = size_threshold_;
    state["count_mode"] = counting_mode_;
    state["show_overlay"] = show_overlay_;
    json pos;
    pos["x"] = text_pos_.x;
    pos["y"] = text_pos_.y;
    state["position"] = pos;
    state["scale"] = text_scale_;
    state["font"] = text_font_;
    state["thickness"] = text_thickness_;
    json tColor;
    tColor["R"] = text_color_.x;
    tColor["G"] = text_color_.y;
    tColor["B"] = text_color_.z;
    tColor["A"] = text_color_.w;
    state["color"] = tColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ShapeCounter::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("size_match"))
        size_match_ = state["size_match"].get<int>();
    if (state.contains("size_thresh"))
        size_threshold_ = state["size_thresh"].get<int>();
    if (state.contains("count_mode"))
        counting_mode_ = state["count_mode"].get<int>();
    if (state.contains("show_overlay"))
        show_overlay_ = state["show_overlay"].get<bool>();
    if (state.contains("color")) {
        text_color_.x = state["color"]["R"].get<float>();
        text_color_.y = state["color"]["G"].get<float>();
        text_color_.z = state["color"]["B"].get<float>();
        text_color_.w = state["color"]["A"].get<float>();
    }
    if (state.contains("position")) {
        text_pos_.x = state["position"]["x"].get<int>();
        text_pos_.y = state["position"]["y"].get<int>();
    }
    if (state.contains("scale"))
        text_scale_ = state["scale"].get<float>();
    if (state.contains("thickness"))
        text_thickness_ = state["thickness"].get<int>();
    if (state.contains("font"))
        text_font_ = state["font"].get<int>();
}
