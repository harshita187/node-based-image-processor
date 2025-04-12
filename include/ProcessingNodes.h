#pragma once

#include "Node.h"

namespace ImageProcessor {

// Brightness/Contrast Node
class BrightnessContrastNode : public BaseNode {
public:
    BrightnessContrastNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat adjustBrightnessContrast(const cv::Mat& input, float brightness, float contrast);
};

// Color Channel Splitter Node
class ColorChannelSplitterNode : public BaseNode {
public:
    ColorChannelSplitterNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
};

// Blur Node
class BlurNode : public BaseNode {
public:
    BlurNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat applyGaussianBlur(const cv::Mat& input, int radius, bool directional, 
                             double sigmaX, double sigmaY);
    void generateKernel(int radius, double sigmaX, double sigmaY);
    
    cv::Mat kernel;  // Store the kernel for visualization
};

// Threshold Node
class ThresholdNode : public BaseNode {
public:
    ThresholdNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat calculateHistogram(const cv::Mat& input);
    cv::Mat histogramImage;  // Store the histogram for visualization
};

// Edge Detection Node
class EdgeDetectionNode : public BaseNode {
public:
    EdgeDetectionNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
};

// Blend Node
class BlendNode : public BaseNode {
public:
    BlendNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat blendImages(const cv::Mat& input1, const cv::Mat& input2, int blendMode, float opacity);
};

// Noise Generation Node
class NoiseGenerationNode : public BaseNode {
public:
    NoiseGenerationNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat generateNoise(int width, int height, int type, float scale, int octaves, float persistence);
};

// Convolution Filter Node
class ConvolutionFilterNode : public BaseNode {
public:
    ConvolutionFilterNode(const std::string& id, NodeGraph* graph);
    
    // Implement abstract methods
    void process() override;
    void renderProperties() override;
    void renderPreview() override;
    
private:
    cv::Mat applyConvolution(const cv::Mat& input, const cv::Mat& kernel);
    void setPreset(const std::string& presetName);
};

} // namespace ImageProcessor