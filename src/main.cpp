#include "Application.h"
#include <iostream>

int main(int argc, char** argv) {
    // Create application
    ImageProcessor::Application app;
    
    // Initialize
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    // Run the application
    app.run();
    
    return 0;
}