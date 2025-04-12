#pragma once

#include "Node.h"
#include <filesystem>

namespace ImageProcessor {

class ImageInputNode : public BaseNode {
public:
    ImageInputNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
    // Load image from file
    bool loadImage(const std::string& filePath);
    
    // Get image metadata
    int getWidth() const;
    int getHeight() const;
    std::string getFormat() const;
    size_t getFileSize() const;
    
private:
    std::string currentFilePath;
    cv::Mat originalImage;
    long fileSize = 0;
    std::string fileFormat;
    
    // Convert file extension to format string
    std::string getFormatFromExtension(const std::string& filePath);
    
    // Get file size in bytes
    long getFileSizeBytes(const std::string& filePath);
};

} // namespace ImageProcessor