//
// Plugin HoughLines
//

#include "hough_lines.hpp"
#include <optional>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

struct Line
{
    cv::Point start;
    cv::Point end;
};

static std::optional<cv::Point> GetLineIntersection(const Line &l1, const Line &l2)
{
    cv::Point pInter = {0, 0};

    double a1 = l1.end.y - l1.start.y;
    double b1 = l1.start.x - l1.end.x;
    double c1 = a1 * (l1.start.x) + b1 * (l1.start.y);

    double a2 = l2.end.y - l2.start.y;
    double b2 = l2.start.x - l2.end.x;
    double c2 = a2 * (l2.start.x) + b2 * (l2.start.y);

    double determinant = a1 * b2 - a2 * b1;

    if (determinant != 0) {
        pInter.x = (b2 * c1 - b1 * c2) / determinant;
        pInter.y = (a1 * c2 - a2 * c1) / determinant;
        return pInter;
    }

    return std::nullopt;
}

static std::optional<Line> GetInsideLine(const Line &l1, int width, int height)
{
    Line innerLine;

    // Define Image Border Lines
    Line bLeft = {cv::Point(0, 0), cv::Point(0, height)};
    Line bTop = {cv::Point(0, 0), cv::Point(width, 0)};
    Line bBot = {cv::Point(0, height), cv::Point(width, height)};
    Line bRight = {cv::Point(width, 0), cv::Point(width, height)};

    bool foundStart = false;

    // Left
    if (auto pt = GetLineIntersection(l1, bLeft)) {
        if (pt->y >= 0 && pt->y <= height) {
            innerLine.start.x = pt->x;
            innerLine.start.y = pt->y;
            foundStart = true;
        }
    }

    // Top
    if (auto pt = GetLineIntersection(l1, bTop)) {
        if (pt->x >= 0 && pt->x <= width) {
            if (!foundStart) {
                innerLine.start.x = pt->x;
                innerLine.start.y = pt->y;
                foundStart = true;
            }
            else {
                innerLine.end.x = pt->x;
                innerLine.end.y = pt->y;
                return innerLine;
            }
        }
    }

    // Bottom
    if (auto pt = GetLineIntersection(l1, bBot)) {
        if (pt->x >= 0 && pt->x <= width) {
            if (!foundStart) {
                innerLine.start.x = pt->x;
                innerLine.start.y = pt->y;
                foundStart = true;
            }
            else {
                innerLine.end.x = pt->x;
                innerLine.end.y = pt->y;
                return innerLine;
            }
        }
    }

    // Right
    if (foundStart) {
        if (auto pt = GetLineIntersection(l1, bRight)) {
            if (pt->y >= 0 && pt->y <= height) {
                innerLine.end.x = pt->x;
                innerLine.end.y = pt->y;
                return innerLine;
            }
        }
    }

    return std::nullopt;
}

namespace DSPatch::DSPatchables
{

HoughLines::HoughLines() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Hough_Lines");
    SetComponentCategory_(DSPatch::Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 2 outputs
    SetOutputCount_(2, {"out", "lines"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    SetEnabled(true);

    valid_format_ = false;
    has_input_ = false;
    rho_ = 1.0f;
    thresh_ = 150;
    srn_ = 0.0f;
    stn_ = 0.0f;
    hough_mode_ = 0;
    min_theta_ = 0.0f;
    max_theta_ = 360.0f;
    min_line_len_ = 100.0f;
    max_line_gap_ = 10.0f;
    line_color_ = {1.0f, 0.0f, 0.0f, 1.0f};
    draw_lines_ = false;
    line_thickness_ = 3;
}

void HoughLines::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    static const float DEG_TO_RAD = CV_PI / 180;

    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        has_input_ = false;
        return;
    }

    if (!in1->empty()) {
        has_input_ = true;
        if (IsEnabled()) {
            if (in1->type() == CV_8U)
                valid_format_ = true;
            else
                valid_format_ = false;

            if (valid_format_) {
                cv::Mat frame, outFrame;
                in1->copyTo(frame);
                std::vector<cv::Vec2f> found_lines;
                std::vector<cv::Vec4i> found_p_lines;

                if (hough_mode_ == 0)
                    cv::HoughLines(frame, found_lines, rho_, DEG_TO_RAD, thresh_, srn_, stn_, min_theta_ * DEG_TO_RAD, max_theta_ * DEG_TO_RAD);
                else if (hough_mode_ == 1)
                    cv::HoughLinesP(frame, found_p_lines, rho_, DEG_TO_RAD, thresh_, min_line_len_, max_line_gap_);

                nlohmann::json json_out;
                nlohmann::json jLines;
                json_out["data_type"] = "lines";
                nlohmann::json ref;
                ref["w"] = frame.cols;
                ref["h"] = frame.rows;
                json_out["ref_frame"] = ref;

                cv::cvtColor(frame, outFrame, cv::COLOR_GRAY2BGR);
                if (hough_mode_ == 0) {
                    for (auto &found_line : found_lines) {
                        float rho = found_line[0], theta = found_line[1];
                        double a = cos(theta), b = sin(theta);
                        double x0 = a * rho, y0 = b * rho;
                        float w2 = (float)outFrame.cols * 2.0f;
                        Line l1;
                        l1.start.x = cvRound(x0 + w2 * (-b));
                        l1.start.y = cvRound(y0 + w2 * (a));
                        l1.end.x = cvRound(x0 - w2 * (-b));
                        l1.end.y = cvRound(y0 - w2 * (a));
                        if (auto lInt = GetInsideLine(l1, outFrame.cols, outFrame.rows)) {
                            nlohmann::json cTmp;
                            nlohmann::json jLine;
                            cTmp["x0"] = lInt->start.x;
                            cTmp["y0"] = lInt->start.y;
                            cTmp["x1"] = lInt->end.x;
                            cTmp["y1"] = lInt->end.y;
                            jLine["line"] = cTmp;
                            jLines.emplace_back(jLine);
                            if (draw_lines_)
                                line(outFrame, lInt->start, lInt->end, cv::Scalar(line_color_.z * 255, line_color_.y * 255, line_color_.x * 255),
                                    line_thickness_, cv::LINE_AA);
                        }
                    }
                }
                else if (hough_mode_ == 1) {
                    for (auto &found_p_line : found_p_lines) {
                        nlohmann::json cTmp;
                        nlohmann::json jLine;
                        cTmp["x0"] = found_p_line[0];
                        cTmp["y0"] = found_p_line[1];
                        cTmp["x1"] = found_p_line[2];
                        cTmp["y1"] = found_p_line[3];
                        jLine["line"] = cTmp;
                        jLines.emplace_back(jLine);
                        if (draw_lines_) {
                            line(outFrame, cv::Point(found_p_line[0], found_p_line[1]), cv::Point(found_p_line[2], found_p_line[3]),
                                cv::Scalar(line_color_.z * 255, line_color_.y * 255, line_color_.x * 255), line_thickness_, cv::LINE_AA);
                        }
                    }
                }
                if (!outFrame.empty()) {
                    outputs.SetValue(0, outFrame);
                    json_out["data"] = jLines;
                    outputs.SetValue(1, json_out);
                }
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool HoughLines::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void HoughLines::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (has_input_ && !valid_format_) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Input Image Format");
            ImGui::Separator();
        }
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(CreateControlString("Hough Line Mode", GetInstanceName()).c_str(), &hough_mode_, "Standard\0Probabilistic\0\0");
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat(CreateControlString("Accum Dist Res", GetInstanceName()).c_str(), &rho_, 0.01f, 0.01f, 10.0f);
        ImGui::SetNextItemWidth(100);
        ImGui::DragInt(CreateControlString("Threshold", GetInstanceName()).c_str(), &thresh_, 0.1f, 1, 1000);
        if (hough_mode_ == 0) {
            ImGui::SetNextItemWidth(150);
            if (ImGui::DragFloatRange2("Min/Max Theta", &min_theta_, &max_theta_, 0.1f, 0.0f, 360.0f, "Min: %.1f °", "Max: %.1f °")) {
                if (min_theta_ > max_theta_)
                    min_theta_ = max_theta_ - 1.0f;
            }
        }
        else if (hough_mode_ == 1) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Min Line Length", GetInstanceName()).c_str(), &min_line_len_, 0.1f, 1.0f, 1000.0f);
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat(CreateControlString("Max Line Gap", GetInstanceName()).c_str(), &max_line_gap_, 0.1f, 1.0f, 1000.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Draw Lines", GetInstanceName()).c_str(), &draw_lines_);
        if (draw_lines_) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragInt(CreateControlString("Line Thickness", GetInstanceName()).c_str(), &line_thickness_, 0.1f, 1, 25);
            ImGui::ColorEdit3(CreateControlString("Line Color", GetInstanceName()).c_str(), (float *)&line_color_);
        }
    }
}

std::string HoughLines::GetState()
{
    using namespace nlohmann;

    json state;

    state["draw_lines"] = draw_lines_;
    state["hough_mode"] = hough_mode_;
    state["rho"] = rho_;
    state["thresh"] = thresh_;
    state["min_theta"] = min_theta_;
    state["max_theta"] = max_theta_;
    state["min_line_len"] = min_line_len_;
    state["max_line_gap"] = max_line_gap_;
    json lineColor;
    lineColor["R"] = line_color_.x;
    lineColor["G"] = line_color_.y;
    lineColor["B"] = line_color_.z;
    state["line_color"] = lineColor;
    state["line_thickness"] = line_thickness_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void HoughLines::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("draw_lines"))
        draw_lines_ = state["draw_lines"].get<bool>();
    if (state.contains("rho"))
        rho_ = state["rho"].get<float>();
    if (state.contains("hough_mode"))
        hough_mode_ = state["hough_mode"].get<int>();
    if (state.contains("thresh"))
        thresh_ = state["thresh"].get<int>();
    if (state.contains("min_theta"))
        min_theta_ = state["min_theta"].get<float>();
    if (state.contains("max_theta"))
        max_theta_ = state["max_theta"].get<float>();
    if (state.contains("min_line_len"))
        min_line_len_ = state["min_line_len"].get<float>();
    if (state.contains("max_line_gap"))
        max_line_gap_ = state["max_line_gap"].get<float>();
    if (state.contains("line_color")) {
        line_color_.x = state["line_color"]["R"].get<float>();
        line_color_.y = state["line_color"]["G"].get<float>();
        line_color_.z = state["line_color"]["B"].get<float>();
    }
    if (state.contains("line_thickness"))
        line_thickness_ = state["line_thickness"].get<int>();
}

}  // End Namespace DSPatch::DSPatchables