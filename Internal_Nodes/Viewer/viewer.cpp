//
// Created by Richard on 1/3/2022.
//

#include "viewer.hpp"

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{
    Viewer::Viewer()
        : Component( ProcessOrder::InOrder )
    {
        // Name and Category
        SetComponentName_("Viewer");
        SetComponentCategory_(DSPatch::Category::Category_Views);
        SetComponentAuthor_("Richard");
        SetComponentVersion_("0.1.0");
        SetInstanceCount(global_inst_counter);
        global_inst_counter++;

        SetInputCount_( 1, {"in"}, {DSPatch::IoType::Io_Type_CvMat} );
        // add 1 output
        SetOutputCount_( 0 );

        frame_ = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
        has_update_ = true;
        SetEnabled(true);
    }

    void Viewer::Process_( SignalBus const& inputs, SignalBus& outputs ) {
        if (!IsEnabled())
            SetEnabled(true);

        auto in1 = inputs.GetValue<cv::Mat>(0);
        if (io_mutex_.try_lock()) {
            if (!in1) {
                std::chrono::steady_clock::time_point current_time_ = std::chrono::steady_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - last_input_update_).count();
                if (delta > 500) {
                    if (!frame_.empty())
                        frame_.setTo(cv::Scalar(0));
                    else
                        frame_ = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
                    io_mutex_.unlock();
                    return;
                }
            }
            else {
                try {
                    if (!in1->empty()) {
                        in1->copyTo(frame_);
                        has_update_ = true;
                        last_input_update_ = std::chrono::steady_clock::now();
                    }
                }
                catch (const std::exception &e) { std::cout << e.what() << std::endl; }
            }
            io_mutex_.unlock();
        }
    }

    bool Viewer::HasGui(int interface)
    {
        if (interface == (int)FlowCV::GuiInterfaceType_Main) {
            return true;
        }

        return false;
    }

    void Viewer::UpdateGui(void *context, int interface)
    {
        auto *imCurContext = (ImGuiContext *)context;
        ImGui::SetCurrentContext(imCurContext);

        if (interface == (int)FlowCV::GuiInterfaceType_Main) {

            std::string title = "Viewer_" + std::to_string(GetInstanceCount());
            if (!frame_.empty() && has_update_) {
                cv::Mat frame;
                io_mutex_.lock();
                frame_.copyTo(frame);
                has_update_ = false;
                io_mutex_.unlock();
                viewer_.Update(title.c_str(), frame, ImOpenCvWindowAspectFlag_LockH);
            }
            else {
                cv::Mat frame;
                io_mutex_.lock();
                frame_.copyTo(frame);
                io_mutex_.unlock();
                viewer_.Update(title.c_str(), frame, ImOpenCvWindowAspectFlag_LockH);
            }
        }
    }

    std::string Viewer::GetState()
    {

        std::string stateSerialized;

        return stateSerialized;
    }

    void Viewer::SetState(std::string &&json_serialized)
    {


    }

}  // namespace DSPatch
