//
// Plugin SimpleBlobTracker
//

#include "simple_blob_tracker.hpp"
#include <random>

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class SimpleBlobTracker
{
};
}  // namespace DSPatch::DSPatchables::internal

static cv::Vec3b GenerateRandomColor()
{
    static std::random_device rd;                     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());                           // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(10, 255);  // guaranteed unbiased

    cv::Vec3b color;

    color[0] = uni(rng);
    color[1] = uni(rng);
    color[2] = uni(rng);

    return color;
}

SimpleBlobTracker::SimpleBlobTracker() : Component(ProcessOrder::OutOfOrder), p(new internal::SimpleBlobTracker())
{
    // Name and Category
    SetComponentName_("Simple_Blob_Tracker");
    SetComponentCategory_(Category::Category_Feature_Detection);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 2 inputs
    SetInputCount_(3, {"in", "start", "blobs"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Bool, IoType::Io_Type_JSON});

    // 2 outputs
    SetOutputCount_(2, {"vis", "tracks"}, {IoType::Io_Type_CvMat, IoType::Io_Type_JSON});

    reset_tracking_ = false;
    id_counter_ = 1;
    frame_count_ = 0;
    show_tracking_ = true;
    show_ids_ = true;
    tracking_duration_ = 300;
    drop_lost_tracks_ = true;
    lost_track_duration_ = 120;
    max_pos_var_ = 8;
    max_size_var_ = 8;

    SetEnabled(true);
}

void SimpleBlobTracker::Process_(SignalBus const &inputs, SignalBus &outputs)
{
    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>(0);
    auto in2 = inputs.GetValue<bool>(1);
    auto in3 = inputs.GetValue<nlohmann::json>(2);
    if (!in1) {
        return;
    }

    if (in2) {
        bool isStart = *in2;
        if (isStart) {
            reset_tracking_ = true;
        }
    }

    nlohmann::json json_data;
    if (in3) {
        if (!in3->empty()) {
            json_data = *in3;
        }
    }

    if (reset_tracking_) {
        reset_tracking_ = false;
        id_counter_ = 1;
        frame_count_ = 0;
        blob_tracking_.clear();
    }

    if (!in1->empty()) {
        if (IsEnabled()) {
            cv::Mat frame;
            // Process Image
            in1->copyTo(frame);

            int refWidth = -1;
            int refHeight = -1;
            if (!json_data.empty()) {
                if (json_data.contains("ref_frame")) {
                    refWidth = json_data["ref_frame"]["w"].get<int>();
                    refHeight = json_data["ref_frame"]["h"].get<int>();
                }
                if (json_data.contains("data")) {
                    for (auto &d : json_data["data"]) {
                        if (d.contains("x") && d.contains("y") && d.contains("size")) {
                            // Get Current Blob Info
                            cv::Point curPos;
                            curPos.x = (int)d["x"].get<float>();
                            curPos.y = (int)d["y"].get<float>();
                            int curSize = (int)d["size"].get<float>();

                            // Search for Match
                            int foundMatch = -1;
                            double foundDist = max_pos_var_;
                            for (int i = 0; i < blob_tracking_.size(); i++) {
                                double res = cv::norm(curPos - blob_tracking_.at(i).pos.at(blob_tracking_.at(i).pos.size() - 1));
                                if (res < foundDist) {
                                    foundDist = res;
                                    if (blob_tracking_.at(i).pos.size() > 1) {
                                        if (curSize > (blob_tracking_.at(i).last_size - max_size_var_) &&
                                            curSize < (blob_tracking_.at(i).last_size + max_size_var_)) {
                                            if ((frame_count_ - blob_tracking_.at(i).last_frame) < lost_track_duration_)
                                                foundMatch = i;
                                        }
                                    }
                                    else {
                                        foundMatch = i;
                                    }
                                }
                            }
                            if (foundMatch == -1) {
                                // Add new Track
                                internal::BlobTrackingData bt;
                                bt.id = id_counter_;
                                bt.pos.emplace_back(curPos);
                                bt.color = GenerateRandomColor();
                                bt.last_size = curSize;
                                bt.last_frame = frame_count_;
                                blob_tracking_.emplace_back(bt);
                                id_counter_++;
                            }
                            else {
                                // Add to Position
                                blob_tracking_.at(foundMatch).pos.emplace_back(curPos);
                                blob_tracking_.at(foundMatch).last_size = curSize;
                                blob_tracking_.at(foundMatch).last_frame = frame_count_;
                            }
                        }
                    }
                }
            }
            nlohmann::json json_out;
            nlohmann::json jTracks;
            json_out["data_type"] = "tracking";

            nlohmann::json ref;
            if (refWidth != -1 && refHeight != -1) {
                ref["w"] = refWidth;
                ref["h"] = refHeight;
                json_out["ref_frame"] = ref;
            }
            else if (!frame.empty()) {
                ref["w"] = frame.cols;
                ref["h"] = frame.rows;
                json_out["ref_frame"] = ref;
            }

            if (drop_lost_tracks_) {
                // Clean up lost tracks
                for (int i = 0; i < blob_tracking_.size(); i++) {
                    if ((frame_count_ - blob_tracking_.at(i).last_frame) > lost_track_duration_) {
                        blob_tracking_.erase(blob_tracking_.begin() + i);
                    }
                }
            }

            // Draw Tracking Lines
            for (auto &blob : blob_tracking_) {
                if (blob.pos.size() > 1) {
                    if (blob.pos.size() > tracking_duration_)
                        blob.pos.pop_front();
                    nlohmann::json cTmp;
                    cTmp["id"] = blob.id;
                    cTmp["x"] = blob.pos.at(blob.pos.size() - 1).x;
                    cTmp["y"] = blob.pos.at(blob.pos.size() - 1).y;
                    cTmp["size"] = blob.last_size;
                    jTracks.emplace_back(cTmp);
                    if (show_tracking_) {
                        for (int i = 1; i < blob.pos.size(); i++) {
                            cv::line(frame, blob.pos.at(i - 1), blob.pos.at(i), blob.color, 2);
                        }
                    }
                }
            }
            if (show_ids_) {
                // Draw IDs on Top
                for (auto &blob : blob_tracking_) {
                    std::string id = std::to_string(blob.id);
                    cv::putText(frame, id.c_str(), blob.pos.at(blob.pos.size() - 1), cv::FONT_HERSHEY_SIMPLEX, 0.75, blob.color, 2.0);
                }
            }
            if (!frame.empty()) {
                outputs.SetValue(0, frame);
                json_out["data"] = jTracks;
                outputs.SetValue(1, json_out);
                frame_count_++;
            }
        }
        else {
            outputs.SetValue(0, *in1);
        }
    }
}

bool SimpleBlobTracker::HasGui(int interface)
{
    // When Creating Strings for Controls use: CreateControlString("Text Here", GetInstanceCount()).c_str()
    // This will ensure a unique control name for ImGui with multiple instance of the Plugin
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void SimpleBlobTracker::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("Reset Tracking", GetInstanceName()).c_str())) {
            reset_tracking_ = true;
        }
        ImGui::Separator();
        ImGui::Checkbox(CreateControlString("Show Tracking", GetInstanceName()).c_str(), &show_tracking_);
        ImGui::Checkbox(CreateControlString("Show IDs", GetInstanceName()).c_str(), &show_ids_);
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Tracking Duration (Frames)", GetInstanceName()).c_str(), &tracking_duration_, 0.5f, 2, 3000);
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Lost Track Duration (Frames)", GetInstanceName()).c_str(), &lost_track_duration_, 0.5f, 2, 300);
        ImGui::Checkbox(CreateControlString("Drop Lost Tracks", GetInstanceName()).c_str(), &drop_lost_tracks_);
        ImGui::SetNextItemWidth(80);
        ImGui::Separator();
        ImGui::TextUnformatted("Track Matching Parameters:");
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Max Position Diff", GetInstanceName()).c_str(), &max_pos_var_, 0.25f, 1, 1000);
        ImGui::SetNextItemWidth(80);
        ImGui::DragInt(CreateControlString("Max Scale Diff", GetInstanceName()).c_str(), &max_size_var_, 0.25f, 1, 1000);
    }
}

std::string SimpleBlobTracker::GetState()
{
    using namespace nlohmann;

    json state;

    state["show_tracking"] = show_tracking_;
    state["show_ids"] = show_ids_;
    state["tracking_duration"] = tracking_duration_;
    state["drop_lost_tracks"] = drop_lost_tracks_;
    state["lost_track_duration"] = lost_track_duration_;
    state["max_pos_var"] = max_pos_var_;
    state["max_size_var"] = max_size_var_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void SimpleBlobTracker::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("show_tracking"))
        show_tracking_ = state["show_tracking"].get<bool>();
    if (state.contains("show_ids"))
        show_ids_ = state["show_ids"].get<bool>();
    if (state.contains("tracking_duration"))
        tracking_duration_ = state["tracking_duration"].get<int>();
    if (state.contains("drop_lost_tracks"))
        drop_lost_tracks_ = state["drop_lost_tracks"].get<bool>();
    if (state.contains("lost_track_duration"))
        lost_track_duration_ = state["lost_track_duration"].get<int>();
    if (state.contains("max_pos_var"))
        max_pos_var_ = state["max_pos_var"].get<int>();
    if (state.contains("max_size_var"))
        max_size_var_ = state["max_size_var"].get<int>();
}
