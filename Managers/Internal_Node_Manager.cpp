//
// FlowCV Internal Component Manager
//

#include "Internal_Node_Manager.hpp"
#include "internal_nodes.hpp"

namespace FlowCV {

template<class T>
static NodeDescription GetCompInfo(T nodeClass) {
    NodeDescription nodeDesc;
    DSPatch::Component &comp = nodeClass;
    nodeDesc.name = comp.GetComponentName();
    nodeDesc.version = comp.GetComponentVersion();
    nodeDesc.author = comp.GetComponentAuthor();
    nodeDesc.category = comp.GetComponentCategory();
    nodeDesc.input_count = comp.GetInputCount();
    nodeDesc.output_count = comp.GetOutputCount();

    return nodeDesc;
}

static bool compareName(const NodeDescription& a, const NodeDescription& b)
{
    return a.name < b.name;
}

InternalNodeManager::InternalNodeManager() {

    // Add Internal Components Here

    // Viewer
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Viewer>(DSPatch::DSPatchables::Viewer()));

    // Blur
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Blur>(DSPatch::DSPatchables::Blur()));

    // Transform
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Transform>(DSPatch::DSPatchables::Transform()));

    // Canny
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::CannyFilter>(DSPatch::DSPatchables::CannyFilter()));

    // Hough Circles
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::HCircles>(DSPatch::DSPatchables::HCircles()));

    // Blob Detector
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::BlobDetector>(DSPatch::DSPatchables::BlobDetector()));

    // JSON Viewer
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::JsonViewer>(DSPatch::DSPatchables::JsonViewer()));

    // Morphological Operations
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Morphology>(DSPatch::DSPatchables::Morphology()));

    // Convert Color
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ConvertColor>(DSPatch::DSPatchables::ConvertColor()));

    // Threshold
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Threshold>(DSPatch::DSPatchables::Threshold()));

    // Normalize
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Normalize>(DSPatch::DSPatchables::Normalize()));

    // Background Subtraction
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::BackgroundSubtraction>(DSPatch::DSPatchables::BackgroundSubtraction()));

    // Bitwise
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Bitwise>(DSPatch::DSPatchables::Bitwise()));

    // Contours
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Contours>(DSPatch::DSPatchables::Contours()));

    // Crop
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Crop>(DSPatch::DSPatchables::Crop()));

    // Color Correct
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Colorcorrect>(DSPatch::DSPatchables::Colorcorrect()));

    // Draw Text
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DrawText>(DSPatch::DSPatchables::DrawText()));

    // Draw Number
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DrawNumber>(DSPatch::DSPatchables::DrawNumber()));

    // Draw Date Time
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DrawDateTime>(DSPatch::DSPatchables::DrawDateTime()));

    // Draw JSON
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DrawJson>(DSPatch::DSPatchables::DrawJson()));

    // Resize
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Resize>(DSPatch::DSPatchables::Resize()));

    // Get Size
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::GetSize>(DSPatch::DSPatchables::GetSize()));

    // Get Optimal DFT Size
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::GetOptimalDft>(DSPatch::DSPatchables::GetOptimalDft()));

    // Discrete Fourier Transform
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DiscreteFourierTransform>(DSPatch::DSPatchables::DiscreteFourierTransform()));

    // Discrete Cosine Transform
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DiscreteCosineTransform>(DSPatch::DSPatchables::DiscreteCosineTransform()));

    // Copy Make Border
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::CopyMakeBorder>(DSPatch::DSPatchables::CopyMakeBorder()));

    // Mean
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Mean>(DSPatch::DSPatchables::Mean()));

    // Scharr
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ScharrFilter>(DSPatch::DSPatchables::ScharrFilter()));

    // Sobel
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::SobelFilter>(DSPatch::DSPatchables::SobelFilter()));

    // Laplacian
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::LaplacianFilter>(DSPatch::DSPatchables::LaplacianFilter()));

    // Convert Scale Abs
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ScaleAbs>(DSPatch::DSPatchables::ScaleAbs()));

    // Add Weighted
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::AddWeighted>(DSPatch::DSPatchables::AddWeighted()));

    // Add
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Add>(DSPatch::DSPatchables::Add()));

    // Abs Diff
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::AbsDiff>(DSPatch::DSPatchables::AbsDiff()));

    // Divide
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Divide>(DSPatch::DSPatchables::Divide()));

    // Max
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Max>(DSPatch::DSPatchables::Max()));

    // Min
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Min>(DSPatch::DSPatchables::Min()));

    // Multiply
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Multiply>(DSPatch::DSPatchables::Multiply()));

    // Subtract
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Subtract>(DSPatch::DSPatchables::Subtract()));

    // Magnitude
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Magnitude>(DSPatch::DSPatchables::Magnitude()));

    // Random Noise
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::RndNoise>(DSPatch::DSPatchables::RndNoise()));

    // Histogram Viewer
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::HistogramViewer>(DSPatch::DSPatchables::HistogramViewer()));

    // Color Reduce
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ColorReduce>(DSPatch::DSPatchables::ColorReduce()));

    // Depth Viewer 3D
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::DepthViewer3D>(DSPatch::DSPatchables::DepthViewer3D()));

    // Hough Lines
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::HoughLines>(DSPatch::DSPatchables::HoughLines()));

    // Solid
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Solid>(DSPatch::DSPatchables::Solid()));

    // Line Intersect
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::LineIntersect>(DSPatch::DSPatchables::LineIntersect()));

    // Perspective Warp
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::PerspectiveWarp>(DSPatch::DSPatchables::PerspectiveWarp()));

    // Sharpen
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Sharpen>(DSPatch::DSPatchables::Sharpen()));

    // Split
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Split>(DSPatch::DSPatchables::Split()));

    // Combine
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Combine>(DSPatch::DSPatchables::Combine()));

    // DNN - Classification
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Classification>(DSPatch::DSPatchables::Classification()));

    // DNN - Object Detection
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ObjectDetection>(DSPatch::DSPatchables::ObjectDetection()));

    // DNN - Segmentation
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::Segmentation>(DSPatch::DSPatchables::Segmentation()));

    // DNN - Human Pose
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::HumanPose>(DSPatch::DSPatchables::HumanPose()));

    // DNN - Text Detection
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::TextDetection>(DSPatch::DSPatchables::TextDetection()));

    // DNN - Text Recognition
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::TextRecognition>(DSPatch::DSPatchables::TextRecognition()));

    // DNN - Image Processing
    node_list_.emplace_back(GetCompInfo<DSPatch::DSPatchables::ImageProcessing>(DSPatch::DSPatchables::ImageProcessing()));

    // ...

    // Sort Nodes
    std::sort(node_list_.begin(), node_list_.end(), compareName);

}

uint32_t InternalNodeManager::NodeCount() {
    return node_list_.size();
}

bool InternalNodeManager::HasNode(const char *name) {
    for (auto const &p: node_list_) {
        if (p.name == name) {
            return true;
        }
    }

    return false;
}

bool InternalNodeManager::GetNodeDescription(uint32_t index, NodeDescription &nodeDesc) {

    if (index >= 0 && index < node_list_.size()) {
        nodeDesc.name = node_list_.at(index).name;
        nodeDesc.category = node_list_.at(index).category;
        nodeDesc.author = node_list_.at(index).author;
        nodeDesc.version = node_list_.at(index).version;
        nodeDesc.input_count = node_list_.at(index).input_count;
        nodeDesc.output_count = node_list_.at(index).output_count;
        return true;
    }

    return false;
}

bool InternalNodeManager::GetNodeDescription(const char *name, NodeDescription &nodeDesc) {

    for (auto const &p: node_list_) {
        if (p.name == name) {
            nodeDesc.name = p.name;
            nodeDesc.category = p.category;
            nodeDesc.author = p.author;
            nodeDesc.version = p.version;
            nodeDesc.input_count = p.input_count;
            nodeDesc.output_count = p.output_count;
            return true;
        }
    }

    return false;
}

std::shared_ptr<DSPatch::Component> InternalNodeManager::CreateNodeInstance(const char *name) {
    for (auto const &p: node_list_) {
        if (p.name == name) {

            // Handle Instantiation of Internal Classes Here

            // Viewer
            if (p.name == "Viewer")
                return std::make_shared<DSPatch::DSPatchables::Viewer>();

            // Blur
            if (p.name == "Blur")
                return std::make_shared<DSPatch::DSPatchables::Blur>();

            // Transform
            if (p.name == "Transform")
                return std::make_shared<DSPatch::DSPatchables::Transform>();

            // Canny
            if (p.name == "Canny")
                return std::make_shared<DSPatch::DSPatchables::CannyFilter>();

            // Hough Circles
            if (p.name == "Hough_Circles")
                return std::make_shared<DSPatch::DSPatchables::HCircles>();

            // Blob Detector
            if (p.name == "Blob_Detector")
                return std::make_shared<DSPatch::DSPatchables::BlobDetector>();

            // JSON Viewer
            if (p.name == "Json_Viewer")
                return std::make_shared<DSPatch::DSPatchables::JsonViewer>();

            // Morphological Operations
            if (p.name == "Morphology")
                return std::make_shared<DSPatch::DSPatchables::Morphology>();

            // Convert Color
            if (p.name == "Convert_Color")
                return std::make_shared<DSPatch::DSPatchables::ConvertColor>();

            // Threshold
            if (p.name == "Threshold")
                return std::make_shared<DSPatch::DSPatchables::Threshold>();

            // Normalize
            if (p.name == "Normalize")
                return std::make_shared<DSPatch::DSPatchables::Normalize>();

            // Background Subtraction
            if (p.name == "Background_Subtraction")
                return std::make_shared<DSPatch::DSPatchables::BackgroundSubtraction>();

            // Bitwise
            if (p.name == "Bitwise")
                return std::make_shared<DSPatch::DSPatchables::Bitwise>();

            // Contours
            if (p.name == "Contours")
                return std::make_shared<DSPatch::DSPatchables::Contours>();

            // Crop
            if (p.name == "Crop")
                return std::make_shared<DSPatch::DSPatchables::Crop>();

            // Color Correct
            if (p.name == "Color_Correct")
                return std::make_shared<DSPatch::DSPatchables::Colorcorrect>();

            // Draw Text
            if (p.name == "Draw_Text")
                return std::make_shared<DSPatch::DSPatchables::DrawText>();

            // Draw Number
            if (p.name == "Draw_Number")
                return std::make_shared<DSPatch::DSPatchables::DrawNumber>();

            // Draw Date Time
            if (p.name == "Draw_Date_Time")
                return std::make_shared<DSPatch::DSPatchables::DrawDateTime>();

            // Draw JSON
            if (p.name == "Draw_JSON")
                return std::make_shared<DSPatch::DSPatchables::DrawJson>();

            // Resize
            if (p.name == "Resize")
                return std::make_shared<DSPatch::DSPatchables::Resize>();

            // Get Size
            if (p.name == "Get_Size")
                return std::make_shared<DSPatch::DSPatchables::GetSize>();

            // Get Optimal DFT Size
            if (p.name == "Get_Optimal_DFT_Size")
                return std::make_shared<DSPatch::DSPatchables::GetOptimalDft>();

            // Discrete Fourier Transform
            if (p.name == "DFT")
                return std::make_shared<DSPatch::DSPatchables::DiscreteFourierTransform>();

            // Discrete Cosine Transform
            if (p.name == "DCT")
                return std::make_shared<DSPatch::DSPatchables::DiscreteCosineTransform>();

            // Copy Make Border
            if (p.name == "Copy_Make_Border")
                return std::make_shared<DSPatch::DSPatchables::CopyMakeBorder>();

            // Mean
            if (p.name == "Mean")
                return std::make_shared<DSPatch::DSPatchables::Mean>();

            // Scharr
            if (p.name == "Scharr")
                return std::make_shared<DSPatch::DSPatchables::ScharrFilter>();

            // Sobel
            if (p.name == "Sobel")
                return std::make_shared<DSPatch::DSPatchables::SobelFilter>();

            // Laplacian
            if (p.name == "Laplacian")
                return std::make_shared<DSPatch::DSPatchables::LaplacianFilter>();

            // Convert Scale Abs
            if (p.name == "Scale_Abs")
                return std::make_shared<DSPatch::DSPatchables::ScaleAbs>();

            // Add Weighted
            if (p.name == "Add_Weighted")
                return std::make_shared<DSPatch::DSPatchables::AddWeighted>();

            // Add
            if (p.name == "Add")
                return std::make_shared<DSPatch::DSPatchables::Add>();

            // Abs Diff
            if (p.name == "Abs_Diff")
                return std::make_shared<DSPatch::DSPatchables::AbsDiff>();

            // Divide
            if (p.name == "Divide")
                return std::make_shared<DSPatch::DSPatchables::Divide>();

            // Max
            if (p.name == "Max")
                return std::make_shared<DSPatch::DSPatchables::Max>();

            // Min
            if (p.name == "Min")
                return std::make_shared<DSPatch::DSPatchables::Min>();

            // Multiply
            if (p.name == "Multiply")
                return std::make_shared<DSPatch::DSPatchables::Multiply>();

            // Subtract
            if (p.name == "Subtract")
                return std::make_shared<DSPatch::DSPatchables::Subtract>();

            // Magnitude
            if (p.name == "Magnitude")
                return std::make_shared<DSPatch::DSPatchables::Magnitude>();

            // Random Noise
            if (p.name == "Rnd_Noise")
                return std::make_shared<DSPatch::DSPatchables::RndNoise>();

            // Histogram Viewer
            if (p.name == "Histogram")
                return std::make_shared<DSPatch::DSPatchables::HistogramViewer>();

            // Color Reduce
            if (p.name == "Color_Reduce")
                return std::make_shared<DSPatch::DSPatchables::ColorReduce>();

            // Depth Viewer 3D
            if (p.name == "Depth_Viewer_3D")
                return std::make_shared<DSPatch::DSPatchables::DepthViewer3D>();

            // Hough Lines
            if (p.name == "Hough_Lines")
                return std::make_shared<DSPatch::DSPatchables::HoughLines>();

            // Solid
            if (p.name == "Solid")
                return std::make_shared<DSPatch::DSPatchables::Solid>();

            // Line Intersect
            if (p.name == "Line_Intersections")
                return std::make_shared<DSPatch::DSPatchables::LineIntersect>();

            // Perspective Warp
            if (p.name == "Perspective_Warp")
                return std::make_shared<DSPatch::DSPatchables::PerspectiveWarp>();

            // Sharpen
            if (p.name == "Sharpen")
                return std::make_shared<DSPatch::DSPatchables::Sharpen>();

            // Split
            if (p.name == "Split")
                return std::make_shared<DSPatch::DSPatchables::Split>();

            // Combine
            if (p.name == "Combine")
                return std::make_shared<DSPatch::DSPatchables::Combine>();

            // DNN - Classification
            if (p.name == "Classification")
                return std::make_shared<DSPatch::DSPatchables::Classification>();

            // DNN - Object Detection
            if (p.name == "Object_Detection")
                return std::make_shared<DSPatch::DSPatchables::ObjectDetection>();

            // DNN - Segmentation
            if (p.name == "Segmentation")
                return std::make_shared<DSPatch::DSPatchables::Segmentation>();

            // DNN - Human Pose
            if (p.name == "Human_Pose")
                return std::make_shared<DSPatch::DSPatchables::HumanPose>();

            // DNN - Text Detection
            if (p.name == "Text_Detection")
                return std::make_shared<DSPatch::DSPatchables::TextDetection>();

            // DNN - Text Recognition
            if (p.name == "Text_Recognition")
                return std::make_shared<DSPatch::DSPatchables::TextRecognition>();

            // DNN - Image Processing
            if (p.name == "Image_Processing")
                return std::make_shared<DSPatch::DSPatchables::ImageProcessing>();

            // ...

        }
    }

    return nullptr;
}
} // End Namespace FlowCV
