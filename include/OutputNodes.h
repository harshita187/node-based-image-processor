#pragma once

#include "Node.h"

namespace ImageProcessor {

class ImageOutputNode : public BaseNode {
public:
    ImageOutputNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
    // Save image to file
    bool saveImage(const std::string& filePath);
    
    // Set quality settings
    void setQuality(int quality); // 0-100 for JPG, compression level for PNG
    void setFormat(const std::string& format); // "jpg", "png", "bmp"
    
private:
    std::string currentFilePath;
    std::string selectedFormat = "png";
    int quality = 95;
    cv::Mat processedImage;
    
    // Map format to OpenCV imwrite parameters
    std::vector<int> getImwriteParams();
};

} // namespace ImageProcessor