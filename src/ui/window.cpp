#include "ui/window.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace bass {

Window::Window() {
  if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  handle_ = glfwCreateWindow(1100, 760, "Bass-sim", nullptr, nullptr);
  if (!handle_) {
    glfwTerminate();
    throw std::runtime_error("Could not create a graphics window");
  }
  glfwMakeContextCurrent(handle_);
  glfwSwapInterval(1);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  auto& style = ImGui::GetStyle();
  style.WindowRounding = 7.f;
  style.FrameRounding = 5.f;
  style.GrabRounding = 5.f;
  style.WindowPadding = ImVec2(14.f, 12.f);
  style.FramePadding = ImVec2(9.f, 6.f);
  style.ItemSpacing = ImVec2(9.f, 8.f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.065f, 0.075f, 0.11f, 1.f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.13f, 0.36f, 0.39f, 1.f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.11f, 0.35f, 0.37f, 1.f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.52f, 0.53f, 1.f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.61f, 0.59f, 1.f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.29f, 0.75f, 0.65f, 1.f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.88f, 0.72f, 1.f);
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui_ImplGlfw_InitForOpenGL(handle_, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

Window::~Window() {
  if (handle_) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(handle_);
  }
  glfwTerminate();
}

bool Window::open() const { return !glfwWindowShouldClose(handle_); }

void Window::begin_frame() {
  glfwPollEvents();
  float x = 1.f, y = 1.f;
  glfwGetWindowContentScale(handle_, &x, &y);
  const float scale = std::clamp((x + y) * 0.5f, 0.75f, 3.f);
  if (std::abs(scale - last_scale_) > 0.01f) {
    ImGui::GetStyle().ScaleAllSizes(scale / last_scale_);
    last_scale_ = scale;
  }
  ImGui::GetStyle().FontScaleDpi = scale;
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Window::end_frame() {
  ImGui::Render();
  int width = 0, height = 0;
  glfwGetFramebufferSize(handle_, &width, &height);
  glViewport(0, 0, width, height);
  glClearColor(0.06f, 0.07f, 0.1f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(handle_);
}

} // namespace bass
