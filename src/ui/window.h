#pragma once

struct GLFWwindow;

namespace bass {

class Window {
public:
  Window();
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool open() const;
  void begin_frame();
  void end_frame();

private:
  GLFWwindow* handle_{};
  float last_scale_{1.f};
};

} // namespace bass
