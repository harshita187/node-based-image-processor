#include "InputNodes.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <algorithm>

namespace fs = std::filesystem;

namespace ImageProcessor {

ImageInputNode::ImageInputNode(const std::string& id, NodeGraph* graph)
    : BaseNode(id, "Image Input", NodeType::Input, graph) {
    
    // Add output socket for the image
    addOutputSocket("Image", DataType::Image);
    
    // Add parameters
    addParameter(NodeParameter::createFilePath("file_path", "Image File", ""));
    addParameter(NodeParameter::createButton("load_button", "Load Image", [this]() {
        auto param = getParameter("file_path");
        if (param) {
            loadImage(param->stringValue);
        }
    }));
}

void ImageInputNode::process() {
    // Check if we have an image
    if (originalImage.empty()) {
        return;
    }
    
    // Just set the output to the loaded image
    auto* outputSocket = getOutputSocket("out_Image_0");
    if (outputSocket) {
        outputSocket->value = originalImage.clone();
    }
}

void ImageInputNode::renderProperties() {
    // This will be implemented with ImGui
    // For now, it's just a placeholder
    
    // In real implementation:
    // 1. Show file input field
    // 2. Show load button
    // 3. Display metadata
}

void ImageInputNode::renderPreview() {
    // This will be implemented with ImGui
    // For now, it's just a placeholder
    
    // In real implementation:
    // Display a downscaled version of the image
}

bool ImageInputNode::loadImage(const std::string& filePath) {
    if (filePath.empty()) {
        return false;
    }
    
    // Check if file exists
    if (!fs::exists(filePath)) {
        std::cerr << "File not found: " << filePath << std::endl;
        return false;
    }
    
    // Try to load the image
    cv::Mat loadedImage = cv::imread(filePath, cv::IMREAD_UNCHANGED);
    if (loadedImage.empty()) {
        std::cerr << "Failed to load image: " << filePath << std::endl;
        return false;
    }
    
    // Update properties
    currentFilePath = filePath;
    originalImage = loadedImage;
    fileSize = getFileSizeBytes(filePath);
    fileFormat = getFormatFromExtension(filePath);
    
    // Update parameter
    auto* param = getParameter("file_path");
    if (param) {
        param->stringValue = filePath;
    }
    
    // Mark dirty to trigger reprocessing
    setDirty(true);
    
    return true;
}

int ImageInputNode::getWidth() const {
    return originalImage.empty() ? 0 : originalImage.cols;
}

int ImageInputNode::getHeight() const {
    return originalImage.empty() ? 0 : originalImage.rows;
}

std::string ImageInputNode::getFormat() const {
    return fileFormat;
}

size_t ImageInputNode::getFileSize() const {
    return fileSize;
}

std::string ImageInputNode::getFormatFromExtension(const std::string& filePath) {
    std::string extension = fs::path(filePath).extension().string();
    
    // Convert to lowercase
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    // Remove the dot if present
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }
    
    // Map extension to format name
    if (extension == "jpg" || extension == "jpeg") {
        return "JPEG";
    } else if (extension == "png") {
        return "PNG";
    } else if (extension == "bmp") {
        return "BMP";
    } else if (extension == "tif" || extension == "tiff") {
        return "TIFF";
    } else {
        return extension;
    }
}

long ImageInputNode::getFileSizeBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) {
        return 0;
    }
    return file.tellg();
}

} // namespace ImageProcessor