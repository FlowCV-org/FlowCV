//
// Plugin ShapeColorizer
//

#include "Shape_Colorizer.hpp"
#include <math.h>

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

template<typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1)
{
    static const char *digits = "0123456789ABCDEF";
    std::string rc(hex_len, '0');
    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
        rc[i] = digits[(w >> j) & 0x0f];
    return rc;
}

cv::Vec3f ColorToLab(float R, float G, float B)
{
    // http://www.brucelindbloom.com

    double r, g, b, X, Y, Z, fx, fy, fz, xr, yr, zr;
    double Ls, fas, fbs;
    double eps = 216.0f / 24389.0f;
    double k = 24389.0f / 27.0f;

    double Xr = 0.964221f;  // reference white D50
    double Yr = 1.0f;
    double Zr = 0.825211f;

    // RGB to XYZ
    r = R;  // R 0..1
    g = G;  // G 0..1
    b = B;  // B 0..1

    // assuming sRGB (D65)
    if (r <= 0.04045)
        r = r / 12;
    else
        r = (float)pow((r + 0.055) / 1.055, 2.4);

    if (g <= 0.04045)
        g = g / 12;
    else
        g = (float)pow((g + 0.055) / 1.055, 2.4);

    if (b <= 0.04045)
        b = b / 12;
    else
        b = (float)pow((b + 0.055) / 1.055, 2.4);

    X = 0.436052025f * r + 0.385081593f * g + 0.143087414f * b;
    Y = 0.222491598f * r + 0.71688606f * g + 0.060621486f * b;
    Z = 0.013929122f * r + 0.097097002f * g + 0.71418547f * b;

    // XYZ to Lab
    xr = X / Xr;
    yr = Y / Yr;
    zr = Z / Zr;

    if (xr > eps)
        fx = (float)pow(xr, 1 / 3.0);
    else
        fx = (float)((k * xr + 16.0) / 116.0);

    if (yr > eps)
        fy = (float)pow(yr, 1 / 3.0);
    else
        fy = (float)((k * yr + 16.0) / 116.0);

    if (zr > eps)
        fz = (float)pow(zr, 1 / 3.0);
    else
        fz = (float)((k * zr + 16.0) / 116);

    Ls = (116 * fy) - 16;
    fas = 500 * (fx - fy);
    fbs = 200 * (fy - fz);

    cv::Vec3f lab;
    lab[0] = (2.55 * Ls + 0.5);
    lab[1] = (fas + 0.5);
    lab[2] = (fbs + 0.5);

    return lab;
}

namespace DSPatch::DSPatchables::internal
{
class ShapeColorizer
{
};
}  // namespace DSPatch::DSPatchables::internal

ShapeColorizer::ShapeColorizer() : Component(ProcessOrder::OutOfOrder), p(new internal::ShapeColorizer())
{
    // Name and Category
    SetComponentName_("Shape_Colorizer");
    SetComponentCategory_(Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(2, {"in", "shapes"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    // 1 outputs
    SetOutputCount_(2, {"out", "shapes"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    method_ = 0;
    overlay_ = true;
    show_labels_ = false;
    match_threshold_ = 75;
    ctl_cnt_ = 0;
    dist_method_ = 0;
    ignore_none_ = true;

    SetEnabled(true);
}

void ShapeColorizer::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto shapes1 = inputs.GetValue<nlohmann::json>(1);
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!shapes1 || !in1) {
        return;
    }

    nlohmann::json json_data_;
    json_data_ = *shapes1;

    // Process Image
    int w = 640;
    int h = 480;
    if (!json_data_.empty()) {
        if (json_data_.contains("ref_frame")) {
            w = json_data_["ref_frame"]["w"].get<int>();
            h = json_data_["ref_frame"]["h"].get<int>();
        }
    }

    if (IsEnabled()) {
        if (in1->empty()) {
            cv::Mat frame(h, w, CV_8UC3, cv::Scalar(0, 0, 0));

            cv::Point pos;
            pos.x = (w - 265) / 2;
            pos.y = h / 2;
            cv::putText(frame, "Need to supply input frame for reference", pos, cv::HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.4f, cv::Scalar(0, 0, 255), 1);
            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            cv::Mat frame;
            in1->copyTo(frame);
            cv::Mat matFrame(h, w, CV_8UC3, cv::Scalar(0, 0, 0));

            if (!json_data_.empty()) {
                std::lock_guard<std::mutex> lck(io_mutex_);
                nlohmann::json json_out;
                if (json_data_.contains("data")) {
                    for (auto &d : json_data_["data"]) {
                        if (d.contains("x") && d.contains("y") && d.contains("size")) {
                            cv::Point pos;
                            pos.x = d["x"].get<int>();
                            pos.y = d["y"].get<int>();
                            int rad = d["size"].get<int>() / 2;
                            cv::circle(matFrame, pos, rad, cv::Scalar(255, 255, 255), -1);
                            cv::Mat mat_8U;
                            cv::cvtColor(matFrame, mat_8U, cv::COLOR_BGR2GRAY);
                            cv::Rect crop;
                            int x2, y2;
                            crop.x = pos.x - rad;
                            crop.y = pos.y - rad;
                            x2 = pos.x + rad;
                            y2 = pos.y + rad;
                            if (crop.x < 0)
                                crop.x = 0;
                            if (crop.y < 0)
                                crop.y = 0;
                            if (x2 >= frame.cols)
                                x2 = frame.cols - 1;
                            if (y2 >= frame.rows)
                                y2 = frame.rows - 1;
                            crop.width = x2 - crop.x;
                            crop.height = y2 - crop.y;
                            cv::Mat roi = frame(crop);
                            cv::Mat roi_mat = mat_8U(crop);
                            cv::Scalar m = cv::mean(roi, roi_mat);

                            if (method_ == 1) {
                                ImVec4 hsv;
                                ImVec4 rgb;
                                ImGui::ColorConvertRGBtoHSV(m[2] / 255.0f, m[1] / 255.0f, m[0] / 255.0f, hsv.x, hsv.y, hsv.z);
                                hsv.y = 1.0f;
                                hsv.z = 1.0f;
                                ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, rgb.x, rgb.y, rgb.z);
                                m[2] = rgb.x * 255;
                                m[1] = rgb.y * 255;
                                m[0] = rgb.z * 255;
                            }
                            else if (method_ == 2) {
                                bool foundMatch = false;
                                for (auto &c : color_match_list_) {
                                    cv::Scalar match(c.col.z * 255, c.col.y * 255, c.col.x * 255);
                                    double distMeasure = 255;

                                    if (dist_method_ == 0) {
                                        cv::Vec4d diff = match - m;
                                        distMeasure = cv::norm(diff);
                                    }
                                    else if (dist_method_ == 1) {
                                        cv::Vec3f lab1 = ColorToLab(c.col.x, c.col.y, c.col.z);
                                        cv::Vec3f lab2 = ColorToLab(m[2] / 255.0f, m[1] / 255.0f, m[0] / 255.0f);
                                        distMeasure = sqrt(pow(lab2[0] - lab1[0], 2) + pow(lab2[1] - lab1[1], 2) + pow(lab2[2] - lab1[2], 2));
                                    }
                                    if (abs(distMeasure) < match_threshold_) {
                                        if (d.contains("dist")) {
                                            float oldDist = d["dist"].get<float>();
                                            if (distMeasure < oldDist) {
                                                m = match;
                                                d["dist"] = (float)distMeasure;
                                                d["class_id"] = c.tag;
                                                foundMatch = true;
                                            }
                                        }
                                        else {
                                            m = match;
                                            d["dist"] = distMeasure;
                                            d["class_id"] = c.tag;
                                            foundMatch = true;
                                        }
                                    }
                                }
                                if (!foundMatch) {
                                    d["dist"] = -1;
                                    d["class_id"] = "None";
                                }
                            }
                            uint32_t col = ((int)m[2] << 16) + ((int)m[1] << 8) + (int)m[0];
                            std::string hex = "#";
                            hex += n2hexstr<uint32_t>(col, 6);
                            d["color"] = hex;
                            if (overlay_) {
                                cv::circle(frame, pos, rad, m, -1);
                                if (show_labels_) {
                                    if (method_ != 2) {
                                        cv::putText(frame, hex, cv::Point(pos.x - rad, pos.y), cv::FONT_HERSHEY_PLAIN, 0.65, cv::Scalar(255, 255, 255), 1.0);
                                    }
                                    else {
                                        if (d.contains("class_id")) {
                                            if (ignore_none_) {
                                                if (d["class_id"].get<std::string>() == "None")
                                                    continue;
                                            }
                                            cv::putText(frame, d["class_id"].get<std::string>(), cv::Point(pos.x - (rad * 0.5f), pos.y), cv::FONT_HERSHEY_PLAIN,
                                                0.65, cv::Scalar(255, 255, 255), 1.0);
                                        }
                                    }
                                }
                            }
                            else {
                                cv::circle(matFrame, pos, rad, m, -1);
                                if (show_labels_) {
                                    if (method_ != 2) {
                                        cv::putText(matFrame, hex, cv::Point(pos.x - rad, pos.y), cv::FONT_HERSHEY_PLAIN, 0.65, cv::Scalar(255, 255, 255), 1.0);
                                    }
                                    else {
                                        if (d.contains("class_id")) {
                                            if (ignore_none_) {
                                                if (d["class_id"].get<std::string>() == "None")
                                                    continue;
                                            }
                                            cv::putText(matFrame, d["class_id"].get<std::string>(), cv::Point(pos.x - (rad * 0.5f), pos.y),
                                                cv::FONT_HERSHEY_PLAIN, 0.65, cv::Scalar(255, 255, 255), 1.0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                outputs.SetValue(1, json_data_);
            }
            if (overlay_) {
                if (!frame.empty())
                    outputs.SetValue(0, frame);
            }
            else {
                if (!matFrame.empty())
                    outputs.SetValue(0, matFrame);
            }
        }
    }
    else {
        outputs.SetValue(0, *in1);
    }
}

bool ShapeColorizer::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void ShapeColorizer::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Combo(CreateControlString("Color Method", GetInstanceName()).c_str(), &method_, "Mean\0Max\0Closest\0\0");
        ImGui::Checkbox(CreateControlString("Overlay", GetInstanceName()).c_str(), &overlay_);
        ImGui::Checkbox(CreateControlString("Show Labels", GetInstanceName()).c_str(), &show_labels_);
        ImGui::Checkbox(CreateControlString("Ignore None", GetInstanceName()).c_str(), &ignore_none_);
        if (method_ == 2) {
            ImGui::Separator();
            ImGui::SetNextItemWidth(100);
            ImGui::Combo(CreateControlString("Distance Method", GetInstanceName()).c_str(), &dist_method_, "RGB\0CIE76\0\0");
            ImGui::SetNextItemWidth(80);
            ImGui::DragInt(CreateControlString("Match Threshold", GetInstanceName()).c_str(), &match_threshold_, 0.25f, 1, 255);
            ImGui::Separator();
            ImGui::TextUnformatted("Color Match List:");
            ImGui::Separator();
            if (ImGui::Button(CreateControlString("Add", GetInstanceName()).c_str())) {
                ColorMatch c;
                c.ctl_label = "Color##";
                c.ctl_label += std::to_string(ctl_cnt_);
                c.ctl_label += GetInstanceName();
                c.col = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                c.text_label = "Label##";
                c.text_label += std::to_string(ctl_cnt_);
                c.text_label += GetInstanceName();
                c.close_label = "X##";
                c.close_label += std::to_string(ctl_cnt_);
                c.close_label += GetInstanceName();
                color_match_list_.emplace_back(c);
                ctl_cnt_++;
            }
            ImGui::SameLine();
            if (ImGui::Button(CreateControlString("Clear All", GetInstanceName()).c_str())) {
                color_match_list_.clear();
                ctl_cnt_ = 1;
            }
            ImGui::Separator();
            for (int i = 0; i < color_match_list_.size(); i++) {
                ImGui::Separator();
                if (ImGui::Button(color_match_list_.at(i).close_label.c_str())) {
                    color_match_list_.erase(color_match_list_.begin() + i);
                    continue;
                }
                ImGui::ColorEdit3(color_match_list_.at(i).ctl_label.c_str(), (float *)&color_match_list_.at(i).col);
                ImGui::InputText(color_match_list_.at(i).text_label.c_str(), color_match_list_.at(i).tag, 32);
            }
        }
    }
}

std::string ShapeColorizer::GetState()
{
    using namespace nlohmann;

    json state;

    state["overlay"] = overlay_;
    state["show_labels"] = show_labels_;
    state["method"] = method_;
    state["threshold"] = match_threshold_;
    state["dist_method"] = dist_method_;
    state["ignore_none"] = ignore_none_;
    json colorList;
    for (auto &c : color_match_list_) {
        json colorEntry;
        json color;
        color["R"] = c.col.x;
        color["G"] = c.col.y;
        color["B"] = c.col.z;
        colorEntry["color"] = color;
        colorEntry["label"] = c.tag;
        colorList.emplace_back(colorEntry);
    }
    if (!colorList.empty())
        state["color_match"] = colorList;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void ShapeColorizer::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("overlay"))
        overlay_ = state["overlay"].get<bool>();
    if (state.contains("ignore_none"))
        ignore_none_ = state["ignore_none"].get<bool>();
    if (state.contains("dist_method"))
        dist_method_ = state["dist_method"].get<int>();
    if (state.contains("show_labels"))
        show_labels_ = state["show_labels"].get<bool>();
    if (state.contains("method"))
        method_ = state["method"].get<int>();
    if (state.contains("threshold"))
        match_threshold_ = state["threshold"].get<int>();
    if (state.contains("color_match")) {
        color_match_list_.clear();
        ctl_cnt_ = 1;
        for (auto &c : state["color_match"]) {
            ColorMatch cm;
            cm.col.x = c["color"]["R"].get<float>();
            cm.col.y = c["color"]["G"].get<float>();
            cm.col.z = c["color"]["B"].get<float>();
            sprintf(cm.tag, "%s", c["label"].get<std::string>().c_str());
            cm.ctl_label = "Color##";
            cm.ctl_label += std::to_string(ctl_cnt_);
            cm.ctl_label += std::to_string(GetInstanceCount());
            cm.text_label = "Label##";
            cm.text_label += std::to_string(ctl_cnt_);
            cm.text_label += std::to_string(GetInstanceCount());
            cm.close_label = "X##";
            cm.close_label += std::to_string(ctl_cnt_);
            cm.close_label += std::to_string(GetInstanceCount());
            color_match_list_.emplace_back(cm);
            ctl_cnt_++;
        }
    }
}
