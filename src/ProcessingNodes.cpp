// Continue ColorChannelSplitterNode implementation from where it was cut off
NodeSocket* blueSocket = getOutputSocket("out_Blue_2");
if (blueSocket) {
    if (outputGrayscale) {
        blueSocket->value = channels[0]; // Blue is already grayscale
    } else {
        // Create a 3-channel image with only blue channel
        cv::Mat blueOnly = cv::Mat::zeros(inputImage.size(), CV_8UC3);
        cv::mixChannels(&channels[0], 1, &blueOnly, 1, {0,0}, 1); // Copy blue to blue
        blueSocket->value = blueOnly;
    }
}

// Green channel
NodeSocket* greenSocket = getOutputSocket("out_Green_1");
if (greenSocket) {
    if (outputGrayscale) {
        greenSocket->value = channels[1]; // Green is already grayscale
    } else {
        // Create a 3-channel image with only green channel
        cv::Mat greenOnly = cv::Mat::zeros(inputImage.size(), CV_8UC3);
        cv::mixChannels(&channels[1], 1, &greenOnly, 1, {0,1}, 1); // Copy green to green
        greenSocket->value = greenOnly;
    }
}

// Red channel
NodeSocket* redSocket = getOutputSocket("out_Red_0");
if (redSocket) {
    if (outputGrayscale) {
        redSocket->value = channels[2]; // Red is already grayscale
    } else {
        // Create a 3-channel image with only red channel
        cv::Mat redOnly = cv::Mat::zeros(inputImage.size(), CV_8UC3);
        cv::mixChannels(&channels[2], 1, &redOnly, 1, {0,2}, 1); // Copy red to red
        redSocket->value = redOnly;
    }
}

// Alpha channel (if present)
if (channels.size() >= 4) {
    NodeSocket* alphaSocket = getOutputSocket("out_Alpha_3");
    if (alphaSocket) {
        alphaSocket->value = channels[3]; // Alpha is always grayscale
    }
}
}
}

void ColorChannelSplitterNode::renderProperties() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Show checkbox for grayscale option
}

void ColorChannelSplitterNode::renderPreview() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// Display thumbnails of each channel
}

//------------------------------------------------------------------------------
// BlurNode Implementation
//------------------------------------------------------------------------------

BlurNode::BlurNode(const std::string& id, NodeGraph* graph)
: BaseNode(id, "Blur", NodeType::Processing, graph) {

// Add input socket
addInputSocket("Image", DataType::Image);

// Add output socket
addOutputSocket("Image", DataType::Image);

// Add parameters
addParameter(NodeParameter("radius", "Blur Radius", 1.0f, 20.0f, 5.0f));
addParameter(NodeParameter("directional", "Directional Blur", false));
addParameter(NodeParameter("sigmaX", "Sigma X", 0.0f, 20.0f, 0.0f));
addParameter(NodeParameter("sigmaY", "Sigma Y", 0.0f, 20.0f, 0.0f));
}

void BlurNode::process() {
// Get input socket
NodeSocket* inputSocket = getInputSocket("in_Image_0");
if (!inputSocket || !inputSocket->connectedSocket) {
// No input connected
return;
}

// Get input image
cv::Mat inputImage = inputSocket->connectedSocket->value;
if (inputImage.empty()) {
return;
}

// Get parameters
int radius = static_cast<int>(getParameter("radius")->value);
bool directional = getParameter("directional")->boolValue;
double sigmaX = getParameter("sigmaX")->value;
double sigmaY = directional ? getParameter("sigmaY")->value : sigmaX;

// Process image
cv::Mat result = applyGaussianBlur(inputImage, radius, directional, sigmaX, sigmaY);

// Generate kernel for visualization
generateKernel(radius, sigmaX, sigmaY);

// Set output
NodeSocket* outputSocket = getOutputSocket("out_Image_0");
if (outputSocket) {
outputSocket->value = result;
}
}

void BlurNode::renderProperties() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Show radius slider
// 2. Show directional checkbox
// 3. Show sigma X and Y sliders (Y only if directional is checked)
}

void BlurNode::renderPreview() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Display the processed image
// 2. Display the kernel visualization
}

cv::Mat BlurNode::applyGaussianBlur(const cv::Mat& input, int radius, bool directional, 
                           double sigmaX, double sigmaY) {
cv::Mat result;

// Convert radius to ksize (must be odd)
int ksize = 2 * radius + 1;

// Apply Gaussian blur
cv::GaussianBlur(input, result, cv::Size(ksize, ksize), sigmaX, sigmaY);

return result;
}

void BlurNode::generateKernel(int radius, double sigmaX, double sigmaY) {
// Generate a visualization of the kernel
int ksize = 2 * radius + 1;

// Create the kernel
kernel = cv::Mat::zeros(ksize, ksize, CV_32F);

// Fill the kernel with Gaussian values
double sum = 0.0;

for (int y = 0; y < ksize; y++) {
for (int x = 0; x < ksize; x++) {
    double normX = (x - radius) / sigmaX;
    double normY = (y - radius) / sigmaY;
    
    double val = exp(-(normX * normX + normY * normY) / 2.0);
    kernel.at<float>(y, x) = static_cast<float>(val);
    sum += val;
}
}

// Normalize
kernel = kernel / sum;
}

//------------------------------------------------------------------------------
// ThresholdNode Implementation
//------------------------------------------------------------------------------

ThresholdNode::ThresholdNode(const std::string& id, NodeGraph* graph)
: BaseNode(id, "Threshold", NodeType::Processing, graph) {

// Add input socket
addInputSocket("Image", DataType::Image);

// Add output socket
addOutputSocket("Image", DataType::Image);

// Add parameters
addParameter(NodeParameter("threshold", "Threshold Value", 0.0f, 255.0f, 128.0f));

// Add threshold method selection
std::vector<std::string> methods = {"Binary", "Binary Inverted", "Truncate", "To Zero", "To Zero Inverted", "Otsu", "Adaptive Mean", "Adaptive Gaussian"};
addParameter(NodeParameter("method", "Threshold Method", methods, 0));

// Add adaptive block size parameter
addParameter(NodeParameter("blockSize", "Block Size", 3.0f, 99.0f, 11.0f));
}

void ThresholdNode::process() {
// Get input socket
NodeSocket* inputSocket = getInputSocket("in_Image_0");
if (!inputSocket || !inputSocket->connectedSocket) {
// No input connected
return;
}

// Get input image
cv::Mat inputImage = inputSocket->connectedSocket->value;
if (inputImage.empty()) {
return;
}

// Convert to grayscale if needed
cv::Mat grayImage;
if (inputImage.channels() == 1) {
grayImage = inputImage;
} else {
cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
}

// Get parameters
double thresh = getParameter("threshold")->value;
int method = getParameter("method")->intValue;
int blockSize = static_cast<int>(getParameter("blockSize")->value);

// Make block size odd
if (blockSize % 2 == 0) {
blockSize++;
}

// Calculate histogram for visualization
histogramImage = calculateHistogram(grayImage);

// Apply threshold
cv::Mat result;

switch (method) {
case 0: // Binary
    cv::threshold(grayImage, result, thresh, 255, cv::THRESH_BINARY);
    break;
case 1: // Binary Inverted
    cv::threshold(grayImage, result, thresh, 255, cv::THRESH_BINARY_INV);
    break;
case 2: // Truncate
    cv::threshold(grayImage, result, thresh, 255, cv::THRESH_TRUNC);
    break;
case 3: // To Zero
    cv::threshold(grayImage, result, thresh, 255, cv::THRESH_TOZERO);
    break;
case 4: // To Zero Inverted
    cv::threshold(grayImage, result, thresh, 255, cv::THRESH_TOZERO_INV);
    break;
case 5: // Otsu
    cv::threshold(grayImage, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    break;
case 6: // Adaptive Mean
    cv::adaptiveThreshold(grayImage, result, 255, cv::ADAPTIVE_THRESH_MEAN_C, 
                        cv::THRESH_BINARY, blockSize, 5);
    break;
case 7: // Adaptive Gaussian
    cv::adaptiveThreshold(grayImage, result, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                        cv::THRESH_BINARY, blockSize, 5);
    break;
}

// Convert back to color if input was color
cv::Mat colorResult;
if (inputImage.channels() > 1) {
cv::cvtColor(result, colorResult, cv::COLOR_GRAY2BGR);
if (inputImage.channels() == 4) {
    // Add alpha channel
    cv::Mat alpha;
    cv::extractChannel(inputImage, alpha, 3);
    cv::Mat channels[4] = {colorResult.channels()[0], colorResult.channels()[1], 
                          colorResult.channels()[2], alpha};
    cv::merge(channels, 4, colorResult);
}
result = colorResult;
}

// Set output
NodeSocket* outputSocket = getOutputSocket("out_Image_0");
if (outputSocket) {
outputSocket->value = result;
}
}

void ThresholdNode::renderProperties() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Show threshold slider
// 2. Show method dropdown
// 3. Show block size slider for adaptive methods
}

void ThresholdNode::renderPreview() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Display the processed image
// 2. Display the histogram
}

cv::Mat ThresholdNode::calculateHistogram(const cv::Mat& input) {
// Calculate histogram
int histSize = 256;
float range[] = {0, 256};
const float* histRange = {range};
cv::Mat hist;
cv::calcHist(&input, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

// Normalize histogram
cv::normalize(hist, hist, 0, 100, cv::NORM_MINMAX);

// Create histogram image
cv::Mat histImage(100, 256, CV_8UC3, cv::Scalar(0, 0, 0));

// Draw histogram
for (int i = 0; i < histSize; i++) {
cv::line(
    histImage, 
    cv::Point(i, 100), 
    cv::Point(i, 100 - cvRound(hist.at<float>(i))), 
    cv::Scalar(255, 255, 255)
);
}

// Draw threshold line
int thresh = static_cast<int>(getParameter("threshold")->value);
cv::line(
histImage,
cv::Point(thresh, 0),
cv::Point(thresh, 100),
cv::Scalar(0, 0, 255),
1
);

return histImage;
}

//------------------------------------------------------------------------------
// EdgeDetectionNode Implementation
//------------------------------------------------------------------------------

EdgeDetectionNode::EdgeDetectionNode(const std::string& id, NodeGraph* graph)
: BaseNode(id, "Edge Detection", NodeType::Processing, graph) {

// Add input socket
addInputSocket("Image", DataType::Image);

// Add output socket
addOutputSocket("Image", DataType::Image);

// Add parameters
std::vector<std::string> algorithms = {"Sobel", "Canny"};
addParameter(NodeParameter("algorithm", "Algorithm", algorithms, 0));

// Sobel parameters
addParameter(NodeParameter("sobelKSize", "Kernel Size", 1.0f, 7.0f, 3.0f));
addParameter(NodeParameter("sobelScale", "Scale", 0.1f, 10.0f, 1.0f));
addParameter(NodeParameter("sobelDelta", "Delta", 0.0f, 255.0f, 0.0f));

// Canny parameters
addParameter(NodeParameter("cannyThreshold1", "Threshold 1", 0.0f, 255.0f, 100.0f));
addParameter(NodeParameter("cannyThreshold2", "Threshold 2", 0.0f, 255.0f, 200.0f));

// Overlay option
addParameter(NodeParameter("overlay", "Overlay on Original", false));
}

void EdgeDetectionNode::process() {
// Get input socket
NodeSocket* inputSocket = getInputSocket("in_Image_0");
if (!inputSocket || !inputSocket->connectedSocket) {
// No input connected
return;
}

// Get input image
cv::Mat inputImage = inputSocket->connectedSocket->value;
if (inputImage.empty()) {
return;
}

// Convert to grayscale if needed
cv::Mat grayImage;
if (inputImage.channels() == 1) {
grayImage = inputImage;
} else {
cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
}

// Get parameters
int algorithm = getParameter("algorithm")->intValue;
bool overlay = getParameter("overlay")->boolValue;

// Apply edge detection
cv::Mat edges;

if (algorithm == 0) {  // Sobel
int ksize = static_cast<int>(getParameter("sobelKSize")->value);
if (ksize % 2 == 0) ksize--;  // Must be odd
if (ksize < 1) ksize = 1;

double scale = getParameter("sobelScale")->value;
double delta = getParameter("sobelDelta")->value;

// Compute Sobel derivatives
cv::Mat gradX, gradY, absGradX, absGradY;

cv::Sobel(grayImage, gradX, CV_16S, 1, 0, ksize, scale, delta);
cv::Sobel(grayImage, gradY, CV_16S, 0, 1, ksize, scale, delta);

cv::convertScaleAbs(gradX, absGradX);
cv::convertScaleAbs(gradY, absGradY);

// Combine gradients
cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, edges);
}
else {  // Canny
double threshold1 = getParameter("cannyThreshold1")->value;
double threshold2 = getParameter("cannyThreshold2")->value;

cv::Canny(grayImage, edges, threshold1, threshold2);
}

// Create output
cv::Mat result;

if (overlay) {
// Overlay edges on original image
if (inputImage.channels() == 1) {
    // For grayscale, use color edges
    cv::Mat colorEdges;
    cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);
    
    // Create mask from edges
    cv::Mat mask = edges > 0;
    
    // Create overlay
    result = inputImage.clone();
    colorEdges.copyTo(result, mask);
}
else {
    // For color images, use white edges
    result = inputImage.clone();
    
    // Apply edges as white overlay
    for (int y = 0; y < result.rows; y++) {
        for (int x = 0; x < result.cols; x++) {
            if (edges.at<uchar>(y, x) > 0) {
                if (result.channels() == 3) {
                    result.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
                }
                else if (result.channels() == 4) {
                    cv::Vec4b& pixel = result.at<cv::Vec4b>(y, x);
                    pixel[0] = 255;  // B
                    pixel[1] = 255;  // G
                    pixel[2] = 255;  // R
                    // Keep alpha
                }
            }
        }
    }
}
}
else {
// Just return edges
if (inputImage.channels() == 1) {
    result = edges;
}
else if (inputImage.channels() == 3) {
    cv::cvtColor(edges, result, cv::COLOR_GRAY2BGR);
}
else {  // 4 channels
    cv::Mat rgbEdges;
    cv::cvtColor(edges, rgbEdges, cv::COLOR_GRAY2BGR);
    
    // Add alpha channel from original
    cv::Mat channels[4];
    cv::split(inputImage, channels);
    
    // Combine RGB edges with original alpha
    cv::Mat rgbaEdges;
    cv::Mat rgbChannels[3];
    cv::split(rgbEdges, rgbChannels);
    
    cv::Mat outChannels[4] = {rgbChannels[0], rgbChannels[1], rgbChannels[2], channels[3]};
    cv::merge(outChannels, 4, result);
}
}

// Set output
NodeSocket* outputSocket = getOutputSocket("out_Image_0");
if (outputSocket) {
outputSocket->value = result;
}
}

void EdgeDetectionNode::renderProperties() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Show algorithm dropdown
// 2. Show parameters based on selected algorithm
// 3. Show overlay checkbox
}

void EdgeDetectionNode::renderPreview() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// Display the processed image
}

//------------------------------------------------------------------------------
// BlendNode Implementation
//------------------------------------------------------------------------------

BlendNode::BlendNode(const std::string& id, NodeGraph* graph)
: BaseNode(id, "Blend", NodeType::Processing, graph) {

// Add input sockets
addInputSocket("Image 1", DataType::Image);
addInputSocket("Image 2", DataType::Image);

// Add output socket
addOutputSocket("Image", DataType::Image);

// Add parameters
std::vector<std::string> blendModes = {"Normal", "Multiply", "Screen", "Overlay", "Darken"};
addParameter(NodeParameter("blendMode", "Blend Mode", blendModes, 0));

// Add opacity parameter
addParameter(NodeParameter("opacity", "Opacity", 0.0f, 1.0f, 1.0f));
}

void BlendNode::process() {
// Get input sockets
NodeSocket* input1Socket = getInputSocket("in_Image 1_0");
NodeSocket* input2Socket = getInputSocket("in_Image 2_1");

if (!input1Socket || !input1Socket->connectedSocket ||
!input2Socket || !input2Socket->connectedSocket) {
// Both inputs required
return;
}

// Get input images
cv::Mat image1 = input1Socket->connectedSocket->value;
cv::Mat image2 = input2Socket->connectedSocket->value;

if (image1.empty() || image2.empty()) {
return;
}

// Get parameters
int blendMode = getParameter("blendMode")->intValue;
float opacity = getParameter("opacity")->value;

// Process images
cv::Mat result = blendImages(image1, image2, blendMode, opacity);

// Set output
NodeSocket* outputSocket = getOutputSocket("out_Image_0");
if (outputSocket) {
outputSocket->value = result;
}
}

void BlendNode::renderProperties() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// 1. Show blend mode dropdown
// 2. Show opacity slider
}

void BlendNode::renderPreview() {
// This will be implemented with ImGui
// For now, it's just a placeholder

// In real implementation:
// Display the processed image
}

cv::Mat BlendNode::blendImages(const cv::Mat& image1, const cv::Mat& image2, int blendMode, float opacity) {
// Resize image2 to match image1 if sizes differ
cv::Mat resizedImage2;
if (image1.size() != image2.size()) {
cv::resize(image2, resizedImage2, image1.size());
} else {
resizedImage2 = image2;
}

// Convert to same type if needed
cv::Mat img1, img2;
if (image1.type() != resizedImage2.type()) {
if (image1.channels() > resizedImage2.channels()) {
    cv::cvtColor(resizedImage2, img2, image1.channels() == 3 ? cv::COLOR_GRAY2BGR : cv::COLOR_GRAY2BGRA);
    img1 = image1;
} else {
    cv::cvtColor(image1, img1, resizedImage2.channels() == 3 ? cv::COLOR_GRAY2BGR : cv::COLOR_GRAY2BGRA);
    img2 = resizedImage2;
}
} else {
img1 = image1;
img2 = resizedImage2;
}

// Create result matrix
cv::Mat result = img1.clone();

// Apply blend mode
switch (blendMode) {
case 0:  // Normal
    // Just copy img2 over img1 with opacity
    cv::addWeighted(img1, 1.0 - opacity, img2, opacity, 0.0, result);
    break;
    
case 1:  // Multiply
    for (int y = 0; y < result.rows; y++) {
        for (int x = 0; x < result.cols; x++) {
            if (result.channels() == 3) {
                cv::Vec3b pixel1 = img1.at<cv::Vec3b>(y, x);
                cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
                cv::Vec3b& outPixel = result.at<cv::Vec3b>(y, x);
                
                for (int c = 0; c < 3; c++) {
                    // Multiply: A * B / 255
                    float blended = (pixel1[c] * pixel2[c]) / 255.0f;
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
            }
            else if (result.channels() == 4) {
                cv::Vec4b pixel1 = img1.at<cv::Vec4b>(y, x);
                cv::Vec4b pixel2 = img2.at<cv::Vec4b>(y, x);
                cv::Vec4b& outPixel = result.at<cv::Vec4b>(y, x);
                
                for (int c = 0; c < 3; c++) {  // Don't multiply alpha
                    // Multiply: A * B / 255
                    float blended = (pixel1[c] * pixel2[c]) / 255.0f;
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
                
                // Blend alpha
                outPixel[3] = cv::saturate_cast<uchar>(pixel1[3] * (1.0f - opacity) + pixel2[3] * opacity);
            }
        }
    }
    break;
    
case 2:  // Screen
    for (int y = 0; y < result.rows; y++) {
        for (int x = 0; x < result.cols; x++) {
            if (result.channels() == 3) {
                cv::Vec3b pixel1 = img1.at<cv::Vec3b>(y, x);
                cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
                cv::Vec3b& outPixel = result.at<cv::Vec3b>(y, x);
                
                for (int c = 0; c < 3; c++) {
                    // Screen: 255 - (255 - A) * (255 - B) / 255
                    float blended = 255.0f - ((255.0f - pixel1[c]) * (255.0f - pixel2[c])) / 255.0f;
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
            }
            else if (result.channels() == 4) {
                cv::Vec4b pixel1 = img1.at<cv::Vec4b>(y, x);
                cv::Vec4b pixel2 = img2.at<cv::Vec4b>(y, x);
                cv::Vec4b& outPixel = result.at<cv::Vec4b>(y, x);
                
                for (int c = 0; c < 3; c++) {  // Don't screen alpha
                    // Screen: 255 - (255 - A) * (255 - B) / 255
                    float blended = 255.0f - ((255.0f - pixel1[c]) * (255.0f - pixel2[c])) / 255.0f;
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
                
                // Blend alpha
                outPixel[3] = cv::saturate_cast<uchar>(pixel1[3] * (1.0f - opacity) + pixel2[3] * opacity);
            }
        }
    }
    break;
    
case 3:  // Overlay
    for (int y = 0; y < result.rows; y++) {
        for (int x = 0; x < result.cols; x++) {
            if (result.channels() == 3) {
                cv::Vec3b pixel1 = img1.at<cv::Vec3b>(y, x);
                cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
                cv::Vec3b& outPixel = result.at<cv::Vec3b>(y, x);
                
                for (int c = 0; c < 3; c++) {
                    // Overlay: if A < 128 then 2*A*B/255 else 255 - 2*(255-A)*(255-B)/255
                    float blended;
                    if (pixel1[c] < 128) {
                        blended = 2.0f * pixel1[c] * pixel2[c] / 255.0f;
                    } else {
                        blended = 255.0f - 2.0f * (255.0f - pixel1[c]) * (255.0f - pixel2[c]) / 255.0f;
                    }
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
            }
            else if (result.channels() == 4) {
                cv::Vec4b pixel1 = img1.at<cv::Vec4b>(y, x);
                cv::Vec4b pixel2 = img2.at<cv::Vec4b>(y, x);
                cv::Vec4b& outPixel = result.at<cv::Vec4b>(y, x);
                
                for (int c = 0; c < 3; c++) {  // Don't overlay alpha
                    // Overlay: if A < 128 then 2*A*B/255 else 255 - 2*(255-A)*(255-B)/255
                    float blended;
                    if (pixel1[c] < 128) {
                        blended = 2.0f * pixel1[c] * pixel2[c] / 255.0f;
                    } else {
                        blended = 255.0f - 2.0f * (255.0f - pixel1[c]) * (255.0f - pixel2[c]) / 255.0f;
                    }
                    float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                    outPixel[c] = cv::saturate_cast<uchar>(final);
                }
                
                // Blend alpha
                outPixel[3] = cv::saturate_cast<uchar>(pixel1[3] * (1.0f - opacity) + pixel2[3] * opacity);
            }
        }
    }
}
break;

case 4:  // Darken
for (int y = 0; y < result.rows; y++) {
    for (int x = 0; x < result.cols; x++) {
        if (result.channels() == 3) {
            cv::Vec3b pixel1 = img1.at<cv::Vec3b>(y, x);
            cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
            cv::Vec3b& outPixel = result.at<cv::Vec3b>(y, x);
            
            for (int c = 0; c < 3; c++) {
                // Darken: min(A, B)
                float blended = std::min(pixel1[c], pixel2[c]);
                float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                outPixel[c] = cv::saturate_cast<uchar>(final);
            }
        }
        else if (result.channels() == 4) {
            cv::Vec4b pixel1 = img1.at<cv::Vec4b>(y, x);
            cv::Vec4b pixel2 = img2.at<cv::Vec4b>(y, x);
            cv::Vec4b& outPixel = result.at<cv::Vec4b>(y, x);
            
            for (int c = 0; c < 3; c++) {  // Don't darken alpha
                // Darken: min(A, B)
                float blended = std::min(pixel1[c], pixel2[c]);
                float final = pixel1[c] * (1.0f - opacity) + blended * opacity;
                outPixel[c] = cv::saturate_cast<uchar>(final);
            }
            
            // Blend alpha
            outPixel[3] = cv::saturate_cast<uchar>(pixel1[3] *