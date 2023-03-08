//
// Plugin CsvFile
//

#ifndef FLOWCV_PLUGIN_CSV_FILE_HPP_
#define FLOWCV_PLUGIN_CSV_FILE_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include "imgui_wrapper.hpp"
#include "imgui_opencv.hpp"
#include "json.hpp"
#include <chrono>
#include <fstream>
#include <ImGuiFileBrowser.h>
#include <filesystem>

namespace DSPatch::DSPatchables
{
namespace internal
{
class CsvFile;
}

class DLLEXPORT CsvFile final : public Component
{
  public:
    CsvFile();
    ~CsvFile() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_(SignalBus const &inputs, SignalBus &outputs) override;
    void OpenCsvFile(nlohmann::json &json_data);
    std::string GetRotateFilePath();

  private:
    std::unique_ptr<internal::CsvFile> p;
    bool start_new_file_;
    int counter_;
    int last_counter_;
    bool save_timestamp_;
    bool save_counter_;
    std::string csv_file_path_;
    int csv_cur_file_num_;
    std::ofstream csv_file_;
    bool show_file_dialog_;
    bool limit_file_size_;
    int max_file_size_kb_;
    imgui_addons::ImGuiFileBrowser file_dialog_;
    bool rotating_files_;
    int num_files_;
};

EXPORT_PLUGIN(CsvFile)

}  // namespace DSPatch::DSPatchables
#endif  // FLOWCV_PLUGIN_CSV_FILE_HPP_
