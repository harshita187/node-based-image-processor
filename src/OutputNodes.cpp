#include "OutputNodes.h"
#include <imgui.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace ImageProcessor {

ImageOutputNode::ImageOutputNode(const std::string& id, NodeGraph* graph)
    : BaseNode(id, "Image Output", NodeType::Output, graph) {
    
    // Add input socket
    addInputSocket("Image", DataType::Image);
    
    // Add parameters
    addParameter(NodeParameter::createFilePath("output_path", "Output Path", ""));
    
    // Format selection
    std::vector<std::string> formats = {"PNG", "JPG", "BMP"};
    addParameter(NodeParameter("format", "Format", formats, 0));
    
    // Quality setting
    addParameter(NodeParameter("quality", "Quality", 1, 100, 95));
    
    // Save button
    addParameter(NodeParameter::createButton("save_button", "Save Image", [this]() {
        auto param = getParameter("output_path");
        if (param) {
            saveImage(param->stringValue);
        }
    }));
}

void ImageOutputNode::process() {
    // Get input socket
    NodeSocket* inputSocket = getInputSocket("in_Image_0");
    if (!inputSocket || !inputSocket->connectedSocket) {
        // No input connected, nothing to process
        processedImage = cv::Mat();
        return;
    }
    
    // Get input image from connected socket
    processedImage = inputSocket->connectedSocket->value;
}

void ImageOutputNode::renderProperties() {
    // This will be implemented with ImGui
    // For now, it's just a placeholder
    
    // In real implementation:
    // 1. Show output path field
    // 2. Show format dropdown
    // 3. Show quality slider
    // 4. Show save button
}

void ImageOutputNode::renderPreview() {
    // This will be implemented with ImGui
    // For now, it's just a placeholder
    
    // In real implementation:
    // Display a downscaled version of the processed image
}

bool ImageOutputNode::saveImage(const std::string& filePath) {
    if (filePath.empty() || processedImage.empty()) {
        return false;
    }
    
    // Set current file path
    currentFilePath = filePath;
    
    // Update format based on file extension if not specified
    std::string extension = fs::path(filePath).extension().string();
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }
    
    // Convert to lowercase
    std::transform(extension.begin(), extension.end(), extension.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    // Check if extension matches a known format
    if (extension == "jpg" || extension == "jpeg" || 
        extension == "png" || extension == "bmp") {
        // Use extension's format
        selectedFormat = extension;
    }
    
    // Get imwrite parameters
    std::vector<int> params = getImwriteParams();
    
    // Try to save the image
    try {
        return cv::imwrite(filePath, processedImage, params);
    } catch (const cv::Exception& e) {
        std::cerr << "Error saving image: " << e.what() << std::endl;
        return false;
    }
}

void ImageOutputNode::setQ