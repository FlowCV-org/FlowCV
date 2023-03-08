//
// Plugin DrawDateTime
//

#include "draw_datetime.hpp"
#include <ctime>

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

DrawDateTime::DrawDateTime() : Component(ProcessOrder::OutOfOrder)
{
    // Name and Category
    SetComponentName_("Draw_Date_Time");
    SetComponentCategory_(DSPatch::Category::Category_Draw);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_(1, {"in"}, {IoType::Io_Type_CvMat});

    // 1 outputs
    SetOutputCount_(1, {"out"}, {IoType::Io_Type_CvMat});

    text_scale_ = 1.0f;
    text_pos_ = {10, 30};
    text_color_ = {1.0f, 1.0f, 1.0f, 1.0f};
    text_thickness_ = 2;
    text_font_ = 0;
    draw_time_ = true;
    draw_date_ = true;
    memset(date_format_, '\0', 32);
    memset(time_format_, '\0', 32);
#ifdef _WIN32
    strcpy_s(date_format_, 32, "%Y-%m-%d");
    strcpy_s(time_format_, 32, " %X");
#else
    strncpy(date_format_, "%Y-%m-%d", 32);
    strncpy(time_format_, " %X", 32);
#endif
    date_format_str_ = date_format_;
    time_format_str_ = time_format_;
    is_valid_date_format_ = true;
    is_valid_time_format_ = true;

    SetEnabled(true);
}

void DrawDateTime::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    if (!in1) {
        return;
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            in1->copyTo(frame);
            std::string outStr;
            time_t now = time(nullptr);
            struct tm tstruct
            {
            };
            char buf[128];
#ifdef _WIN32
            localtime_s(&tstruct, &now);
#else
            localtime_r(&now, &tstruct);
#endif
            std::lock_guard<std::mutex> lck(io_mutex_);

            if (draw_date_) {
                strftime(buf, sizeof(buf), date_format_str_.c_str(), &tstruct);
                outStr += buf;
            }

            if (draw_time_) {
                strftime(buf, sizeof(buf), time_format_str_.c_str(), &tstruct);
                outStr += buf;
            }

            cv::putText(
                frame, outStr, text_pos_, text_font_, text_scale_, cv::Scalar(text_color_.z * 255, text_color_.y * 255, text_color_.x * 255), text_thickness_);

            if (!frame.empty())
                outputs.SetValue(0, frame);
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool DrawDateTime::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void DrawDateTime::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceName()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::Checkbox(CreateControlString("Draw Date", GetInstanceName()).c_str(), &draw_date_);
        std::lock_guard<std::mutex> lck(io_mutex_);
        if (draw_date_) {
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText(CreateControlString("Date Format", GetInstanceName()).c_str(), date_format_, 32, ImGuiInputTextFlags_EnterReturnsTrue)) {
                is_valid_date_format_ = IsFormatValid(0);
                if (is_valid_date_format_)
                    date_format_str_ = date_format_;
            }
            ImGui::SameLine();
            HelpMarker(
                "%Y - Year, full four digits\n"
                "%y - Year, last two digits (00-99)\n"
                "%C - Year divided by 100 and truncated to integer (00-99)\n"
                "%G - Week-based year\n"
                "%g - Week-based year, last two digits (00-99)\n"
                "%B - Full month name\n"
                "%b - Abbreviated month name\n"
                "%h - Abbreviated month name (same as %b)\n"
                "%m - Month as a decimal number (01-12)\n"
                "%U - Week number with the first Sunday as the first day of week one (00-53)\n"
                "%W - Week number with the first Monday as the first day of week one (00-53)\n"
                "%V - ISO 8601 week number (01-53)\n"
                "%j - Day of the year (001-366)\n"
                "%d - Day of the month, zero-padded (01-31)\n"
                "%e - Day of the month, space-padded (1-31)\n"
                "%A - Full weekday name\n"
                "%a - Abbreviated weekday name\n");
            if (!is_valid_date_format_) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Date Format");
            }
        }
        ImGui::Checkbox(CreateControlString("Draw Time", GetInstanceName()).c_str(), &draw_time_);
        if (draw_time_) {
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText(CreateControlString("Time Format", GetInstanceName()).c_str(), time_format_, 32, ImGuiInputTextFlags_EnterReturnsTrue)) {
                is_valid_time_format_ = IsFormatValid(1);
                if (is_valid_time_format_)
                    time_format_str_ = time_format_;
            }
            ImGui::SameLine();
            HelpMarker(
                "%H - Hour in 24h format (00-23)\n"
                "%I - Hour in 12h format (01-12)\n"
                "%M - Minute (00-59)\n"
                "%S - Second (00-61)\n"
                "%c - Date and time representation\n"
                "%X - Time representation\n"
                "%R - 24-hour HH:MM time, equivalent to %H:%M\n"
                "%r - 12-hour clock time\n"
                "%T - ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S\n"
                "%p - AM or PM designation\n"
                "%Z - Timezone name or abbreviation\n"
                "%z - ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100)\n");
            if (!is_valid_time_format_) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Time Format");
            }
        }
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
        ImGui::DragInt(CreateControlString("Thickness", GetInstanceName()).c_str(), &text_thickness_, 0.1f);
        ImGui::Separator();
        ImGui::ColorEdit3(CreateControlString("Color", GetInstanceName()).c_str(), (float *)&text_color_);
    }
}

std::string DrawDateTime::GetState()
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
    json tColor;
    tColor["R"] = text_color_.x;
    tColor["G"] = text_color_.y;
    tColor["B"] = text_color_.z;
    state["color"] = tColor;
    state["show_date"] = draw_date_;
    if (draw_date_)
        state["date_fmt"] = date_format_str_;
    state["show_time"] = draw_time_;
    if (draw_time_)
        state["time_fmt"] = time_format_str_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void DrawDateTime::SetState(std::string &&json_serialized)
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
    if (state.contains("font"))
        text_font_ = state["font"].get<int>();
    if (state.contains("show_date"))
        draw_date_ = state["show_date"].get<bool>();
    if (state.contains("show_time"))
        draw_time_ = state["show_time"].get<bool>();
    if (state.contains("date_fmt")) {
        date_format_str_ = state["date_fmt"].get<std::string>();
        sprintf(date_format_, "%s", date_format_str_.c_str());
    }
    if (state.contains("time_fmt")) {
        time_format_str_ = state["time_fmt"].get<std::string>();
        sprintf(time_format_, "%s", time_format_str_.c_str());
    }
}

bool DrawDateTime::IsFormatValid(int mode)
{
    static const std::vector<char> date_allow = {'Y', 'y', 'C', 'G', 'g', 'b', 'h', 'B', 'm', 'U', 'W', 'V', 'j', 'd', 'e', 'a', 'A', 'w', 'u', 'x', 'D', 'F'};
    static const std::vector<char> time_allow = {'H', 'I', 'M', 'S', 'c', 'X', 'r', 'R', 'T', 'p', 'z', 'Z'};

    int foundPercent = 0;
    int foundPercentMatch = -1;

    if (mode == 0) {  // Date Mode
#ifdef _WIN32
        size_t fmtSize = strnlen_s(date_format_, 32);
#else
        size_t fmtSize = strnlen(date_format_, 32);
#endif
        if (fmtSize < 2)
            return false;
        if (date_format_[fmtSize - 1] == '%')
            return false;
        for (int i = 0; i < fmtSize - 1; i++) {
            if (date_format_[i] == '%') {
                foundPercent++;
                for (auto &c : date_allow) {
                    if (c == date_format_[i + 1]) {
                        if (foundPercentMatch == -1)
                            foundPercentMatch = 1;
                        else
                            foundPercentMatch++;
                        i++;
                        break;
                    }
                }
            }
        }
        if (foundPercent == foundPercentMatch)
            return true;
    }
    else if (mode == 1) {  // Time Mode
#ifdef _WIN32
        size_t fmtSize = strnlen_s(time_format_, 32);
#else
        size_t fmtSize = strnlen(time_format_, 32);
#endif
        if (fmtSize < 2)
            return false;
        if (time_format_[fmtSize - 1] == '%')
            return false;
        for (int i = 0; i < fmtSize - 1; i++) {
            if (time_format_[i] == '%') {
                foundPercent++;
                for (auto &c : time_allow) {
                    if (c == time_format_[i + 1]) {
                        if (foundPercentMatch == -1)
                            foundPercentMatch = 1;
                        else
                            foundPercentMatch++;
                        i++;
                        break;
                    }
                }
            }
        }
        if (foundPercent == foundPercentMatch)
            return true;
    }

    return false;
}

}  // End Namespace DSPatch::DSPatchables