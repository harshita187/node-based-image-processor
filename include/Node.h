#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <opencv2/opencv.hpp>

namespace ImageProcessor {

// Forward declarations
class BaseNode;
class NodeGraph;

// Data types that can be passed between nodes
enum class DataType {
    Image,          // OpenCV Mat
    Integer,        // Integer value
    Float,          // Float value
    Color,          // RGBA color
    Bool,           // Boolean value
    String,         // String value
    Vector2,        // 2D vector
    Vector3,        // 3D vector
    Array           // Array of values
};

// Socket for node connections
struct NodeSocket {
    enum class Type { Input, Output };
    
    std::string id;
    std::string name;
    Type type;
    DataType dataType;
    BaseNode* parent;
    NodeSocket* connectedSocket = nullptr;
    cv::Mat value;  // For Image type
    
    // Various value types
    int intValue = 0;
    float floatValue = 0.0f;
    bool boolValue = false;
    std::string stringValue;
    
    // Constructor for creating a socket
    NodeSocket(const std::string& id, const std::string& name, Type type, DataType dataType, BaseNode* parent)
        : id(id), name(name), type(type), dataType(dataType), parent(parent) {}
    
    // Connect this socket to another
    bool connectTo(NodeSocket* other);
    
    // Disconnect this socket
    void disconnect();
    
    // Check if connected
    bool isConnected() const { return connectedSocket != nullptr; }
    
    // Get value from connected socket if input, or from self if output
    template<typename T>
    T getValue() const;
};

// Parameter for node configuration
struct NodeParameter {
    enum class Type {
        Slider,         // Slider control (min, max, value)
        Color,          // Color picker
        Checkbox,       // Boolean checkbox
        Dropdown,       // Dropdown selection
        Button,         // Button trigger
        Text,           // Text input
        FilePath        // File path selector
    };
    
    std::string id;
    std::string name;
    Type type;
    
    // Parameter values
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float value = 0.0f;
    int intValue = 0;
    bool boolValue = false;
    std::string stringValue;
    std::vector<std::string> options;  // For dropdown
    
    std::function<void()> callback;  // Callback when parameter changes
    
    // Constructor for numeric slider
    NodeParameter(const std::string& id, const std::string& name, float min, float max, float defaultValue)
        : id(id), name(name), type(Type::Slider), minValue(min), maxValue(max), value(defaultValue) {}
    
    // Constructor for checkbox
    NodeParameter(const std::string& id, const std::string& name, bool defaultValue)
        : id(id), name(name), type(Type::Checkbox), boolValue(defaultValue) {}
    
    // Constructor for dropdown
    NodeParameter(const std::string& id, const std::string& name, 
                  const std::vector<std::string>& options, int defaultIndex = 0)
        : id(id), name(name), type(Type::Dropdown), options(options), intValue(defaultIndex) {}
    
    // Constructor for text
    NodeParameter(const std::string& id, const std::string& name, const std::string& defaultValue)
        : id(id), name(name), type(Type::Text), stringValue(defaultValue) {}
    
    // Constructor for button
    static NodeParameter createButton(const std::string& id, const std::string& name, 
                                     std::function<void()> callback) {
        NodeParameter param(id, name, 0.0f, 0.0f, 0.0f); // Dummy values
        param.type = Type::Button;
        param.callback = callback;
        return param;
    }
    
    // Constructor for file path
    static NodeParameter createFilePath(const std::string& id, const std::string& name, 
                                       const std::string& defaultPath = "") {
        NodeParameter param(id, name, defaultPath);
        param.type = Type::FilePath;
        return param;
    }
};

// Base node class that all node types will inherit from
class BaseNode {
public:
    enum class NodeType {
        Input,      // Nodes that generate or import data
        Processing, // Nodes that process data
        Output      // Nodes that output or export data
    };
    
    BaseNode(const std::string& id, const std::string& name, NodeType type, NodeGraph* graph);
    virtual ~BaseNode() = default;
    
    // Core functionality
    virtual void process() = 0;           // Process inputs and produce outputs
    virtual void renderProperties() = 0;  // Render property UI 
    virtual void renderPreview() = 0;     // Render node preview
    
    // Node management
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    NodeType getType() const { return type; }
    
    // Socket management
    NodeSocket* addInputSocket(const std::string& name, DataType dataType);
    NodeSocket* addOutputSocket(const std::string& name, DataType dataType);
    NodeSocket* getInputSocket(const std::string& id);
    NodeSocket* getOutputSocket(const std::string& id);
    const std::vector<NodeSocket>& getInputSockets() const { return inputs; }
    const std::vector<NodeSocket>& getOutputSockets() const { return outputs; }
    
    // Parameter management
    void addParameter(const NodeParameter& param);
    NodeParameter* getParameter(const std::string& id);
    
    // Cache management
    void invalidateCache();
    bool isDirty() const { return dirty; }
    void setDirty(bool value) { dirty = value; }
    
    // Position for UI
    void setPosition(float x, float y) { posX = x; posY = y; }
    float getPositionX() const { return posX; }
    float getPositionY() const { return posY; }
    
    // Get output value
    cv::Mat getOutputImage(const std::string& socketId);
    
protected:
    std::string id;
    std::string name;
    NodeType type;
    NodeGraph* parentGraph;
    
    std::vector<NodeSocket> inputs;
    std::vector<NodeSocket> outputs;
    std::unordered_map<std::string, NodeParameter> parameters;
    
    cv::Mat cachedResult;
    bool dirty = true;
    
    float posX = 0.0f;
    float posY = 0.0f;
};

// Node connection class to represent connections between nodes
class NodeConnection {
public:
    NodeConnection(NodeSocket* output, NodeSocket* input)
        : outputSocket(output), inputSocket(input) {}
    
    NodeSocket* getOutputSocket() const { return outputSocket; }
    NodeSocket* getInputSocket() const { return inputSocket; }
    
private:
    NodeSocket* outputSocket;
    NodeSocket* inputSocket;
};

// Node graph manages all nodes and their connections
class NodeGraph {
public:
    NodeGraph();
    ~NodeGraph();
    
    // Node management
    void addNode(std::unique_ptr<BaseNode> node);
    void removeNode(const std::string& id);
    BaseNode* getNode(const std::string& id);
    std::vector<BaseNode*> getAllNodes();
    
    // Connection management
    bool connect(const std::string& outputNodeId, const std::string& outputSocketId,
                const std::string& inputNodeId, const std::string& inputSocketId);
    void disconnect(const std::string& nodeId, const std::string& socketId);
    std::vector<NodeConnection> getConnections() const { return connections; }
    
    // Processing
    void processGraph();
    std::vector<BaseNode*> getExecutionOrder();
    bool detectCycles();
    
    // UI
    void render();
    
private:
    std::unordered_map<std::string, std::unique_ptr<BaseNode>> nodes;
    std::vector<NodeConnection> connections;
    std::vector<BaseNode*> executionOrder;
    
    void buildExecutionOrder();
    void processNode(BaseNode* node);
    
    // Helper for cycle detection
    bool detectCyclesHelper(BaseNode* node, std::unordered_set<BaseNode*>& visited, 
                           std::unordered_set<BaseNode*>& recursionStack);
};

} // namespace ImageProcessor