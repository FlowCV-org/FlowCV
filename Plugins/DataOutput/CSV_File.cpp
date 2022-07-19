//
// Plugin CsvFile
//

#include "CSV_File.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class CsvFile
{
};
}  // namespace DSPatch

CsvFile::CsvFile()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::CsvFile() )
{
    // Name and Category
    SetComponentName_("CSV_File");
    SetComponentCategory_(Category::Category_Output);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 3, {"start", "frame", "json"}, {IoType::Io_Type_Bool, IoType::Io_Type_Int, IoType::Io_Type_JSON} );

    start_new_file_ = false;
    counter_ = 0;
    csv_cur_file_num_ = 0;
    last_counter_ = -1;
    save_timestamp_ = true;
    save_counter_ = false;
    show_file_dialog_ = false;
    max_file_size_kb_ = 1000;
    rotating_files_ = false;
    limit_file_size_ = false;
    num_files_ = 10;
    SetEnabled(true);

}

CsvFile::~CsvFile()
{
    if (csv_file_.is_open()) {
        csv_file_.flush();
        csv_file_.close();
    }
}

std::string CsvFile::GetRotateFilePath()
{
    std::string out_file;

    std::filesystem::path fPath = csv_file_path_;
    std::string basePath = fPath.parent_path().string();
    std::string baseName = fPath.filename().replace_extension().string();
    std::string ext = fPath.extension().string();
    out_file = basePath;
    out_file += '/';
    out_file += baseName;
    out_file += "_";
    out_file += std::to_string(csv_cur_file_num_);
    out_file += ext;

    return out_file;
}

void CsvFile::OpenCsvFile(nlohmann::json &json_data)
{
    if (!csv_file_path_.empty()) {
        if (csv_file_.is_open()) {
            csv_file_.flush();
            csv_file_.close();
        }
        // Open New File
        std::string out_file;
        if (start_new_file_) {
            counter_ = 0;
            csv_cur_file_num_ = 0;
        }

        if (rotating_files_) {
            out_file = GetRotateFilePath();
        }
        else
            out_file = csv_file_path_;

        csv_file_.open(out_file, std::ofstream::out);
        if (!rotating_files_)
            counter_ = 0;

        if (csv_file_.is_open()) {
            // Added Column Names
            csv_file_ << "# ";
            if (save_timestamp_)
                csv_file_ << "timestamp,";
            if (save_counter_)
                csv_file_ << "index,";
            if (json_data.contains("ref_frame"))
                csv_file_ << "resolution,";
            if (json_data.contains("data_type"))
                csv_file_ << "data_type,";

            csv_file_ << "data_index,";
            if (json_data.contains("data")) {
                if (!json_data["data"].empty()) {
                    for (nlohmann::json::iterator it = json_data["data"].at(0).begin(); it != json_data["data"].at(0).end(); ++it) {
                        if (!it->is_array()) {
                            csv_file_ << it.key() << ",";
                        }
                        else if (it->is_array()) {
                            for (int i = 0; i < it->size(); i++) {
                                std::string keyname = it.key();
                                keyname += '_';
                                keyname += std::to_string(i + 1);
                                csv_file_ << keyname << "," ;
                            }
                        }
                    }
                }
            }
            csv_file_ << "\n";
            csv_file_.flush();
        }
    }
}

void CsvFile::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<bool>( 0 );
    auto in2 = inputs.GetValue<int>( 1 );
    auto in3 = inputs.GetValue<nlohmann::json>( 2 );

    if (!in3)
        return;

    if (in1) {
        if (*in1)
            start_new_file_ = true;
    }

    if (in2) {
        counter_ = *in2;
    }

    // Get Current timestamp
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::chrono::system_clock::duration dtn = tp.time_since_epoch();
    uint64_t epochTimeMillis = dtn.count() / 10000;

    nlohmann::json json_in = *in3;
    if (!json_in.empty()) {
        if (start_new_file_) {
            if (json_in.contains("data")) {
                OpenCsvFile(json_in);
                start_new_file_ = false;
            }
            else
                return;
        }
        if (csv_file_.is_open()) {
            if (limit_file_size_) {
                std::filesystem::path fPath;
                if (rotating_files_)
                    fPath = std::filesystem::path(GetRotateFilePath());
                else
                    fPath = std::filesystem::path(csv_file_path_);

                if (exists(fPath)) {
                    auto fSizeK = std::filesystem::file_size(fPath) / 1000;
                    if (fSizeK >= max_file_size_kb_) {
                        if (rotating_files_) {
                            csv_cur_file_num_++;
                            if (csv_cur_file_num_ > num_files_)
                                csv_cur_file_num_ = 0;
                            OpenCsvFile(json_in);
                        } else { // File has reached limit, close and return
                            csv_file_.flush();
                            csv_file_.close();
                            return;
                        }
                    }
                }
            }
            if (counter_ != last_counter_) {
                if (json_in.contains("data")) {
                    if (!json_in["data"].empty()) {
                        int dIndex = 0;
                        for (auto &d : json_in["data"]) {
                            if (save_timestamp_)
                                csv_file_ << epochTimeMillis << ",";
                            if (save_counter_)
                                csv_file_ << counter_ << ",";
                            if (json_in.contains("ref_frame")) {
                                csv_file_ << json_in["ref_frame"]["w"].get<int>() << "x" << json_in["ref_frame"]["h"].get<int>() << ",";
                            }
                            if (json_in.contains("data_type")) {
                                csv_file_ << json_in["data_type"].get<std::string>() << ",";
                            }
                            csv_file_ << dIndex << ",";
                            for (nlohmann::json::iterator it = d.begin(); it != d.end(); ++it) {
                                if (it->is_number_float())
                                    csv_file_ << it->get<float>() << ",";
                                else if (it->is_number_integer())
                                    csv_file_ << it->get<int>() << ",";
                                else if (it->is_boolean()) {
                                    csv_file_ << it->get<bool>() << ",";
                                } else if (it->is_array()) {
                                    for (int i = 0; i < it->size(); i++) {
                                        if (it->at(i).is_number_float())
                                            csv_file_ << it->at(i).get<float>() << ",";
                                        else if (it->is_number_integer())
                                            csv_file_ << it->at(i).get<int>() << ",";
                                    }
                                } else if (it->is_object()) { // TODO: Handle Deeper Level JSON Objects

                                }
                            }
                            csv_file_ << "\n";
                            csv_file_.flush();
                            dIndex++;
                        }
                    }
                }
                last_counter_ = counter_;
                if (!in2)
                    counter_++;
            }
        }
    }
}

bool CsvFile::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void CsvFile::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Save CSV File", GetInstanceName()).c_str())) {
            show_file_dialog_ = true;
        }
        ImGui::Text("CSV File:");
        if (csv_file_path_.empty())
            ImGui::Text("[None]");
        else
            ImGui::TextWrapped("%s", csv_file_path_.c_str());

        if(show_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Save CSV", GetInstanceName()).c_str());

        if(file_dialog_.showFileDialog(CreateControlString("Save CSV", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".csv", &show_file_dialog_))
        {
            auto loc = file_dialog_.selected_path.find_last_of('.');
            if (loc == std::string::npos) {
                csv_file_path_ = file_dialog_.selected_path;
                csv_file_path_ += ".csv";
            }
            else {
                csv_file_path_ = file_dialog_.selected_path;
            }
            show_file_dialog_ = false;
            start_new_file_ = true;
        }
        ImGui::Separator();
        bool newFile = false;
        if(ImGui::Checkbox(CreateControlString("Save Timestamp", GetInstanceName()).c_str(), &save_timestamp_)) {
            newFile = true;
        }
        if (ImGui::Checkbox(CreateControlString("Save Index Count", GetInstanceName()).c_str(), &save_counter_)) {
            newFile = true;
        }
        ImGui::SetNextItemWidth(120);
        if (ImGui::Checkbox(CreateControlString("Limit File Size", GetInstanceName()).c_str(), &limit_file_size_)) {
            newFile = true;
        }
        if (limit_file_size_) {
            ImGui::SetNextItemWidth(120);
            ImGui::DragInt(CreateControlString("Max Size KB", GetInstanceName()).c_str(), &max_file_size_kb_, 0.5f, 100, INT_MAX);
            if (ImGui::Checkbox(CreateControlString("Use Rotating Files", GetInstanceName()).c_str(), &rotating_files_)) {
                newFile = true;
            }
            if (rotating_files_) {
                ImGui::SetNextItemWidth(120);
                if (ImGui::DragInt(CreateControlString("Num Files", GetInstanceName()).c_str(), &num_files_, 0.5f, 2, 100)) {
                    newFile = true;
                }
            }
        }
        if (newFile)
            start_new_file_ = true;
    }

}

std::string CsvFile::GetState()
{
    using namespace nlohmann;

    json state;

    state["save_timestamp"] = save_timestamp_;
    state["save_counter"] = save_counter_;
    state["csv_file_path"] = csv_file_path_;
    state["limit_file_size"] = limit_file_size_;
    state["max_file_size_kb"] = max_file_size_kb_;
    state["rotating_files"] = rotating_files_;
    state["num_files"] = num_files_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void CsvFile::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("save_timestamp"))
        save_timestamp_ = state["save_timestamp"].get<bool>();
    if (state.contains("save_counter"))
        save_counter_ = state["save_counter"].get<bool>();
    if (state.contains("csv_file_path"))
        csv_file_path_ = state["csv_file_path"].get<std::string>();
    if (state.contains("limit_file_size"))
        limit_file_size_ = state["limit_file_size"].get<bool>();
    if (state.contains("max_file_size_kb"))
        max_file_size_kb_ = state["max_file_size_kb"].get<int>();
    if (state.contains("rotating_files"))
        rotating_files_ = state["rotating_files"].get<bool>();
    if (state.contains("num_files"))
        num_files_ = state["num_files"].get<int>();

    if (!csv_file_path_.empty())
        start_new_file_ = true;
}

