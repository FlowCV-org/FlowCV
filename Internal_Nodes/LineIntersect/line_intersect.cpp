//
// Plugin Line Intersections
//

#include "line_intersect.hpp"

using namespace DSPatch;
using namespace DSPatchables;

#define MAX_LINE_INTERSECT_COUNT 200

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

LineIntersect::LineIntersect()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Line_Intersections");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"in", "lines"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    // 1 outputs
    SetOutputCount_( 2, {"out", "points"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON} );

    min_angle_ = 60;
    min_dist_ = 10;
    intersect_count_ = 0;
    line_count_ = 0;
    point_color_ = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    point_radius_ = 4.0f;
    point_thickness_ = 1.0f;
    draw_intersections_ = true;
    point_solid_ = false;

    SetEnabled(true);

}

static cv::Point2f computeIntersect(const cv::Vec4i& a, const cv::Vec4i& b)
{
    cv::Point2f pInter = {-1, -1};

    float a1 = a[3] - a[1];
    float b1 = a[0] - a[2];
    float c1 = a1 * a[0] + b1 * a[1];

    float a2 = b[3] - b[1];
    float b2 = b[0] - b[2];
    float c2 = a2 * b[0] + b2 * b[1];

    float determinant = a1 * b2 - a2 * b1;

    if (determinant != 0) {
        pInter.x = (b2*c1 - b1*c2)/determinant;
        pInter.y = (a1*c2 - a2*c1)/determinant;
    }

    return pInter;
}

void LineIntersect::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // Input Handler
    auto in_img = inputs.GetValue<cv::Mat>(0);
    auto in_json = inputs.GetValue<nlohmann::json>(1);
    if ( !in_json ) {
        return;
    }

    cv::Mat ref_frame;
    if (in_img) {
        if (!in_img->empty())
            in_img->copyTo(ref_frame);
    }

    if (!in_json->empty()) {
        nlohmann::json json_lines = *in_json;
        nlohmann::json json_points;

        if (IsEnabled()) {
            if (json_lines.contains("data_type")) {
                if(json_lines["data_type"].get<std::string>() != "lines")
                    return;
            }
            if (json_lines.contains("data")) {
                line_count_ = (int)json_lines["data"].size();
                if (line_count_ > MAX_LINE_INTERSECT_COUNT) {
                    return;
                }
                std::vector<cv::Vec4i> lines;
                for (const auto& jLine : json_lines["data"]) {
                    cv::Vec4i vIn;
                    vIn[0] = jLine["line"]["x0"].get<int>();
                    vIn[1] = jLine["line"]["y0"].get<int>();
                    vIn[2] = jLine["line"]["x1"].get<int>();
                    vIn[3] = jLine["line"]["y1"].get<int>();
                    lines.emplace_back(vIn);
                }
                std::vector<cv::Point2f> intersect_points;
                for(auto i = 0u; i < lines.size(); i++) {
                    for (unsigned int j = i+1; j < lines.size(); j++) {
                        cv::Point2f int_pnt = computeIntersect(lines[i], lines[j]);
                        if (int_pnt.x >= 0 && int_pnt.y >= 0) {
                            cv::Vec2f v1 = cv::Vec2i(lines[i][0] - lines[i][2], lines[i][1] - lines[i][3]);
                            v1 = cv::normalize(v1);
                            cv::Vec2f v2 = cv::Vec2i(lines[j][0] - lines[j][2], lines[j][1] - lines[j][3]);
                            v2 = cv::normalize(v2);
                            float d = v1.dot(v2);
                            float angle = acos(d) * 57.2957795130823208f;
                            if (angle > min_angle_) {
                                float closest_dist = 9999999.0f;
                                for (const auto &chk_pnt : intersect_points) {
                                    float dist = (float)cv::norm(int_pnt - chk_pnt);
                                    if (dist < min_dist_) {
                                        closest_dist = dist;
                                    }
                                }
                                if (closest_dist > min_dist_) {
                                    intersect_points.emplace_back(int_pnt);
                                    if (!ref_frame.empty() && draw_intersections_) {
                                        int thickness = point_thickness_;
                                        if (point_solid_)
                                            thickness = -1;
                                        cv::circle(ref_frame, int_pnt, point_radius_,
                                                   cv::Scalar(point_color_.z * 255, point_color_.y * 255, point_color_.x * 255),
                                                   thickness);
                                    }
                                }
                            }
                        }
                    }
                }
                intersect_count_ = (int)intersect_points.size();
                if (intersect_count_ > 0) {
                    nlohmann::json jPoints;
                    for (const auto &pnt : intersect_points) {
                        nlohmann::json jc;
                        jc["x"] = pnt.x;
                        jc["y"] = pnt.y;
                        jPoints.emplace_back(jc);
                    }
                    json_points["data"] = jPoints;
                    json_points["data_type"] = "points";
                    json_points["ref_frame"] = json_lines["ref_frame"];
                }
            }
            if (!json_points.empty())
                outputs.SetValue(1, json_points);
        } else {
            outputs.SetValue(1, json_points);
        }
    }
    if (!ref_frame.empty())
        outputs.SetValue(0, ref_frame);
}

bool LineIntersect::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void LineIntersect::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);


    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (line_count_ > MAX_LINE_INTERSECT_COUNT) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Too Many Input Lines");
        }
        else {
            ImGui::Text("Intersections: %i", intersect_count_);
            ImGui::Separator();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Min Angle", GetInstanceName()).c_str(), &min_angle_, 0.1f, 0.0f, 360.0f);
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Min Dist", GetInstanceName()).c_str(), &min_dist_, 0.1f, 0.0f, 1000.0f);
            ImGui::Separator();
            ImGui::Checkbox(CreateControlString("Draw Intersection Points", GetInstanceName()).c_str(), &draw_intersections_);
            if (draw_intersections_) {
                ImGui::SetNextItemWidth(150);
                ImGui::ColorEdit3(CreateControlString("Point Color", GetInstanceName()).c_str(), (float*)&point_color_);
                ImGui::SetNextItemWidth(100);
                ImGui::Checkbox(CreateControlString("Solid Point", GetInstanceName()).c_str(), &point_solid_);
                ImGui::SetNextItemWidth(100);
                ImGui::DragInt(CreateControlString("Point Radius", GetInstanceName()).c_str(), &point_radius_, 0.1f, 1, 25);
                if (!point_solid_) {
                    ImGui::SetNextItemWidth(100);
                    ImGui::DragInt(CreateControlString("Point Thickness", GetInstanceName()).c_str(), &point_thickness_, 0.1f, 1, 25);
                }
            }
        }
    }

}

std::string LineIntersect::GetState()
{
    using namespace nlohmann;

    json state;

    state["min_angle"] = min_angle_;
    state["min_dist"] = min_dist_;
    state["draw_points"] = draw_intersections_;
    state["point_radius"] = point_radius_;
    state["point_thickness"] = point_thickness_;
    state["point_solid"] = point_solid_;
    json pntColor;
    pntColor["R"] = point_color_.x;
    pntColor["G"] = point_color_.y;
    pntColor["B"] = point_color_.z;
    state["point_color"] = pntColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void LineIntersect::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("min_angle"))
        min_angle_ = state["min_angle"].get<float>();
    if (state.contains("min_dist"))
        min_dist_ = state["min_dist"].get<float>();
    if (state.contains("point_color")) {
        point_color_.x = state["point_color"]["R"].get<float>();
        point_color_.y = state["point_color"]["G"].get<float>();
        point_color_.z = state["point_color"]["B"].get<float>();
    }
    if (state.contains("point_radius"))
        point_radius_ = state["point_radius"].get<int>();
    if (state.contains("point_thickness"))
        point_thickness_ = state["point_thickness"].get<int>();
    if (state.contains("point_solid"))
        point_solid_ = state["point_solid"].get<bool>();
    if (state.contains("draw_points"))
        draw_intersections_ = state["draw_points"].get<bool>();
}

} // End Namespace DSPatch::DSPatchables