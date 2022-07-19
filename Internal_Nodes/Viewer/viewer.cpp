//
// Created by Richard on 1/3/2022.
//

#include "viewer.hpp"

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{
    Viewer::Viewer()
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
        std::lock_guard<std::mutex> lck (io_mutex_);
        auto *imCurContext = (ImGuiContext *)context;
        ImGui::SetCurrentContext(imCurContext);

        if (interface == (int)FlowCV::GuiInterfaceType_Main) {
            std::string title = "Viewer_" + std::to_string(GetInstanceCount());
            if (!frame_.empty()) {
                viewer_.Update(title.c_str(), frame_, ImOpenCvWindowAspectFlag_LockH);
            }
            else {
                cv::Mat frame(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
                viewer_.Update(title.c_str(), frame, ImOpenCvWindowAspectFlag_LockH);
            }
        }
    }

    void Viewer::Process_( SignalBus const& inputs, SignalBus& outputs )
    {
        if (!IsEnabled())
            SetEnabled(true);

        std::lock_guard<std::mutex> lck (io_mutex_);
        auto in1 = inputs.GetValue<cv::Mat>( 0 );
        if ( !in1 )
        {
            return;
        }

        // Do something with Input
        if (!in1->empty()) {
            in1->copyTo(frame_);
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
