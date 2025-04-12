cmake_minimum_required(VERSION 3.10)
project(NodeBasedImageProcessor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find GLFW
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends
)

# Add ImGui source files
set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

# Add project source files
file(GLOB PROJECT_SOURCES
    src/*.cpp
)

# Create executable
add_executable(NodeBasedImageProcessor ${PROJECT_SOURCES} ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(NodeBasedImageProcessor ${OpenCV_LIBS} glfw OpenGL32)