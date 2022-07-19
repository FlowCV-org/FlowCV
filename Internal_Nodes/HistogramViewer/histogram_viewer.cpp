//
// Plugin HistogramViewer
//

#include "histogram_viewer.hpp"

using namespace DSPatch;
using namespace DSPatchables;

static int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables
{

HistogramViewer::HistogramViewer()
    : Component( ProcessOrder::OutOfOrder )
{
    // Name and Category
    SetComponentName_("Histogram");
    SetComponentCategory_(DSPatch::Category::Category_Views);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 1, {"in"}, {IoType::Io_Type_CvMat} );

    // 0 outputs
    SetOutputCount_(0);

    has_input_ = false;
    is_color_ = true;

    SetEnabled(true);

}

void HistogramViewer::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    if (!IsEnabled())
        SetEnabled(true);

    // Input 1 Handler
    auto in1 = inputs.GetValue<cv::Mat>( 0 );
    if ( !in1 ) {
        return;
    }
    std::lock_guard<std::mutex> lck (io_mutex_);

    if (!in1->empty()) {
        has_input_ = true;
        if (in1->channels() > 1)
            is_color_ = true;
        else
            is_color_ = false;

        int hist_size = 256;
        float range[] = { 0, 256 }; //the upper boundary is exclusive
        const float* hist_range = { range };
        bool uniform = true, accumulate = false;
        std::vector<cv::Mat> bgr_planes;
        x_range_.clear();
        values_r_.clear();
        values_g_.clear();
        values_b_.clear();

        split( *in1, bgr_planes );
        cv::Mat b_hist, g_hist, r_hist;

        calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &hist_size, &hist_range, uniform, accumulate);
        normalize(b_hist, b_hist, 0, bgr_planes[0].rows, cv::NORM_MINMAX, -1, cv::Mat());
        if (is_color_) {
            calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &hist_size, &hist_range, uniform, accumulate);
            normalize(g_hist, g_hist, 0, bgr_planes[1].rows, cv::NORM_MINMAX, -1, cv::Mat());
            calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &hist_size, &hist_range, uniform, accumulate);
            normalize(r_hist, r_hist, 0, bgr_planes[2].rows, cv::NORM_MINMAX, -1, cv::Mat());
        }

        for( int i = 1; i < hist_size; i++ )
        {
            x_range_.emplace_back((float)i);
            values_b_.emplace_back(b_hist.at<float>(i-1));
            if (is_color_) {
                values_r_.emplace_back(r_hist.at<float>(i - 1));
                values_g_.emplace_back(g_hist.at<float>(i - 1));
            }
        }
    }
    else
        has_input_ = false;
}

bool HistogramViewer::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        return true;
    }

    return false;
}

void HistogramViewer::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Main) {
        std::lock_guard<std::mutex> lck (io_mutex_);
        std::string title = "Histogram_" + std::to_string(GetInstanceCount());
        ImGui::Begin(CreateControlString(title.c_str(), GetInstanceName()).c_str());
        if (ImPlot::BeginPlot(CreateControlString("Histogram View", GetInstanceName()).c_str(), ImVec2(-1, -1))) {
            ImPlot::SetupAxes("Range","Value");
            ImPlot::SetupAxesLimits(0,255,0,1024);
            if (has_input_ && !x_range_.empty()) {
                std::string singleChannelName = "Gray";
                ImVec4 singleChannelColor = ImVec4(0.75, 0.75, 0.75, 0.75);
                if (is_color_) {
                    singleChannelName = "Blue";
                    singleChannelColor = ImVec4(0.0, 0.0, 1.0, 0.5);

                    ImPlot::SetNextFillStyle(ImVec4(1.0, 0.0, 0.0, 0.5));
                    ImPlot::PlotShaded<float>("Red", x_range_.data(), values_r_.data(), 255, 0);
                    ImPlot::PlotLine<float>("Red", x_range_.data(), values_r_.data(), 255);

                    ImPlot::SetNextFillStyle(ImVec4(0.0, 1.0, 0.0, 0.5));
                    ImPlot::PlotShaded<float>("Green", x_range_.data(), values_g_.data(), 255, 0);
                    ImPlot::PlotLine<float>("Green", x_range_.data(), values_g_.data(), 255);
                }
                ImPlot::SetNextFillStyle(singleChannelColor);
                ImPlot::PlotShaded<float>(singleChannelName.c_str(), x_range_.data(), values_b_.data(), 255, 0);
                ImPlot::PlotLine<float>(singleChannelName.c_str(), x_range_.data(), values_b_.data(), 255);
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
    }
}

std::string HistogramViewer::GetState()
{
    using namespace nlohmann;

    json state;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void HistogramViewer::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);


}

} // End Namespace DSPatch::DSPatchables