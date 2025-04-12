#include "Application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace ImageProcessor {

// Window size
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// GLFW error callback
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

Application::Application() : nodeGraph(std::make_unique<NodeGraph>()) {
    registerNodeTypes();
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // Set up error callback
    glfwSetErrorCallback(glfw_error_callback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        return false;
    }
    
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        "Node-Based Image Processor", 
        nullptr, nullptr
    );
    
    if (window == nullptr) {
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // Setup style
    setupStyles();
    
    return true;
}

void Application::shutdown() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
}

void Application::run() {
    // Main window pointer
    GLFWwindow* window = glfwGetCurrentContext();
    
    // Main loop
    while (!glfwWindowShouldClose(window) && running) {
        // Poll and handle events
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Create dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGuiWindowFlags windowFlags = 
            ImGuiWindowFlags_MenuBar | 
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar(2);
        
        ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        
        // Menu bar
        createMainMenuBar();
        
        // Create panels
        createNodeCanvas();
        createPropertiesPanel();
        createPreviewPanel();
        
        ImGui::End(); // End dockspace
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
}

void Application::setupStyles() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Set node editor style
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(4, 3);
    style.CellPadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 21;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 10;
    
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
    
    // Setup colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
    colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}

void Application::createMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                // Clear the node graph
                nodeGraph = std::make_unique<NodeGraph>();
            }
            
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // Open a file dialog to load a graph
                // Placeholder for file dialog
            }
            
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // Save the current graph
                // Placeholder for file dialog
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                running = false;
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Image Input")) {
                auto node = createNode("ImageInput");
                node->setPosition(200, 200);
            }
            
            if (ImGui::MenuItem("Brightness/Contrast")) {
                auto node = createNode("BrightnessContrast");
                node->setPosition(400, 200);
            }
            
            if (ImGui::MenuItem("Color Channel Splitter")) {
                auto node = createNode("ColorChannelSplitter");
                node->setPosition(400, 300);
            }
            
            if (ImGui::MenuItem("Blur")) {
                auto node = createNode("Blur");
                node->setPosition(400, 400);
            }
            
            if (ImGui::MenuItem("Threshold")) {
                auto node = createNode("Threshold");
                node->setPosition(400, 500);
            }
            
            if (ImGui::MenuItem("Edge Detection")) {
                auto node = createNode("EdgeDetection");
                node->setPosition(600, 200);
            }
            
            if (ImGui::MenuItem("Blend")) {
                auto node = createNode("Blend");
                node->setPosition(600, 300);
            }
            
            if (ImGui::MenuItem("Noise Generation")) {
                auto node = createNode("NoiseGeneration");
                node->setPosition(600, 400);
            }
            
            if (ImGui::MenuItem("Convolution Filter")) {
                auto node = createNode("ConvolutionFilter");
                node->setPosition(600, 500);
            }
            
            if (ImGui::MenuItem("Output")) {
                auto node = createNode("ImageOutput");
                node->setPosition(800, 200);
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void Application::createNodeCanvas() {
    ImGui::Begin("Node Canvas", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    // Canvas state
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    // Draw grid
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float GRID_STEP = 64.0f * canvasZoom;
    
    // Calculate grid offset based on canvas pan
    float offsetX = canvasOffsetX - std::floor(canvasOffsetX / GRID_STEP) * GRID_STEP;
    float offsetY = canvasOffsetY - std::floor(canvasOffsetY / GRID_STEP) * GRID_STEP;
    
    // Minor grid lines
    for (float x = offsetX; x < canvasSize.x; x += GRID_STEP) {
        drawList->AddLine(
            ImVec2(canvasPos.x + x, canvasPos.y), 
            ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
            IM_COL32(50, 50, 50, 40)
        );
    }
    
    for (float y = offsetY; y < canvasSize.y; y += GRID_STEP) {
        drawList->AddLine(
            ImVec2(canvasPos.x, canvasPos.y + y), 
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
            IM_COL32(50, 50, 50, 40)
        );
    }
    
    // Draw nodes
    for (BaseNode* node : nodeGraph->getAllNodes()) {
        // Calculate node position
        float x = canvasPos.x + node->getPositionX() * canvasZoom + canvasOffsetX;
        float y = canvasPos.y + node->getPositionY() * canvasZoom + canvasOffsetY;
        
        // Node style based on type
        ImU32 nodeColor;
        switch (node->getType()) {
            case BaseNode::NodeType::Input:
                nodeColor = IM_COL32(70, 150, 70, 255);
                break;
            case BaseNode::NodeType::Processing:
                nodeColor = IM_COL32(70, 70, 150, 255);
                break;
            case BaseNode::NodeType::Output:
                nodeColor = IM_COL32(150, 70, 70, 255);
                break;
            default:
                nodeColor = IM_COL32(150, 150, 150, 255);
        }
        
        // Draw node background
        ImVec2 nodeSize(150, 100);
        drawList->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + nodeSize.x, y + nodeSize.y),
            nodeColor,
            4.0f
        );
        
        // Draw node border (thicker if selected)
        float borderThickness = (node == selectedNode) ? 2.0f : 1.0f;
        drawList->AddRect(
            ImVec2(x, y),
            ImVec2(x + nodeSize.x, y + nodeSize.y),
            IM_COL32(200, 200, 200, 255),
            4.0f,
            0,
            borderThickness
        );
        
        // Draw node title
        drawList->AddText(
            ImVec2(x + 5, y + 5),
            IM_COL32(255, 255, 255, 255),
            node->getName().c_str()
        );
        
        // Draw node sockets
        float socketRadius = 5.0f;
        float socketY = y + 30.0f;
        
        // Input sockets on the left
        for (const auto& socket : node->getInputSockets()) {
            ImVec2 socketPos(x, socketY);
            drawList->AddCircleFilled(
                socketPos,
                socketRadius,
                socket.isConnected() ? IM_COL32(100, 255, 100, 255) : IM_COL32(200, 200, 200, 255)
            );
            
            drawList->AddText(
                ImVec2(socketPos.x + socketRadius * 2, socketPos.y - socketRadius),
                IM_COL32(255, 255, 255, 255),
                socket.name.c_str()
            );
            
            socketY += 20.0f;
        }
        
        // Output sockets on the right
        socketY = y + 30.0f;
        for (const auto& socket : node->getOutputSockets()) {
            ImVec2 socketPos(x + nodeSize.x, socketY);
            drawList->AddCircleFilled(
                socketPos,
                socketRadius,
                socket.isConnected() ? IM_COL32(100, 255, 100, 255) : IM_COL32(200, 200, 200, 255)
            );
            
            // Measure text to right-align
            ImVec2 textSize = ImGui::CalcTextSize(socket.name.c_str());
            drawList->AddText(
                ImVec2(socketPos.x - textSize.x - socketRadius * 2, socketPos.y - socketRadius),
                IM_COL32(255, 255, 255, 255),
                socket.name.c_str()
            );
            
            socketY += 20.0f;
        }
        
        // Check for node interaction (click to select)
        if (ImGui::IsMouseClicked(0) && 
            ImGui::IsMouseHoveringRect(ImVec2(x, y), ImVec2(x + nodeSize.x, y + nodeSize.y))) {
            setSelectedNode(node);
        }
    }
    
    // Draw connections
    for (const auto& connection : nodeGraph->getConnections()) {
        NodeSocket* outputSocket = connection.getOutputSocket();
        NodeSocket* inputSocket = connection.getInputSocket();
        
        if (!outputSocket || !inputSocket) continue;
        
        // Get nodes
        BaseNode* outputNode = outputSocket->parent;
        BaseNode* inputNode = inputSocket->parent;
        
        // Get socket positions
        float outX = canvasPos.x + outputNode->getPositionX() * canvasZoom + canvasOffsetX + 150; // Right side
        float outY = canvasPos.y + outputNode->getPositionY() * canvasZoom + canvasOffsetY + 30.0f + 
                    std::distance(outputNode->getOutputSockets().data(), outputSocket) * 20.0f;
        
        float inX = canvasPos.x + inputNode->getPositionX() * canvasZoom + canvasOffsetX; // Left side
        float inY = canvasPos.y + inputNode->getPositionY() * canvasZoom + canvasOffsetY + 30.0f + 
                   std::distance(inputNode->getInputSockets().data(), inputSocket) * 20.0f;
        
        // Draw bezier curve
        ImVec2 p1(outX, outY);
        ImVec2 p2(inX, inY);
        ImVec2 cp1(p1.x + 50, p1.y);
        ImVec2 cp2(p2.x - 50, p2.y);
        
        drawList->AddBezierCubic(
            p1, cp1, cp2, p2,
            IM_COL32(200, 200, 100, 255),
            2.0f
        );
    }
    
    // Canvas controls
    // Middle-click drag to pan
    if (ImGui::IsMouseDragging(2) && ImGui::IsWindowHovered()) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        canvasOffsetX += delta.x;
        canvasOffsetY += delta.y;
    }
    
    // Mouse wheel to zoom
    if (ImGui::IsWindowHovered()) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0) {
            const float zoomFactor = 0.1f;
            canvasZoom = std::max(0.1f, canvasZoom + wheel * zoomFactor);
        }
    }
    
    ImGui::End();
}

void Application::createPropertiesPanel() {
    ImGui::Begin("Properties");
    
    if (selectedNode) {
        ImGui::Text("Node: %s", selectedNode->getName().c_str());
        ImGui::Separator();
        
        // Call the node's properties render function
        selectedNode->renderProperties();
    }
    else {
        ImGui::Text("No node selected");
    }
    
    ImGui::End();
}

void Application::createPreviewPanel() {
    ImGui::Begin("Preview");
    
    if (selectedNode) {
        // Call the node's preview render function
        selectedNode->renderPreview();
    }
    else {
        ImGui::Text("No node selected");
    }
    
    ImGui::End();
}

void Application::registerNodeTypes() {
    // Register node factories
    nodeFactories["ImageInput"] = [this](const std::string& id, NodeGraph* graph) {
        return new ImageInputNode(id, graph);
    };
    
    nodeFactories["BrightnessContrast"] = [this](const std::string& id, NodeGraph* graph) {
        return new BrightnessContrastNode(id, graph);
    };
    
    nodeFactories["ColorChannelSplitter"] = [this](const std::string& id, NodeGraph* graph) {
        return new ColorChannelSplitterNode(id, graph);
    };
    
    nodeFactories["Blur"] = [this](const std::string& id, NodeGraph* graph) {
        return new BlurNode(id, graph);
    };
    
    nodeFactories["Threshold"] = [this](const std::string& id, NodeGraph* graph) {
        return new ThresholdNode(id, graph);
    };
    
    nodeFactories["ImageOutput"] = [this](const std::string& id, NodeGraph* graph) {
        return new ImageOutputNode(id, graph);
    };
}

BaseNode* Application::createNode(const std::string& type) {
    auto it = nodeFactories.find(type);
    if (it == nodeFactories.end()) {
        return nullptr;
    }
    
    // Generate a unique ID for the node
    std::string id = generateNodeId(type);
    
    // Create the node
    BaseNode* node = it->second(id, nodeGraph.get());
    
    // Add to graph
    nodeGraph->addNode(std::unique_ptr<BaseNode>(node));
    
    return node;
}

std::string Application::generateNodeId(const std::string& prefix) {
    return prefix + "_" + std::to_string(nextNodeId++);
}

} // namespace ImageProcessor