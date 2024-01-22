//
// Plugin BoundingRegion
//

#include "bounding_region.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

BoundingRegion::BoundingRegion() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Bounding_Region");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 3 inputs
    SetInputCount_(3, {"viz", "in", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 2 outputs
    SetOutputCount_(2, {"out", "region"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // Add and Configure Options
    props_.AddOption("mode", "Mode", 0, {"Rectangle", "Rotated Rectangle", "Circle", "Ellipse"});
    props_.AddBool("combine", "Combine All", true);
    props_.AddBool("draw_bbox", "Draw Bounding Box", true);
    props_.AddInt("line_thickness", "Line Thickness", 2, 1, 100, 0.1f);
    bbox_color_ = ImVec4(0.0f, 0.0f, 1.0f, 0.0f);

    SetEnabled(true);
}

void BoundingRegion::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in_viz = inputs.GetValue<cv::Mat>(0);
    auto in_mat = inputs.GetValue<cv::Mat>(1);
    auto in_json = inputs.GetValue<nlohmann::json>(2);

    if (!in_viz && !in_mat && !in_json) {
        return;
    }

    cv::Mat tmp, frame;

    if (in_viz) {
        if (!in_viz->empty()) {
            in_viz->copyTo(frame);
        }
    }

    if (IsEnabled()) {
        // Thread safe property sync from UI
        props_.Sync();

        // Process Image
        std::vector<std::vector<cv::Point>> points_in(1);
        bool has_mat_points = false;
        // First Check if in_mat is valid and not empty
        if (in_mat) {
            if (!in_mat->empty()) {
                // Check MAT pixel format/channels and fix if needed
                if (in_mat->channels() > 1)
                    cv::cvtColor(*in_mat, tmp, cv::COLOR_BGR2GRAY);
                else
                    tmp = *in_mat;

                // Bounding Box
                if (props_.Get<int>("mode") == 0) {
                    cv::Rect2i rect_in = cv::boundingRect(tmp);
                    points_in.at(0).emplace_back(rect_in.x, rect_in.y);
                    points_in.at(0).emplace_back(rect_in.x + rect_in.width, rect_in.y);
                    points_in.at(0).emplace_back(rect_in.x + rect_in.width, rect_in.y + rect_in.height);
                    points_in.at(0).emplace_back(rect_in.x, rect_in.y + rect_in.height);
                    has_mat_points = true;
                }
                // Rotated Rectangle
                else if (props_.Get<int>("mode") == 1) {
                    std::vector<std::vector<cv::Point>> contours;
                    std::vector<cv::Point> ctPnts;
                    cv::findContours(tmp, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                    for (const auto &p1 : contours) {
                        for (const auto &p2 : p1) {
                            ctPnts.emplace_back(p2.x, p2.y);
                        }
                    }
                    auto rect_out = cv::minAreaRect(ctPnts);
                    cv::Point2f rect_points[4];
                    rect_out.points(rect_points);
                    for (int j = 0; j < 4; j++) {
                        points_in.at(0).emplace_back((int)rect_points[j].x, (int)rect_points[j].y);
                    }
                    has_mat_points = true;
                }
                // Circle or Ellipse
                else if (props_.Get<int>("mode") == 2 || props_.Get<int>("mode") == 3) {
                    std::vector<std::vector<cv::Point>> contours;
                    std::vector<cv::Point> ctPnts;
                    cv::findContours(tmp, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                    for (const auto &p1 : contours) {
                        for (const auto &p2 : p1) {
                            points_in.at(0).emplace_back(p2.x, p2.y);
                        }
                    }
                }
            }
        }

        // Check if JSON is valid and not empty
        if (in_json) {
            nlohmann::json json_data;
            json_data = *in_json;
            if (!json_data.empty()) {
                // Check for Points or Contours
                if (json_data.contains("data")) {
                    std::string dt;
                    if (json_data.contains("data_type"))
                        dt = json_data["data_type"].get<std::string>();

                    if (dt == "points") {
                        for (const auto &data : json_data["data"]) {
                            if (!data.empty()) {
                                cv::Point pt;
                                pt.x = (int)data["x"].get<float>();
                                pt.y = (int)data["y"].get<float>();
                                if (props_.Get<bool>("combine")) {
                                    points_in.at(0).emplace_back(pt);
                                }
                            }
                        }
                    }
                    else if (dt == "contours") {
                        int i = 0;
                        for (const auto &data : json_data["data"]) {
                            if (data.contains("contour")) {
                                if (!props_.Get<bool>("combine")) {
                                    if (!points_in[0].empty() || has_mat_points) {
                                        points_in.emplace_back();
                                        i++;
                                    }
                                }
                                for (const auto &ct : data["contour"]) {
                                    points_in.at(i).emplace_back(ct["x"].get<int>(), ct["y"].get<int>());
                                }
                            }
                        }
                    }
                }
            }
        }

        // Process data to generate bounding region
        if (!points_in.empty()) {
            nlohmann::json json_out;
            nlohmann::json jRegion;
            nlohmann::json ref;
            ref["w"] = frame.cols;
            ref["h"] = frame.rows;
            json_out["ref_frame"] = ref;

            // Rectangle
            if (props_.Get<int>("mode") == 0) {
                json_out["data_type"] = "bbox";
                for (const auto &p : points_in) {
                    if (!p.empty()) {
                        auto rect_out = cv::boundingRect(p);
                        jRegion["x"] = rect_out.x;
                        jRegion["y"] = rect_out.y;
                        jRegion["w"] = rect_out.width;
                        jRegion["h"] = rect_out.height;
                        if (props_.Get<bool>("draw_bbox")) {
                            if (!frame.empty()) {
                                cv::rectangle(frame, rect_out, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255),
                                    props_.Get<int>("line_thickness"));
                            }
                        }
                        nlohmann::json bbox;
                        bbox["bbox"] = jRegion;
                        json_out["data"].emplace_back(bbox);
                    }
                }
            }
            // Rotated Rectangle
            else if (props_.Get<int>("mode") == 1) {
                json_out["data_type"] = "rot_rect";
                for (const auto &p : points_in) {
                    if (!p.empty()) {
                        auto rect_out = cv::minAreaRect(p);
                        cv::Point2f rect_points[4];
                        rect_out.points(rect_points);
                        nlohmann::json rot_rect;
                        nlohmann::json rot_rect_data;
                        rot_rect_data["angle"] = rect_out.angle;
                        rot_rect_data["cx"] = rect_out.center.x;
                        rot_rect_data["cy"] = rect_out.center.y;
                        rot_rect_data["w"] = rect_out.size.width;
                        rot_rect_data["h"] = rect_out.size.height;
                        nlohmann::json rot_rect_pts;
                        for (int j = 0; j < 4; j++) {
                            nlohmann::json jc;
                            jc["x"] = rect_points[j].x;
                            jc["y"] = rect_points[j].y;
                            rot_rect_pts.emplace_back(jc);
                            if (props_.Get<bool>("draw_bbox")) {
                                cv::Point p1, p2;
                                p1.x = (int)rect_points[j].x;
                                p1.y = (int)rect_points[j].y;
                                p2.x = (int)rect_points[(j + 1) % 4].x;
                                p2.y = (int)rect_points[(j + 1) % 4].y;
                                line(frame, p1, p2, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255),
                                    props_.Get<int>("line_thickness"));
                            }
                        }
                        rot_rect_data["points"] = rot_rect_pts;
                        rot_rect["rot_rect"] = rot_rect_data;
                        jRegion.emplace_back(rot_rect);
                    }
                }
                json_out["data"] = jRegion;
            }
            // Circle
            else if (props_.Get<int>("mode") == 2) {
                json_out["data_type"] = "circle";
                for (const auto &p : points_in) {
                    if (!p.empty()) {
                        cv::Point2f center;
                        float rad;
                        cv::minEnclosingCircle(p, center, rad);
                        nlohmann::json fit_circle;
                        nlohmann::json fit_circle_data;
                        fit_circle_data["cx"] = center.x;
                        fit_circle_data["cy"] = center.y;
                        fit_circle_data["radius"] = rad;
                        fit_circle["circle"] = fit_circle_data;
                        jRegion.emplace_back(fit_circle);
                        if (props_.Get<bool>("draw_bbox")) {
                            cv::circle(frame, center, (int)rad, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255),
                                props_.Get<int>("line_thickness"));
                        }
                    }
                }
                json_out["data"] = jRegion;
            }
            // Ellipse
            else if (props_.Get<int>("mode") == 3) {
                json_out["data_type"] = "ellipse";
                for (const auto &p : points_in) {
                    if (!p.empty()) {
                        if (p.size() > 4) {
                            auto el = cv::fitEllipse(p);
                            nlohmann::json fit_circle;
                            nlohmann::json fit_circle_data;
                            fit_circle_data["cx"] = el.center.x;
                            fit_circle_data["cy"] = el.center.y;
                            fit_circle_data["w"] = el.size.width;
                            fit_circle_data["h"] = el.size.height;
                            fit_circle_data["angle"] = el.angle;
                            fit_circle["ellipse"] = fit_circle_data;
                            jRegion.emplace_back(fit_circle);
                            if (props_.Get<bool>("draw_bbox")) {
                                cv::ellipse(
                                    frame, el, cv::Scalar(bbox_color_.z * 255, bbox_color_.y * 255, bbox_color_.x * 255), props_.Get<int>("line_thickness"));
                            }
                        }
                    }
                }
                json_out["data"] = jRegion;
            }

            if (!json_out.empty()) {
                outputs.SetValue(1, json_out);
            }
        }
    }

    if (!frame.empty()) {
        outputs.SetValue(0, frame);
    }
}

bool BoundingRegion::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void BoundingRegion::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        // Draw Property Controls (will be drawn in order added)
        props_.DrawUi(GetInstanceName());

        // Add additional UI property logic here
        if (props_.GetW<bool>("draw_bbox")) {
            props_.SetVisibility("line_thickness", true);
            ImGui::ColorEdit3(CreateControlString("BBox Color", GetInstanceName()).c_str(), (float *)&bbox_color_);
        }
        else {
            props_.SetVisibility("line_thickness", false);
        }
    }
}

std::string BoundingRegion::GetState()
{
    using namespace nlohmann;

    json state;

    // Convert properties to JSON
    props_.ToJson(state);
    json bboxColor;
    bboxColor["R"] = bbox_color_.x;
    bboxColor["G"] = bbox_color_.y;
    bboxColor["B"] = bbox_color_.z;
    state["bbox_color"] = bboxColor;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void BoundingRegion::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("bbox_color")) {
        bbox_color_.x = state["bbox_color"]["R"].get<float>();
        bbox_color_.y = state["bbox_color"]["G"].get<float>();
        bbox_color_.z = state["bbox_color"]["B"].get<float>();
    }

    // Set Properties from JSON
    props_.FromJson(state);
}

}  // End Namespace DSPatch::DSPatchables