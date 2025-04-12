#include "Node.h"
#include <queue>
#include <unordered_set>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace ImageProcessor {

//------------------------------------------------------------------------------
// NodeSocket Implementation
//------------------------------------------------------------------------------

bool NodeSocket::connectTo(NodeSocket* other) {
    // Check for valid connection
    if (!other) return false;
    
    // Check if compatible types
    if (type == other->type) {
        // Can't connect input to input or output to output
        return false;
    }
    
    // Check if data types are compatible
    if (dataType != other->dataType) {
        // For now, only allow connections between same data types
        // Could extend this to allow compatible conversions
        return false;
    }
    
    // Check which is input and which is output
    NodeSocket* inputSocket = (type == Type::Input) ? this : other;
    NodeSocket* outputSocket = (type == Type::Output) ? this : other;
    
    // Check if input socket is already connected
    if (inputSocket->connectedSocket) {
        // Disconnect first
        inputSocket->disconnect();
    }
    
    // Establish connection
    inputSocket->connectedSocket = outputSocket;
    outputSocket->connectedSocket = inputSocket;
    
    // Mark the input node as dirty since it now has a new connection
    inputSocket->parent->setDirty(true);
    
    return true;
}

void NodeSocket::disconnect() {
    if (connectedSocket) {
        // Disconnect from the other socket as well
        connectedSocket->connectedSocket = nullptr;
        
        // Set this socket as having no connection
        connectedSocket = nullptr;
        
        // Mark the parent node as dirty if this is an input socket
        if (type == Type::Input) {
            parent->setDirty(true);
        }
    }
}

//------------------------------------------------------------------------------
// BaseNode Implementation
//------------------------------------------------------------------------------

BaseNode::BaseNode(const std::string& id, const std::string& name, NodeType type, NodeGraph* graph)
    : id(id), name(name), type(type), parentGraph(graph), dirty(true) {
}

NodeSocket* BaseNode::addInputSocket(const std::string& name, DataType dataType) {
    std::string socketId = "in_" + name + "_" + std::to_string(inputs.size());
    inputs.emplace_back(socketId, name, NodeSocket::Type::Input, dataType, this);
    return &inputs.back();
}

NodeSocket* BaseNode::addOutputSocket(const std::string& name, DataType dataType) {
    std::string socketId = "out_" + name + "_" + std::to_string(outputs.size());
    outputs.emplace_back(socketId, name, NodeSocket::Type::Output, dataType, this);
    return &outputs.back();
}

NodeSocket* BaseNode::getInputSocket(const std::string& id) {
    for (auto& socket : inputs) {
        if (socket.id == id) {
            return &socket;
        }
    }
    return nullptr;
}

NodeSocket* BaseNode::getOutputSocket(const std::string& id) {
    for (auto& socket : outputs) {
        if (socket.id == id) {
            return &socket;
        }
    }
    return nullptr;
}

void BaseNode::addParameter(const NodeParameter& param) {
    parameters[param.id] = param;
}

NodeParameter* BaseNode::getParameter(const std::string& id) {
    auto it = parameters.find(id);
    if (it != parameters.end()) {
        return &it->second;
    }
    return nullptr;
}

void BaseNode::invalidateCache() {
    dirty = true;
    
    // Also invalidate all nodes that depend on this one
    for (auto& outputSocket : outputs) {
        for (auto& connection : parentGraph->getConnections()) {
            if (connection.getOutputSocket() == &outputSocket) {
                connection.getInputSocket()->parent->invalidateCache();
            }
        }
    }
}

cv::Mat BaseNode::getOutputImage(const std::string& socketId) {
    NodeSocket* socket = getOutputSocket(socketId);
    if (!socket || socket->dataType != DataType::Image) {
        return cv::Mat();
    }
    
    // Ensure we have processed data
    if (dirty) {
        process();
        dirty = false;
    }
    
    return socket->value;
}

//------------------------------------------------------------------------------
// NodeGraph Implementation
//------------------------------------------------------------------------------

NodeGraph::NodeGraph() {
}

NodeGraph::~NodeGraph() {
    // Nodes will be automatically cleaned up via unique_ptr
}

void NodeGraph::addNode(std::unique_ptr<BaseNode> node) {
    if (!node) return;
    
    std::string id = node->getId();
    nodes[id] = std::move(node);
    
    // Mark that execution order needs to be rebuilt
    executionOrder.clear();
}

void NodeGraph::removeNode(const std::string& id) {
    auto nodeIt = nodes.find(id);
    if (nodeIt == nodes.end()) {
        return;
    }
    
    // Disconnect all sockets
    BaseNode* node = nodeIt->second.get();
    
    // Disconnect inputs
    for (auto& socket : node->getInputSockets()) {
        if (socket.connectedSocket) {
            socket.disconnect();
        }
    }
    
    // Disconnect outputs
    for (auto& socket : node->getOutputSockets()) {
        if (socket.connectedSocket) {
            socket.disconnect();
        }
    }
    
    // Remove all connections related to this node
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [node](const NodeConnection& conn) {
                return conn.getInputSocket()->parent == node || 
                       conn.getOutputSocket()->parent == node;
            }),
        connections.end()
    );
    
    // Remove the node
    nodes.erase(nodeIt);
    
    // Mark that execution order needs to be rebuilt
    executionOrder.clear();
}

BaseNode* NodeGraph::getNode(const std::string& id) {
    auto it = nodes.find(id);
    if (it != nodes.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<BaseNode*> NodeGraph::getAllNodes() {
    std::vector<BaseNode*> result;
    result.reserve(nodes.size());
    for (auto& pair : nodes) {
        result.push_back(pair.second.get());
    }
    return result;
}

bool NodeGraph::connect(const std::string& outputNodeId, const std::string& outputSocketId,
                        const std::string& inputNodeId, const std::string& inputSocketId) {
    // Get the nodes
    BaseNode* outputNode = getNode(outputNodeId);
    BaseNode* inputNode = getNode(inputNodeId);
    
    if (!outputNode || !inputNode) {
        return false;
    }
    
    // Get the sockets
    NodeSocket* outputSocket = outputNode->getOutputSocket(outputSocketId);
    NodeSocket* inputSocket = inputNode->getInputSocket(inputSocketId);
    
    if (!outputSocket || !inputSocket) {
        return false;
    }
    
    // Establish connection
    if (inputSocket->connectTo(outputSocket)) {
        connections.emplace_back(outputSocket, inputSocket);
        
        // Rebuild execution order
        executionOrder.clear();
        
        return true;
    }
    
    return false;
}

void NodeGraph::disconnect(const std::string& nodeId, const std::string& socketId) {
    BaseNode* node = getNode(nodeId);
    if (!node) {
        return;
    }
    
    // Try to find as input socket first
    NodeSocket* socket = node->getInputSocket(socketId);
    if (!socket) {
        // Try as output socket
        socket = node->getOutputSocket(socketId);
    }
    
    if (!socket || !socket->connectedSocket) {
        return;
    }
    
    // Disconnect socket
    socket->disconnect();
    
    // Remove connection from list
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [socket](const NodeConnection& conn) {
                return conn.getInputSocket() == socket || 
                       conn.getOutputSocket() == socket;
            }),
        connections.end()
    );
    
    // Mark that execution order needs to be rebuilt
    executionOrder.clear();
}

void NodeGraph::processGraph() {
    // If execution order is empty, build it
    if (executionOrder.empty()) {
        buildExecutionOrder();
    }
    
    // Process nodes in order
    for (BaseNode* node : executionOrder) {
        if (node->isDirty()) {
            node->process();
            node->setDirty(false);
        }
    }
}

std::vector<BaseNode*> NodeGraph::getExecutionOrder() {
    if (executionOrder.empty()) {
        buildExecutionOrder();
    }
    return executionOrder;
}

bool NodeGraph::detectCycles() {
    std::unordered_set<BaseNode*> visited;
    std::unordered_set<BaseNode*> recursionStack;
    
    for (auto& pair : nodes) {
        BaseNode* node = pair.second.get();
        if (detectCyclesHelper(node, visited, recursionStack)) {
            return true;  // Cycle detected
        }
    }
    
    return false;  // No cycles detected
}

void NodeGraph::buildExecutionOrder() {
    // Clear existing order
    executionOrder.clear();
    
    // Check for cycles
    if (detectCycles()) {
        throw std::runtime_error("Cycle detected in node graph. Cannot determine execution order.");
    }
    
    // Build execution order using topological sort
    std::unordered_set<BaseNode*> visited;
    std::unordered_set<BaseNode*> added;
    
    // Helper function for topological sort
    std::function<void(BaseNode*)> visit = [&](BaseNode* node) {
        if (added.find(node) != added.end()) {
            return;  // Already in execution order
        }
        
        visited.insert(node);
        
        // Visit all nodes that this one depends on (input connections)
        for (auto& socket : node->getInputSockets()) {
            if (socket.connectedSocket) {
                BaseNode* dependencyNode = socket.connectedSocket->parent;
                if (visited.find(dependencyNode) == visited.end()) {
                    visit(dependencyNode);
                }
            }
        }
        
        // Add this node to execution order
        executionOrder.push_back(node);
        added.insert(node);
    };
    
    // Visit all nodes
    for (auto& pair : nodes) {
        visit(pair.second.get());
    }
}

bool NodeGraph::detectCyclesHelper(BaseNode* node, std::unordered_set<BaseNode*>& visited, 
                                  std::unordered_set<BaseNode*>& recursionStack) {
    if (visited.find(node) == visited.end()) {
        // Mark the current node as visited and part of recursion stack
        visited.insert(node);
        recursionStack.insert(node);
        
        // Check all nodes that this one points to (via outputs)
        for (auto& socket : node->getOutputSockets()) {
            if (socket.connectedSocket) {
                BaseNode* dependentNode = socket.connectedSocket->parent;
                
                // If the node is not visited, then check its dependencies
                if (visited.find(dependentNode) == visited.end()) {
                    if (detectCyclesHelper(dependentNode, visited, recursionStack)) {
                        return true;  // Cycle detected in subtree
                    }
                } 
                // If the node is already in recursion stack, then there is a cycle
                else if (recursionStack.find(dependentNode) != recursionStack.end()) {
                    return true;  // Cycle detected
                }
            }
        }
    }
    
    // Remove the node from recursion stack
    recursionStack.erase(node);
    
    return false;  // No cycle detected
}

void NodeGraph::render() {
    // This will be implemented with ImGui
    // For now this is just a placeholder
}

} // namespace ImageProcessor