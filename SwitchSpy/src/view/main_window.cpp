#include "switchspy/view/main_window.hpp"
#include "switchspy/view/connection_tab.hpp"
#include "switchspy/view/log_panel.hpp"
#include "switchspy/view/performance_panel.hpp"
#include "switchspy/controller/connection_manager.hpp"
#include "switchspy/common/logger.hpp"
#include "switchspy/common/benchmark.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

namespace switchspy { namespace view {
MainWindow::MainWindow() : running_(false), window_(nullptr) {}
MainWindow::~MainWindow() { shutdown(); }

bool MainWindow::initialize() {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    window_ = glfwCreateWindow(1280, 720, "SwitchSpy - Network Relay Visualizer", NULL, NULL);
    if (!window_) { glfwTerminate(); return false; }
    
    glfwMakeContextCurrent((GLFWwindow*)window_);
    glfwSwapInterval(1);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    running_ = true;
    return true;
}

void MainWindow::run() {
    while (running_ && !glfwWindowShouldClose((GLFWwindow*)window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        render();
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize((GLFWwindow*)window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers((GLFWwindow*)window_);
    }
}

void MainWindow::render() {
    BENCHMARK_SCOPE_CAT(benchmark::categories::GUI_RENDER);

    renderMenuBar();

    // Main window with connection tabs
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1280, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Connections", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("ConnectionTabs")) {
        auto& connections = controller::ConnectionManager::instance().getAllConnections();
        for (const auto& [name, conn] : connections) {
            if (ImGui::BeginTabItem(name.c_str())) {
                ConnectionTab tab(conn);
                tab.render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

    // Bottom panel with logs and performance tabs
    ImGui::SetNextWindowPos(ImVec2(0, 430), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1280, 290), ImGuiCond_FirstUseEver);
    ImGui::Begin("Monitoring", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("MonitoringTabs")) {
        if (ImGui::BeginTabItem("Logs")) {
            static LogPanel log_panel;
            log_panel.render();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Performance")) {
            static PerformancePanel perf_panel;
            perf_panel.render();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    renderStatusBar();
}

void MainWindow::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) running_ = false;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void MainWindow::renderStatusBar() {
    ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::Text("SwitchSpy Ready");
    ImGui::End();
}

void MainWindow::shutdown() {
    if (window_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow((GLFWwindow*)window_);
        glfwTerminate();
        window_ = nullptr;
    }
}
}} // namespace
