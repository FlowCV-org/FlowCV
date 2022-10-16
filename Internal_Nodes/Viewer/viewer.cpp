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
                io_mutex_.lock();
                cv::Mat frame;
                frame_.copyTo(frame);
                has_update_ = false;
                io_mutex_.unlock();
                viewer_.Update(title.c_str(), frame, ImOpenCvWindowAspectFlag_LockH);
            }
            else {
                viewer_.Update(title.c_str(), frame_, ImOpenCvWindowAspectFlag_LockH);
            }

        }
    }

    void Viewer::Process_( SignalBus const& inputs, SignalBus& outputs ) {
        if (!IsEnabled())
            SetEnabled(true);

        if (io_mutex_.try_lock()) {
            auto in1 = inputs.GetValue<cv::Mat>(0);
            if (!in1) {
                frame_ = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
                has_update_ = true;
                io_mutex_.unlock();
                return;
            }

            try {
                if (!in1->empty()) {
                    in1->copyTo(frame_);
                    has_update_ = true;
                }
            }
            catch (const std::exception &e) { std::cout << e.what() << std::endl; }
            io_mutex_.unlock();
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
