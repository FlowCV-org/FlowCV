//
// Plugin HoughCircles
//

#include "hough_circles.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

HCircles::HCircles()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Hough_Circles");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"orig", "canny"}, {DSPatch::IoType::Io_Type_CvMat, DSPatch::IoType::Io_Type_CvMat} );

    // 2 outputs
    SetOutputCount_( 2, {"vis", "circles"}, {DSPatch::IoType::Io_Type_CvMat, DSPatch::IoType::Io_Type_JSON} );

    dp_ = 2;
    min_dist_ = 150;
    param1_ = 200;
    param2_ = 100;
    min_radius_ = 0;
    max_radius_ = 0;

    circle_color_ = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    center_color_ = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);;
    text_color_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);;
    fill_circle_ = false;
    show_radius_ = true;
    circle_thickness_ = 2;
    show_center_ = true;
    text_scale_ = 1.0f;
    text_thickness_ = 2;
    text_offset_ = 10;

    SetEnabled(true);

}

void HCircles::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 1 );
    if ( !in1 ) {
        return;
    }
    auto in2 = inputs.GetValue<cv::Mat>( 0 );

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat cannyImg, visImg;

            in1->copyTo(cannyImg);

            // Process Image
            if (cannyImg.type() == CV_8UC3 && cannyImg.channels() == 3) {
                cv::cvtColor(cannyImg, cannyImg, cv::COLOR_BGR2GRAY);
            }
            std::vector<cv::Vec3f> circles;
            cv::HoughCircles(cannyImg, circles, cv::HOUGH_GRADIENT, dp_, min_dist_, param1_, param2_, min_radius_, max_radius_);

            if (in2) {
                if (!in2->empty()) {
                    in2->copyTo(visImg);
                }
                else {
                    visImg = cv::Mat(cannyImg.rows, cannyImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));
                }
            }
            else {
                visImg = cv::Mat(cannyImg.rows, cannyImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));
            }
            nlohmann::json json_out;
            nlohmann::json jCircles;
            json_out["data_type"] = "circles";
            nlohmann::json ref;
            ref["w"] = cannyImg.cols;
            ref["h"] = cannyImg.rows;
            json_out["ref_frame"] = ref;
            for( const auto &c : circles )
            {
                nlohmann::json cTmp;
                cTmp["x"] = c[0];
                cTmp["y"] = c[1];
                cTmp["size"] = c[2] * 2.0f;
                cv::KeyPoint kp;
                jCircles.emplace_back(cTmp);
                cv::Point center(cvRound(c[0]), cvRound(c[1]));
                int radius = cvRound(c[2]);
                // draw the circle outline
                if (fill_circle_)
                    circle( visImg, center, radius, cv::Scalar(circle_color_.z * 255, circle_color_.y * 255, circle_color_.x * 255), -1, 8, 0 );
                else
                    circle( visImg, center, radius, cv::Scalar(circle_color_.z * 255, circle_color_.y * 255, circle_color_.x * 255), circle_thickness_, 8, 0 );

                // draw the circle center
                if (show_center_)
                    circle( visImg, center, 3, cv::Scalar(center_color_.z * 255, center_color_.y * 255, center_color_.x * 255), -1, 8, 0 );

                // draw radius test
                if (show_radius_) {
                    cv::Point textPos;
                    textPos.x = (center.x - 10) + (int)(sin((float)text_offset_ / 50.0f) * 20.0f);
                    textPos.y = (center.y + 6) - (int)(cos((float)text_offset_ / 50.0f) * 20.0f);
                    cv::putText(visImg, std::to_string(radius), textPos, cv::FONT_HERSHEY_SIMPLEX, text_scale_,
                                cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);
                }
            }
            if (!visImg.empty()) {
                outputs.SetValue(0, visImg);
                json_out["data"] = jCircles;
                outputs.SetValue(1, json_out);
            }

        } else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool HCircles::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void HCircles::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Text("Detection Settings");
        ImGui::Separator();
        ImGui::DragFloat(CreateControlString("Inv Acc Res", GetInstanceName()).c_str(), &dp_, 0.1f, 1.0f, 100.0f, "%.2f");
        ImGui::DragFloat(CreateControlString("Min Dist", GetInstanceName()).c_str(), &min_dist_, 1.0f, 1.0f, 2000.0f, "%.2f");
        ImGui::DragFloat(CreateControlString("Param 1", GetInstanceName()).c_str(), &param1_, 0.1f, 1.0f, 2000.0f, "%.2f");
        ImGui::DragFloat(CreateControlString("Param 2", GetInstanceName()).c_str(), &param2_, 0.1f, 1.0f, 2000.0f, "%.2f");
        ImGui::DragInt(CreateControlString("Min Radius", GetInstanceName()).c_str(), &min_radius_, 0.25f, 0, 1000);
        ImGui::DragInt(CreateControlString("Max Radius", GetInstanceName()).c_str(), &max_radius_, 0.25f, 0, 1000);
        ImGui::Separator();
        ImGui::Text("Visualization Settings");
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Circle Color", GetInstanceName()).c_str(), (float*)&circle_color_);
        ImGui::DragInt(CreateControlString("Circle Thickness", GetInstanceName()).c_str(), &circle_thickness_, 0.1f, 1, 50);
        ImGui::Checkbox(CreateControlString("Fill Circle", GetInstanceName()).c_str(), &fill_circle_);
        ImGui::ColorEdit3(CreateControlString("Center Color", GetInstanceName()).c_str(), (float*)&center_color_);
        ImGui::Checkbox(CreateControlString("Show Center", GetInstanceName()).c_str(), &show_center_);
        ImGui::ColorEdit3(CreateControlString("Text Color", GetInstanceName()).c_str(), (float*)&text_color_);
        ImGui::Checkbox(CreateControlString("Show Radius", GetInstanceName()).c_str(), &show_radius_);
        ImGui::DragInt(CreateControlString("Text Offset", GetInstanceName()).c_str(), &text_offset_, 0.25f, 0, 300);
        ImGui::DragFloat(CreateControlString("Text Scale", GetInstanceName()).c_str(), &text_scale_, 0.01f, 0.1f, 20.0f);
        ImGui::DragInt(CreateControlString("Text Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.25f, 1, 30);
    }

}

std::string HCircles::GetState()
{
    using namespace nlohmann;

    json state;

    state["dp"] = dp_;
    state["min_dist"] = min_dist_;
    state["param1"] = param1_;
    state["param2"] = param2_;
    state["min_radius"] = min_radius_;
    state["max_radius"] = max_radius_;
    state["text_scale"] = text_scale_;
    state["text_thickness"] = text_thickness_;
    state["text_offset"] = text_offset_;
    state["circle_thickness"] = circle_thickness_;
    state["show_radius"] = show_radius_;
    state["show_center"] = show_center_;
    state["fill_circle"] = fill_circle_;
    json circColor;
    circColor["R"] = circle_color_.x;
    circColor["G"] = circle_color_.y;
    circColor["B"] = circle_color_.z;
    state["circle_color"] = circColor;
    json centColor;
    centColor["R"] = center_color_.x;
    centColor["G"] = center_color_.y;
    centColor["B"] = center_color_.z;
    state["center_color"] = centColor;
    json txtColor;
    txtColor["R"] = text_color_.x;
    txtColor["G"] = text_color_.y;
    txtColor["B"] = text_color_.z;
    state["text_color"] = txtColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void HCircles::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("dp")) {
        dp_ = state["dp"].get<float>();
    }
    if (state.contains("min_dist")) {
        min_dist_ = state["min_dist"].get<float>();
    }
    if (state.contains("param1")) {
        param1_ = state["param1"].get<float>();
    }
    if (state.contains("param2")) {
        param2_ = state["param2"].get<float>();
    }
    if (state.contains("min_radius")) {
        min_radius_ = state["min_radius"].get<int>();
    }
    if (state.contains("max_radius")) {
        max_radius_ = state["max_radius"].get<int>();
    }
    if (state.contains("text_scale")) {
        text_scale_ = state["text_scale"].get<float>();
    }
    if (state.contains("text_thickness")) {
        text_thickness_ = state["text_thickness"].get<float>();
    }
    if (state.contains("text_offset")) {
        text_offset_ = state["text_offset"].get<float>();
    }
    if (state.contains("circle_thickness")) {
        circle_thickness_ = state["circle_thickness"].get<float>();
    }
    if (state.contains("show_radius")) {
        show_radius_ = state["show_radius"].get<bool>();
    }
    if (state.contains("show_center")) {
        show_center_ = state["show_center"].get<bool>();
    }
    if (state.contains("fill_circle")) {
        fill_circle_ = state["fill_circle"].get<bool>();
    }
    if (state.contains("circle_color")) {
        circle_color_.x = state["circle_color"]["R"].get<float>();
        circle_color_.y = state["circle_color"]["G"].get<float>();
        circle_color_.z = state["circle_color"]["B"].get<float>();
    }
    if (state.contains("center_color")) {
        center_color_.x = state["center_color"]["R"].get<float>();
        center_color_.y = state["center_color"]["G"].get<float>();
        center_color_.z = state["center_color"]["B"].get<float>();
    }
    if (state.contains("text_color")) {
        text_color_.x = state["text_color"]["R"].get<float>();
        text_color_.y = state["text_color"]["G"].get<float>();
        text_color_.z = state["text_color"]["B"].get<float>();
    }


}

} // End Namespace DSPatch::DSPatchables