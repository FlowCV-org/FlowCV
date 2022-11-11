//
// FlowCV Node Properties
//

#ifndef FLOWCV_PROPERTY_MANAGER_HPP_
#define FLOWCV_PROPERTY_MANAGER_HPP_
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <json.hpp>

namespace FlowCV {

enum class PropertyDataTypes
{
    kDataTypeUndefined = 0,
    kDataTypeBool,
    kDataTypeInt,
    kDataTypeFloat,
    kDataTypeOption,
    kDataTypeSeparator
};

struct PropertyValueRange
{
    std::vector<uint8_t> min;
    std::vector<uint8_t> max;
    std::vector<uint8_t> step;
};

struct DataStruct
{
    std::string key;
    PropertyDataTypes data_type;
    std::vector<uint8_t> d_val;
    std::vector<uint8_t> w_val;
    std::vector<uint8_t> r_val;
    PropertyValueRange range;
    std::vector<std::string> options;
    std::string desc;
    bool visibility;
    bool changed;
};

class FlowCV_Properties
{
  public:
    FlowCV_Properties();
    void AddBool(std::string &&key, std::string&& desc, bool value, bool visible = true);
    void AddInt(std::string &&key, std::string&& desc, int value, int min = 0, int max = 100, float step = 0.5f, bool visible = true);
    void AddFloat(std::string &&key, std::string&& desc, float value, float min = 0.0f, float max = 100.0f, float step = 0.1f, bool visible = true);
    void AddOption(std::string &&key, std::string&& desc, int value, std::vector<std::string> options, bool visible = true);
    void Remove(std::string &&key);
    void RemoveAll();
    void Set(std::string &&key, bool value);
    void Set(std::string &&key, int value);
    void Set(std::string &&key, float value);
    void SetMin(std::string &&key, int value);
    void SetMax(std::string &&key, int value);
    void SetMin(std::string &&key, float value);
    void SetMax(std::string &&key, float value);
    void SetStep(std::string &&key, float value);
    void SetVisibility(std::string &&key, bool show);
    void SetToDefault(std::string &&key);
    void SetAllToDefault();
    bool Exists(const std::string &key);
    bool Changed(const std::string &key);
    std::shared_ptr<std::vector<DataStruct>> GetAll();
    template<typename T> T *GetPointer(const std::string &key);
    template<typename T> T Get(const std::string &key);
    template<typename T> T GetW(const std::string &key);
    template<typename T> T GetMin(const std::string &key);
    template<typename T> T GetMax(const std::string &key);
    template<typename T> T GetStep(const std::string &key);
    void DrawUi(const char* inst_id);
    void ToJson(nlohmann::json &j);
    void FromJson(nlohmann::json &j);
    const std::vector<std::string> &GetOptions(std::string &&key);
    void Sync();

  private:
    bool has_changes_;
    std::shared_ptr<std::vector<DataStruct>> props_;
    std::unordered_map<std::string, int> prop_idx;
    std::mutex mutex_lock_;

}; // End Class Node_Properties
} // End Namespace FlowCV
#endif //FLOWCV_PROPERTY_MANAGER_HPP_
