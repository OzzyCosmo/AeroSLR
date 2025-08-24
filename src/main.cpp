// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <glad/glad.h> // Should be included before glfw3.h
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h" // For DockBuilder APIs

#include <stdio.h>
#include <math.h>
#include <vector>
#define GL_SILENCE_DEPRECATION
#include <string>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

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
    // GL 3.3 + GLSL 330 - Updated for modern OpenGL
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.3+ Core Profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.3+ forward compatible
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsDark();
    
    // Setup scaling
    float main_scale = 1.0f;
    if (monitor) // It's possible we don't have a monitor (e.g. when running headless)
        main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor);
    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.0f; // Ensure full opacity
    
    // Customize colors
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);  // App windows
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);   // Panels
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.98f);  // Popups/modals
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f); // Menu bar
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.65f); // Darken background behind modals
    
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

    // STATES/VALUES --------------------------------------------
    bool show_scene_hierarchy_window = true;
    bool show_console_window = true;
    bool show_inspector_window = true; 
    bool show_properties_window = true;
    bool show_viewport_window = true;
    bool show_viewport_toolbar_window = true;

    bool viewport_wireframe = false;
    
    // Triangle management - use vector to track individual triangles
    std::vector<int> triangle_ids;
    // Parallel vector to store persistent triangle names (so InputText has stable storage)
    std::vector<std::string> triangle_names;
    int next_triangle_id = 0;
    // Rename popup state
    int rename_target = -1;
    char rename_buf[64] = {0};
    // Deferred popup triggers (open at root ID stack)
    bool open_about_popup = false;
    bool open_rename_popup = false;

    {
        char default_name[32];
        snprintf(default_name, sizeof(default_name), "Triangle %d", next_triangle_id);
        triangle_ids.push_back(next_triangle_id);
        triangle_names.push_back(std::string(default_name));
        next_triangle_id++;
    }

    ImVec4 clear_color = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);  // WINDOW BACKGROUND (very dark)

    std::string Version_number = "0.1.0-alpha";
    
    // FPS COUNTER
    double previous_time = glfwGetTime();
    int frame_count = 0;
    float fps = 0.0f;

    // Track the on-screen canvas rect used for OpenGL rendering inside the ImGui Viewport window
    ImVec2 viewport_canvas_pos = ImVec2(0.0f, 0.0f);   // Top-left in ImGui screen space
    ImVec2 viewport_canvas_size = ImVec2(0.0f, 0.0f);  // Size in pixels (ImGui screen space)

    // Remember dock node ids for toolbar and center viewport to keep windows in place
    static ImGuiID g_dock_id_toolbar = 0;
    static ImGuiID g_dock_id_center = 0;

    // OPENGL STUFF HERE

    // SHADERS
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main()
        {
            // aPos is a vec2, expand to vec4 with z=0.0
            gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        void main()
        {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Bright white color
        }
    )";
    
    // ogl
    // Transform will be computed per-frame in the render loop

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Query uniform locations for model/view/projection so we can upload matrices later
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Clean up shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // TRIANGLE 🔺
    GLfloat verts[] = {
        +0.5f, +0.5f,   // Top vertex
        -0.5f, -0.5f,   // Bottom left
        +0.5f, -0.5f,   // Bottom right

        -0.5f, +0.5f,   // Top vertex
        -0.5f, -0.5f,   // Bottom left
        +0.5f, +0.5f    // Top right
    };

    // Create VAO (Vertex Array Object) - REQUIRED for Core Profile
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Create and setup VBO (Vertex Buffer Object)
    GLuint vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Setup vertex attributes (must be done while VAO is bound)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Unbind VAO (good practice)
    glBindVertexArray(0);

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

        // Simple DockSpace for resizable panels
        {
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);
            
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
                
            ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
            
            ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            
            // Build initial dock layout to create shared splitters
            static bool dock_layout_built = false;
            if (!dock_layout_built)
            {
                dock_layout_built = true;
                
                // Clear any existing layout
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, vp->WorkSize);
                
                // Create the main layout: Left | Center | Right
                //                         Bottom spans Left+Center
                ImGuiID dock_left, dock_center, dock_right, dock_bottom;
                
                // Split main area: Left (500px) | Remaining
                ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 500.0f / vp->WorkSize.x, &dock_left, &dock_center);
                
                // Split remaining: Center | Right (500px) 
                ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Right, 500.0f / (vp->WorkSize.x - 500.0f), &dock_right, &dock_center);
                
                // Split bottom from center area: Center | Bottom (500px)
                ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Down, 500.0f / vp->WorkSize.y, &dock_bottom, &dock_center);
                
                // Split center vertically for toolbar and viewport: Toolbar (40px) | Viewport
                ImGuiID dock_toolbar, dock_viewport;
                ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Up, 40.0f / (vp->WorkSize.y - 500.0f), &dock_toolbar, &dock_viewport);
                
                // Configure toolbar node to have no tabs
                if (ImGuiDockNode* toolbar_node = ImGui::DockBuilderGetNode(dock_toolbar))
                    toolbar_node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
                
                // Split right panel: Inspector (top) | Properties (bottom)
                ImGuiID dock_right_top, dock_right_bottom;
                ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Up, 0.6f, &dock_right_top, &dock_right_bottom);
                
                // Dock all windows to create shared splitters
                ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left);
                ImGui::DockBuilderDockWindow("Console", dock_bottom);  
                ImGui::DockBuilderDockWindow("Inspector", dock_right_top);
                ImGui::DockBuilderDockWindow("Properties", dock_right_bottom);
                ImGui::DockBuilderDockWindow("Viewport Toolbar", dock_toolbar);
                ImGui::DockBuilderDockWindow("Viewport", dock_viewport);
                
                ImGui::DockBuilderFinish(dockspace_id);
            }
            
            ImGui::End();
        }

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
                if (ImGui::MenuItem("About AeroSLR"))
                {
                    // Defer popup open to root to avoid ID stack mismatch
                    open_about_popup = true;
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

        // ABOUT WINDOW - moved to render on top

        // SCENE HIERARCHY WINDOW

        if (show_scene_hierarchy_window)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, ImGui::GetIO().DisplaySize.y - 530), ImGuiCond_FirstUseEver);
            
            ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

            if (ImGui::Button("Add..."))
            {
                ImGui::OpenPopup("Add...");
            }

        if (ImGui::BeginPopup("Add..."))
            {
                ImGui::Text("Select an object to add:");
                ImGui::Separator();
                if (ImGui::MenuItem("Triangle"))
                {
            // Create new triangle id and default name
            char default_name[32];
            snprintf(default_name, sizeof(default_name), "Triangle %d", next_triangle_id);
            triangle_ids.push_back(next_triangle_id);
            triangle_names.push_back(std::string(default_name));
            next_triangle_id++;
            ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::Separator();

            // Track which triangle to delete (if any)
            int triangle_to_delete = -1;

            for (int i = 0; i < triangle_ids.size(); i++)
            {
                ImGui::PushID(triangle_ids[i]);

                // Use the persistent name for display so edits stick
                const char* node_label = triangle_names[i].c_str();

                // Use Selectable instead of TreeNode for right-click functionality
                if (ImGui::Selectable(node_label, false))
                {
                    // left-click selection handling can go here
                }

                if (ImGui::BeginPopupContextItem())
                {
                    ImGui::Text("%s", node_label);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Rename"))
                    {
                        rename_target = i;
                        strncpy_s(rename_buf, sizeof(rename_buf), triangle_names[i].c_str(), _TRUNCATE);
                        // Defer popup open to root to avoid ID stack mismatch
                        open_rename_popup = true;
                    }
                    if (ImGui::MenuItem("Duplicate"))
                    {
                        char default_name[32];
                        snprintf(default_name, sizeof(default_name), "Triangle %d", next_triangle_id);
                        triangle_ids.push_back(next_triangle_id);
                        triangle_names.push_back(std::string(default_name));
                        next_triangle_id++;
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        triangle_to_delete = i;
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
            
            // Delete the triangle outside the loop to avoid iterator issues
            if (triangle_to_delete >= 0)
            {
                triangle_ids.erase(triangle_ids.begin() + triangle_to_delete);
                // Keep names vector in sync
                if (triangle_to_delete < (int)triangle_names.size())
                    triangle_names.erase(triangle_names.begin() + triangle_to_delete);
                // If the rename target was after the deleted index, adjust it
                if (rename_target == triangle_to_delete)
                {
                    rename_target = -1;
                    // Close any open rename popup
                    ImGui::CloseCurrentPopup();
                }
                else if (rename_target > triangle_to_delete)
                    rename_target--;
            }

            ImGui::End();   
        }

        // CONSOLE
        if (show_console_window)
        {
            ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 500), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 500), ImGuiCond_FirstUseEver);

            ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoCollapse);

            ImGui::Text("This panel is not functional...");
            ImGui::End();
        }

        // INSPECTOR WINDOW

        if (show_inspector_window)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_FirstUseEver);

            ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);
            

            ImGui::Text("This panel is not functional...");  
            ImGui::End();   
        } 

        // PROPERTIES WINDOW

        if (show_properties_window)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 500, 730), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, ImGui::GetIO().DisplaySize.y - 530), ImGuiCond_FirstUseEver);

            ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse);
            

            ImGui::Text("This panel is not functional...");  
            ImGui::End();   
        }

        // VIEWPORT TOOLBAR

        if (show_viewport_toolbar_window)
        {
            ImGui::Begin("Viewport Toolbar", nullptr,
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

            if (ImGui::Button("Wireframe View"))
            {
                viewport_wireframe = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Solid View"))
            {
                viewport_wireframe = false;
            }
            
            ImGui::End();
        }


        
        // VIEWPORT WINDOW
        if (show_viewport_window)
        {
            ImGui::Begin("Viewport", nullptr,
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
            
            // Get the full content region available in the viewport window
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            
            // Debug info to see what's happening
            ImGui::Text("Canvas pos: %.1f, %.1f", canvas_pos.x, canvas_pos.y);
            ImGui::Text("Canvas size: %.1f x %.1f", canvas_size.x, canvas_size.y);
            ImGui::Text("Triangle count: %d", (int)triangle_ids.size());
            
            // Adjust canvas size to account for debug text
            canvas_size = ImGui::GetContentRegionAvail();
            
            // Use the remaining content region for the OpenGL canvas
            if (canvas_size.x > 0 && canvas_size.y > 0)
            {
                // Add a colored rectangle to visualize the canvas area
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_end = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
                draw_list->AddRectFilled(canvas_pos, canvas_end, IM_COL32(50, 50, 50, 255));
                draw_list->AddRect(canvas_pos, canvas_end, IM_COL32(255, 255, 255, 255));
                
                // Reserve the entire content space for OpenGL rendering
                ImGui::InvisibleButton("opengl_canvas", canvas_size);
                
                // Store viewport information for later OpenGL rendering (outside ImGui pass)
                viewport_canvas_pos = canvas_pos;
                viewport_canvas_size = canvas_size;
            }
            else
            {
                ImGui::Text("Canvas too small: %.1f x %.1f", canvas_size.x, canvas_size.y);
                // Reset canvas info if viewport is too small
                viewport_canvas_pos = ImVec2(0, 0);
                viewport_canvas_size = ImVec2(0, 0);
            }
            
            ImGui::End();
        } 

    // MODAL/POPUP WINDOWS - Rendered right before ImGui::Render() to appear on top
    // Open any deferred popups at the root ID stack
    if (open_about_popup) { ImGui::OpenPopup("About AeroSLR"); open_about_popup = false; }
    if (open_rename_popup) { ImGui::OpenPopup("Rename Triangle"); open_rename_popup = false; }
        
        // ABOUT WINDOW
        if (ImGui::BeginPopupModal("About AeroSLR", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("AeroSLR v%s", Version_number.c_str());
            ImGui::Separator();
            ImGui::Text("(c) 2025 Oscar Forbes");
            ImGui::Text("AeroSLR (Simple, Lightweight Renderer) by Oscar Forbes");

            ImGui::SeparatorText("Technologies");
            ImGui::Text("Written in - C++");
            ImGui::Text("UI Framework - Dear ImGui");
            ImGui::Text("Graphics API - OpenGL");
            
            ImGui::Separator();
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // RENAME WINDOW
        if (ImGui::BeginPopupModal("Rename Triangle", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Rename Triangle:");
            ImGui::Separator();
            
            // Auto-focus the input field when the window appears
            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();
                
            ImGui::InputText("##NewName", rename_buf, sizeof(rename_buf));
            ImGui::Separator();
            
            if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                if (rename_target >= 0 && rename_target < (int)triangle_names.size())
                {
                    triangle_names[rename_target] = std::string(rename_buf);
                }
                rename_target = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                rename_target = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // PREPARE RENDERING - Set up framebuffer before any rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Clear the entire framebuffer first
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // RENDER IMGUI FIRST to create the UI layout
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // RENDER OPENGL TRIANGLES AFTER IMGUI (in a specific scissor area)
        if (show_viewport_window && viewport_canvas_size.x > 0 && viewport_canvas_size.y > 0)
        {
            ImVec2 canvas_pos = viewport_canvas_pos;
            ImVec2 canvas_size = viewport_canvas_size;

            // Convert ImGui screen-space (top-left origin) to OpenGL framebuffer coords (bottom-left origin)
            float scale_x = (float)display_w / ImGui::GetIO().DisplaySize.x;
            float scale_y = (float)display_h / ImGui::GetIO().DisplaySize.y;

            const ImVec2 main_vp_pos = ImGui::GetMainViewport()->Pos;
            int opengl_viewport_x = (int)((canvas_pos.x - main_vp_pos.x) * scale_x);
            int opengl_viewport_y = (int)((display_h - ((canvas_pos.y - main_vp_pos.y) + canvas_size.y) * scale_y));
            int opengl_viewport_w = (int)(canvas_size.x * scale_x);
            int opengl_viewport_h = (int)(canvas_size.y * scale_y);
            
            // Ensure viewport dimensions are positive and reasonable
            if (opengl_viewport_w > 0 && opengl_viewport_h > 0 && 
                opengl_viewport_x >= 0 && opengl_viewport_y >= 0 &&
                opengl_viewport_x < display_w && opengl_viewport_y < display_h)
            {
                // Set viewport and scissor test to limit rendering to our canvas area
                glViewport(opengl_viewport_x, opengl_viewport_y, opengl_viewport_w, opengl_viewport_h);
                glEnable(GL_SCISSOR_TEST);
                glScissor(opengl_viewport_x, opengl_viewport_y, opengl_viewport_w, opengl_viewport_h);
                
                // Clear only our canvas area with a dark background
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.6f, 0.0f, 1.0f));

                glm::mat4 view = glm::mat4(1.0f);
                view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));

                glm::mat4 projection;
                float aspect = (float)opengl_viewport_w / (float)opengl_viewport_h;
                projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

                // Upload matrices to the shader (ensure program is bound)
                glUseProgram(shaderProgram);
                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                for (int i = 0; i < triangle_ids.size(); i++)
                {
                    glBindVertexArray(VAO);
                    if (viewport_wireframe)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }
                    else 
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glBindVertexArray(0);
                }
                
                // Disable scissor test and restore full viewport
                glDisable(GL_SCISSOR_TEST);
                glViewport(0, 0, display_w, display_h);
            }
        }

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteProgram(shaderProgram);
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}