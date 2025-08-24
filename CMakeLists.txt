cmake_minimum_required(VERSION 3.5.0)
project(AeroSLR VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On Windows, ensure we use the proper runtime library
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

add_executable(AeroSLR 
    src/main.cpp
    dependencies/ImGUI/imgui.cpp
    dependencies/ImGUI/imgui_demo.cpp
    dependencies/ImGUI/imgui_draw.cpp
    dependencies/ImGUI/imgui_tables.cpp
    dependencies/ImGUI/imgui_widgets.cpp
    dependencies/ImGUI/backends/imgui_impl_glfw.cpp
    dependencies/ImGUI/backends/imgui_impl_opengl3.cpp
    dependencies/glad/src/glad.c
)

target_include_directories(AeroSLR PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/examples/libs/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include"
    # Project's bundled headers (GLM, KHR, etc.)
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Try to locate GLM headers and add their include directory
set(GLM_FOUND FALSE)
set(_GLM_CANDIDATE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/Users/oscar/Downloads/glad/include"
    "C:/Users/oscar/Downloads/glad (1)/include"
)
foreach(_dir IN LISTS _GLM_CANDIDATE_DIRS)
    if (EXISTS "${_dir}/glm/glm.hpp")
        message(STATUS "Found GLM in: ${_dir}")
        target_include_directories(AeroSLR PRIVATE "${_dir}")
        set(GLM_FOUND TRUE)
        break()
    endif()
endforeach()
if (NOT GLM_FOUND)
    message(WARNING "GLM not found in default locations. Set an include path containing glm/glm.hpp if build fails.")
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(AeroSLR PRIVATE OpenGL::GL)

# For Windows
if(WIN32)
    # Set console subsystem but allow proper runtime library linking
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
    target_link_libraries(AeroSLR PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGUI/examples/libs/glfw/lib-vc2010-64/glfw3.lib"
        gdi32 shell32
        # Add necessary runtime libraries
        legacy_stdio_definitions
    )
else()
    # For Linux
    find_package(glfw3 REQUIRED)
    target_link_libraries(AeroSLR PRIVATE glfw)
endif()
