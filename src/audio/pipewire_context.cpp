#include "audio/pipewire_context.h"

#include <stdexcept>

namespace bass {

PipeWireContext::PipeWireContext() {
  pw_init(nullptr, nullptr);
  loop_ = pw_thread_loop_new("bass-sim-pipewire", nullptr);
  if (!loop_) throw std::runtime_error("PipeWire event loop could not be created");
  context_ = pw_context_new(pw_thread_loop_get_loop(loop_), nullptr, 0);
  if (!context_) {
    pw_thread_loop_destroy(loop_);
    throw std::runtime_error("PipeWire context could not be created");
  }
  core_ = pw_context_connect(context_, nullptr, 0);
  if (!core_) {
    pw_context_destroy(context_);
    pw_thread_loop_destroy(loop_);
    throw std::runtime_error("PipeWire server is unavailable");
  }
  if (pw_thread_loop_start(loop_) < 0) {
    pw_core_disconnect(core_);
    pw_context_destroy(context_);
    pw_thread_loop_destroy(loop_);
    throw std::runtime_error("PipeWire event loop could not start");
  }
}

PipeWireContext::~PipeWireContext() {
  pw_thread_loop_stop(loop_);
  pw_core_disconnect(core_);
  pw_context_destroy(context_);
  pw_thread_loop_destroy(loop_);
  pw_deinit();
}

} // namespace bass
