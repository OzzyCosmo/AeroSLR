// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <glad/glad.h> // Should be included before glfw3.h
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// Fallback definitions for common shader enums in case the platform headers
// or loader did not provide them (prevents 'undeclared identifier' errors).
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <math.h>
#define GL_SILENCE_DEPRECATION
#include <string>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// Vertex Shader source code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
//Fragment Shader source code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"
"}\n\0";

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "AeroSLR", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    // Check OpenGL version
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Renderer: %s\n", renderer);
    fprintf(stdout, "OpenGL version supported %s\n", version);

    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    float main_scale = 1.0f;
    if (monitor) // It's possible we don't have a monitor (e.g. when running headless)
        main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor);
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arial.ttf");
    //IM_ASSERT(font != nullptr);

    // STATES --------------------------------------------
    bool show_about_window = false;

    bool show_scene_hierarchy_window = true;
    bool show_console_window = true;
    bool show_inspector_window = true; 
    bool show_properties_window = true;
    bool show_viewport_window = true;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string Version_number = "0.1.0-alpha";
    
    // FPS COUNTER
    double previous_time = glfwGetTime();
    int frame_count = 0;
    float fps = 0.0f;

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // FPS calculation
        double current_time = glfwGetTime();
        frame_count++;
        if (current_time - previous_time >= 1.0)
        {
            fps = (float)frame_count / (float)(current_time - previous_time);
            previous_time = current_time;
            frame_count = 0;
        }

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // MAIN CODE HERE -------------------------------------------------------------

        // TOOLBAR ----------------------------

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) { /* do something */ }
                if (ImGui::MenuItem("Open...")) { /* do something */ }
                if (ImGui::MenuItem("Save")) { /* do something */ }
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo")) { /* ... */ }
                if (ImGui::MenuItem("Redo")) { /* ... */ }
                ImGui::Separator(); // horizontal line
                if (ImGui::MenuItem("Cut")) { /* ... */ }
                if (ImGui::MenuItem("Copy")) { /* ... */ }
                if (ImGui::MenuItem("Paste")) { /* ... */ }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About AeroSLR")) {
                    show_about_window = true;
                 }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Scene Hierarchy", nullptr, show_scene_hierarchy_window))
                {
                    show_scene_hierarchy_window = !show_scene_hierarchy_window;
                }
                if (ImGui::MenuItem("Console", nullptr, show_console_window))
                {
                    show_console_window = !show_console_window;
                }
                if (ImGui::MenuItem("Inspector", nullptr, show_inspector_window))
                {
                    show_inspector_window = !show_inspector_window;
                }
                if (ImGui::MenuItem("Properties", nullptr, show_properties_window))
                {
                    show_properties_window = !show_properties_window;
                }
                if (ImGui::MenuItem("Viewport", nullptr, show_viewport_window))
                {
                    show_viewport_window = !show_viewport_window;
                }
                ImGui::EndMenu();
            }

            {
                char fps_str[16];
                snprintf(fps_str, sizeof(fps_str), "FPS: %.1f", fps);
                float text_width = ImGui::CalcTextSize(fps_str).x;
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - text_width - ImGui::GetStyle().ItemSpacing.x * 2);
                ImGui::TextUnformatted(fps_str);
            }

            ImGui::EndMainMenuBar();
        }

        // ABOUT WINDOW

        if (show_about_window)
        {
            ImGui::SetNextWindowSize(ImVec2(500.0f, 250.0f));

            ImGui::Begin("About AeroSLR", &show_about_window);
            ImGui::Text("AeroSLR v%s", Version_number.c_str());
            ImGui::Separator();
            ImGui::Text("(c) 2025 Oscar Forbes");
            ImGui::Text("AeroSLR (Slim, Lightweight Renderer) by Oscar Forbes");

            ImGui::SeparatorText("Technologies");
            ImGui::Text("Written in - C++");
            ImGui::Text("UI Framework - Dear ImGui");
            ImGui::Text("Graphics API - OpenGL");
            ImGui::End();
        }

        // SCENE HIERARCHY WINDOW

        if (show_scene_hierarchy_window)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 30), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(500, ImGui::GetIO().DisplaySize.y - 528), ImGuiCond_Always); 

            ImGui::Begin("Scene Hierarchy", nullptr,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse); 
            

            ImGui::Text("This panel is not functional...");  
            ImGui::End();   
        } 

        // CONSOLE

        if (show_console_window)
        {
            ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 500), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 500), ImGuiCond_Always);

            ImGui::Begin("Console", nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::Text("This panel is not functional...");
            ImGui::End();
        }

        // INSPECTOR WINDOW

        if (show_inspector_window)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 30), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_Always); 

            ImGui::Begin("Inspector", nullptr,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse); 
            

            ImGui::Text("This panel is not functional...");  
            ImGui::End();   
        } 

        // PROPERTIES WINDOW

        if (show_properties_window)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 729), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(500, ImGui::GetIO().DisplaySize.y - 528), ImGuiCond_Always);

            ImGui::Begin("Properties", nullptr,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse); 
            

            ImGui::Text("This panel is not functional...");  
            ImGui::End();   
        }
        
        // VIEWPORT WINDOW
        if (show_viewport_window)
        {
            float viewport_x = 500;
            float viewport_y = 30;
            float viewport_width = ImGui::GetIO().DisplaySize.x - 1003; 
            float viewport_height = ImGui::GetIO().DisplaySize.y - 530;
            
            ImGui::SetNextWindowPos(ImVec2(viewport_x, viewport_y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(viewport_width, viewport_height), ImGuiCond_Always);

            ImGui::Begin("Viewport", nullptr,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            
            if (canvas_size.x > 50.0f && canvas_size.y > 50.0f)
            {
                draw_list->AddRectFilled(canvas_pos, 
                                       ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), 
                                       IM_COL32(20, 20, 20, 255)); // VIEWPORT BACKGROUND COLOUR
                
                // Add a border
                draw_list->AddRect(canvas_pos, 
                                 ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), 
                                 IM_COL32(100, 100, 100, 255));
                
                // RENDER OPENGL HERE

                

            }
            else
            {
                ImGui::Text("Viewport too small to render");
            }
            
            ImGui::End();
        } 

        // Rendering
       ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}