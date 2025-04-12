#pragma once

#include "Node.h"
#include "InputNodes.h"
#include "ProcessingNodes.h"
#include "OutputNodes.h"

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace ImageProcessor {

class Application {
public:
    Application();
    ~Application();
    
    // Application lifecycle
    bool initialize();
    void shutdown();
    void run();
    
    // Node graph management
    NodeGraph* getNodeGraph() { return nodeGraph.get(); }
    
    // Factory methods for creating nodes
    BaseNode* createNode(const std::string& type);
    
    // UI state
    BaseNode* getSelectedNode() const { return selectedNode; }
    void setSelectedNode(BaseNode* node) { selectedNode = node; }
    
private:
    // Node graph
    std::unique_ptr<NodeGraph> nodeGraph;
    
    // Node factory
    using NodeFactory = std::function<BaseNode*(const std::string&, NodeGraph*)>;
    std::unordered_map<std::string, NodeFactory> nodeFactories;
    
    // Register node types with the factory
    void registerNodeTypes();
    
    // UI state
    BaseNode* selectedNode = nullptr;
    
    // Window management
    void setupStyles();
    void createMainMenuBar();
    void createNodeCanvas();
    void createPropertiesPanel();
    void createPreviewPanel();
    
    // Window state
    bool running = true;
    float canvasZoom = 1.0f;
    float canvasOffsetX = 0.0f;
    float canvasOffsetY = 0.0f;
    
    // Handling connections
    NodeSocket* draggedSocket = nullptr;
    
    // Node ID counter for generating unique IDs
    int nextNodeId = 1;
    std::string generateNodeId(const std::string& prefix);
};

} // namespace ImageProcessor