cmake_minimum_required(VERSION 3.5.0)
project(hello_window VERSION 1.0.0)

add_executable(hello_window 
    src/main.cpp
    dependencies/ImGUI/imgui.cpp
    dependencies/ImGUI/imgui_demo.cpp
    dependencies/ImGUI/imgui_draw.cpp
    dependencies/ImGUI/imgui_tables.cpp
    dependencies/ImGUI/imgui_widgets.cpp
    dependencies/ImGUI/backends/imgui_impl_glfw.cpp
    dependencies/ImGUI/backends/imgui_impl_opengl3.cpp
)

target_include_directories(hello_window PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/examples/libs/glfw/include"
)

find_package(OpenGL REQUIRED)
target_link_libraries(hello_window PRIVATE OpenGL::GL)

# For Windows
if(WIN32)
    target_link_libraries(hello_window PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/examples/libs/glfw/lib-vc2010-64/glfw3.lib"
        gdi32 shell32
    )
endif()
