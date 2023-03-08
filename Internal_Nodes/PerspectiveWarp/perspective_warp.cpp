//
// Plugin Perspective Warp
//

#include "perspective_warp.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

PerspectiveWarp::PerspectiveWarp() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Perspective_Warp");
    SetComponentCategory_(DSPatch::Category::Category_Transform);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "points"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    corner_count_ = 0;
    is_poly_ = false;
    ratio_adjustment_ = 0.0f;
    use_fixed_res_ = false;
    fixed_width_ = 640;
    fixed_height_ = 480;
    interp_mode_ = 1;

    SetEnabled(true);
}

void sortCorners(std::vector<cv::Point2f> &corners, const cv::Point2f &center)
{
    std::vector<cv::Point2f> top, bot;

    for (auto &corner : corners) {
        if (corner.y < center.y)
            top.emplace_back(corner);
        else
            bot.emplace_back(corner);
    }
    corners.clear();

    if (top.size() == 2 && bot.size() == 2) {
        cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
        cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
        cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
        cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

        corners.emplace_back(tl);
        corners.emplace_back(tr);
        corners.emplace_back(br);
        corners.emplace_back(bl);
    }
}

void PerspectiveWarp::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    auto in_img = inputs.GetValue<cv::Mat>(0);
    auto in_json = inputs.GetValue<nlohmann::json>(1);
    if (!in_img || !in_json) {
        is_poly_ = false;
        return;
    }

    if (!in_img->empty() && !in_json->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            nlohmann::json json_data = *in_json;
            std::vector<cv::Point2f> corners;
            if (json_data.contains("data_type")) {
                if (json_data["data_type"].get<std::string>() != "points") {
                    is_poly_ = false;
                    return;
                }
            }
            if (json_data.contains("data")) {
                corner_count_ = (int)json_data["data"].size();
                for (const auto &data : json_data["data"]) {
                    cv::Point2f pt;
                    pt.x = data["x"];
                    pt.y = data["y"];
                    corners.emplace_back(pt);
                }
            }
            if (corner_count_ == 4) {
                std::vector<cv::Point2f> approx;
                cv::approxPolyDP(cv::Mat(corners), approx, cv::arcLength(cv::Mat(corners), true) * 0.02, true);
                if (approx.size() == 4) {
                    cv::Point2f center(0, 0);
                    for (auto &corner : corners)
                        center += corner;
                    center *= (1.0f / (float)corners.size());
                    sortCorners(corners, center);
                    if (corners.size() == 4) {
                        is_poly_ = true;
                        int w = fixed_width_;
                        int h = fixed_height_;
                        if (!use_fixed_res_) {
                            int w1 = (int)cv::norm(corners.at(0) - corners.at(1));
                            int w2 = (int)cv::norm(corners.at(2) - corners.at(3));
                            int h1 = (int)cv::norm(corners.at(0) - corners.at(3));
                            int h2 = (int)cv::norm(corners.at(1) - corners.at(2));
                            w = (w1 + w2) / 2;
                            h = (h1 + h2) / 2;
                            float ratio = (float)h / (float)w;
                            ratio += ratio_adjustment_;
                            h = (int)((float)w * ratio);
                        }
                        if (w < 2)
                            w = 2;
                        if (h < 2)
                            h = 2;
                        cv::Mat quad = cv::Mat::zeros(h, w, CV_8UC3);

                        std::vector<cv::Point2f> quad_pts;
                        quad_pts.emplace_back(0.0f, 0.0f);
                        quad_pts.emplace_back(cv::Point2f((float)quad.cols, 0.0f));
                        quad_pts.emplace_back(cv::Point2f((float)quad.cols, (float)quad.rows));
                        quad_pts.emplace_back(cv::Point2f(0.0f, (float)quad.rows));

                        cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
                        cv::warpPerspective(*in_img, frame, transmtx, quad.size(), interp_mode_);
                        outputs.SetValue(0, frame);
                        return;
                    }
                }
            }
        }
        else {
            outputs.SetValue(0, *in_img);
        }
    }
    is_poly_ = false;
}

bool PerspectiveWarp::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void PerspectiveWarp::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (!is_poly_)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Quadrilateral Polygon Detected\nNeed Exactly 4 Points");
        else {
            ImGui::Combo(CreateControlString("Interpolation", GetInstanceName()).c_str(), &interp_mode_, "Nearest\0Bilinear\0BiCubic\0Area\0Lanczos\0\0");
            ImGui::Separator();
            ImGui::SetNextItemWidth(120);
            ImGui::Checkbox(CreateControlString("Use Fixed Resolution", GetInstanceName()).c_str(), &use_fixed_res_);
            if (use_fixed_res_) {
                ImGui::SetNextItemWidth(100);
                ImGui::DragInt(CreateControlString("Width", GetInstanceName()).c_str(), &fixed_width_, 0.5f, 2, 5000);
                ImGui::SetNextItemWidth(100);
                ImGui::DragInt(CreateControlString("Height", GetInstanceName()).c_str(), &fixed_height_, 0.5f, 2, 5000);
            }
            else {
                ImGui::SetNextItemWidth(100);
                ImGui::DragFloat(CreateControlString("Aspect Ratio Adjust", GetInstanceName()).c_str(), &ratio_adjustment_, 0.01f, -10.0f, 10.0f);
            }
        }
    }
}

std::string PerspectiveWarp::GetState()
{
    using namespace nlohmann;

    json state;

    state["aspect_ratio_adj"] = ratio_adjustment_;
    state["interp_mode"] = interp_mode_;
    state["use_fixed_res"] = use_fixed_res_;
    if (use_fixed_res_) {
        state["fixed_width"] = fixed_width_;
        state["fixed_height"] = fixed_height_;
    }
    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void PerspectiveWarp::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("aspect_ratio_adj"))
        ratio_adjustment_ = state["aspect_ratio_adj"].get<float>();
    if (state.contains("interp_mode"))
        interp_mode_ = state["interp_mode"].get<int>();
    if (state.contains("use_fixed_res"))
        use_fixed_res_ = state["use_fixed_res"].get<bool>();
    if (state.contains("fixed_width"))
        fixed_width_ = state["fixed_width"].get<int>();
    if (state.contains("fixed_height"))
        fixed_height_ = state["fixed_height"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables