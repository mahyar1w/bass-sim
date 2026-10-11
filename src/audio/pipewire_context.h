#pragma once

#include <pipewire/pipewire.h>

namespace bass {

class PipeWireContext {
public:
  PipeWireContext();
  ~PipeWireContext();
  PipeWireContext(const PipeWireContext&) = delete;
  PipeWireContext& operator=(const PipeWireContext&) = delete;

  void lock() noexcept { pw_thread_loop_lock(loop_); }
  void unlock() noexcept { pw_thread_loop_unlock(loop_); }
  pw_core* core() const noexcept { return core_; }
  pw_thread_loop* loop() const noexcept { return loop_; }

private:
  pw_thread_loop* loop_{};
  pw_context* context_{};
  pw_core* core_{};
};

class PipeWireLock {
public:
  explicit PipeWireLock(PipeWireContext& context) : context_(context) { context_.lock(); }
  ~PipeWireLock() { context_.unlock(); }
private:
  PipeWireContext& context_;
};

} // namespace bass
