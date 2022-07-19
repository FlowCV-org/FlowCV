//
// Plugin Contours
//

#include "contours.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

Contours::Contours()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Contours");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"viz", "in"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat} );

    // 2 outputs
    SetOutputCount_( 2, {"out", "contours"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    mode_ = 2;
    method_ = 1;
    filter_by_area_ = true;
    area_min_ = 100;
    area_max_ = 1000;
    draw_contours_ = true;
    draw_bbox_ = false;
    fill_contours_ = false;
    contour_color_ = ImVec4(0.0f, 1.0f, 0.0f, 0.0f);
    bbox_color_ = ImVec4(0.0f, 0.0f, 1.0f, 0.0f);

    SetEnabled(true);

}

void Contours::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 ) {
        return;
    }

    auto in2 = inputs.GetValue<cv::Mat>( 0 );

    if (!in1->empty()) {
        if (IsEnabled()) {
            std::vector<std::vector<cv::Point> > contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::Mat frame;

            // Process Image
            if (in1->type() == CV_8UC1 && in1->channels() == 1)
                cv::findContours( *in1, contours, hierarchy, mode_, method_ + 1 );

            int thickness = 2;
            if (fill_contours_)
                thickness = -1;

            if (!in2) {
                frame = cv::Mat(in1->rows, in1->cols, CV_8UC3, cv::Scalar(0, 0, 0));
            }
            else {
                in2->copyTo(frame);
            }

            try {
                nlohmann::json json_out;
                nlohmann::json jContours;
                json_out["data_type"] = "contours";
                nlohmann::json ref;
                ref["w"] = frame.cols;
                ref["h"] = frame.rows;
                json_out["ref_frame"] = ref;
                if (!hierarchy.empty()) {
                    for (int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
                        if (idx < contours.size()) {
                            float area = (float) cv::contourArea(contours[idx]);
                            if (filter_by_area_) {
                                if (area > area_min_ && area < area_max_) {
                                    nlohmann::json jCont;
                                    if (draw_contours_) {
                                        cv::drawContours(frame, contours, idx, cv::Scalar(contour_color_.z * 255, contour_color_.y * 255, contour_color_.x * 255), thickness, cv::LINE_8, hierarchy);
                                        nlohmann::json cTmp1;
                                        for (auto &pt : contours.at(idx)) {
                                            nlohmann::json cTmp2;
                                            cTmp2["x"] = pt.x;
                                            cTmp2["y"] = pt.y;
                                            cTmp1.emplace_back(cTmp2);
                                        }
                                        jCont["contour"] = cTmp1;
                                    }
                                    if (draw_bbox_) {
                                        nlohmann::json cTmp2;
                                        cv::Rect bbox = cv::boundingRect(contours[idx]);
                                        cv::rectangle(frame, bbox, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255), 2);
                                        cTmp2["x"] = bbox.x;
                                        cTmp2["y"] = bbox.y;
                                        cTmp2["w"] = bbox.width;
                                        cTmp2["h"] = bbox.height;
                                        jCont["bbox"] = cTmp2;
                                    }
                                    jContours.emplace_back(jCont);
                                }
                            } else {
                                nlohmann::json jCont;
                                if (draw_contours_) {
                                    cv::drawContours(frame, contours, idx, cv::Scalar(contour_color_.z * 255, contour_color_.y * 255, contour_color_.x * 255), thickness, cv::LINE_8, hierarchy);
                                    nlohmann::json cTmp1;
                                    for (auto &pt : contours.at(idx)) {
                                        nlohmann::json cTmp2;
                                        cTmp2["x"] = pt.x;
                                        cTmp2["y"] = pt.y;
                                        cTmp1.emplace_back(cTmp2);
                                    }
                                    jCont["contour"] = cTmp1;
                                }
                                if (draw_bbox_) {
                                    nlohmann::json cTmp2;
                                    cv::Rect bbox = cv::boundingRect(contours[idx]);
                                    cv::rectangle(frame, bbox, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255), 2);
                                    cTmp2["x"] = bbox.x;
                                    cTmp2["y"] = bbox.y;
                                    cTmp2["w"] = bbox.width;
                                    cTmp2["h"] = bbox.height;
                                    jCont["bbox"] = cTmp2;
                                }
                                jContours.emplace_back(jCont);
                            }
                        }
                    }
                }
                if (!frame.empty()) {
                    outputs.SetValue(0, frame);
                    json_out["data"] = jContours;
                    outputs.SetValue(1, json_out);
                }
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool Contours::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void Contours::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(100);
        ImGui::Combo(CreateControlString("Contour Mode", GetInstanceName()).c_str(), &mode_, "External\0List\0COMP\0Tree\0\0");
        ImGui::SetNextItemWidth(100);
        ImGui::Combo(CreateControlString("Contour Method", GetInstanceName()).c_str(), &method_, "None\0Simple\0TC89_L1\0TC89_KCOS\0\0");
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Filter by Area", GetInstanceName()).c_str(), &filter_by_area_);
        if (filter_by_area_) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Min Area", GetInstanceName()).c_str(), &area_min_, 0.1f, 1, 1000);
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Max Area", GetInstanceName()).c_str(), &area_max_, 0.1f, 1, 5000);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Contours", GetInstanceName()).c_str(), &draw_contours_);
        if (draw_contours_) {
            ImGui::Checkbox(CreateControlString("Fill Contours", GetInstanceName()).c_str(), &fill_contours_);
            ImGui::ColorEdit3(CreateControlString("Contour Color", GetInstanceName()).c_str(), (float*)&contour_color_);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw BBox", GetInstanceName()).c_str(), &draw_bbox_);
        if (draw_bbox_) {
            ImGui::ColorEdit3(CreateControlString("BBox Color", GetInstanceName()).c_str(), (float*)&bbox_color_);
        }
    }

}

std::string Contours::GetState()
{
    using namespace nlohmann;

    json state;

    state["mode"] = mode_;
    state["method"] = method_;
    state["filter_by_area"] = filter_by_area_;
    state["area_min"] = area_min_;
    state["area_max"] = area_max_;
    state["draw_contours"] = draw_contours_;
    state["fill_contours"] = fill_contours_;
    state["draw_bbox"] = draw_bbox_;
    json contColor;
    contColor["R"] = contour_color_.x;
    contColor["G"] = contour_color_.y;
    contColor["B"] = contour_color_.z;
    state["contour_color"] = contColor;
    json bboxColor;
    bboxColor["R"] = bbox_color_.x;
    bboxColor["G"] = bbox_color_.y;
    bboxColor["B"] = bbox_color_.z;
    state["bbox_color"] = bboxColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void Contours::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);
    if (state.contains("contour_color")) {
        contour_color_.x = state["contour_color"]["R"].get<float>();
        contour_color_.y = state["contour_color"]["G"].get<float>();
        contour_color_.z = state["contour_color"]["B"].get<float>();
    }
    if (state.contains("bbox_color")) {
        bbox_color_.x = state["bbox_color"]["R"].get<float>();
        bbox_color_.y = state["bbox_color"]["G"].get<float>();
        bbox_color_.z = state["bbox_color"]["B"].get<float>();
    }
    if (state.contains("mode"))
        mode_ = state["mode"].get<int>();
    if (state.contains("method"))
        method_ = state["method"].get<int>();
    if (state.contains("filter_by_area"))
        filter_by_area_ = state["filter_by_area"].get<bool>();
    if (state.contains("area_min"))
        area_min_ = state["area_min"].get<float>();
    if (state.contains("area_max"))
        area_max_ = state["area_max"].get<float>();
    if (state.contains("draw_contours"))
        draw_contours_ = state["draw_contours"].get<bool>();
    if (state.contains("fill_contours"))
        fill_contours_ = state["fill_contours"].get<bool>();
    if (state.contains("draw_bbox"))
        draw_bbox_ = state["draw_bbox"].get<bool>();

}

} // End Namespace DSPatch::DSPatchables