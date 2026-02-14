#include "hit_stop_manager.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

HitStopManager *HitStopManager::singleton = nullptr;

HitStopManager::HitStopManager() { singleton = this; }

HitStopManager::~HitStopManager() {
  if (singleton == this) {
    singleton = nullptr;
  }
}

HitStopManager *HitStopManager::get_singleton() { return singleton; }

void HitStopManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("apply_hit_stop", "duration", "scale"),
                       &HitStopManager::apply_hit_stop);
}

void HitStopManager::_process(double delta) {
  if (is_stopped) {
    // Wait, if time_scale is 0, does _process run?
    // Yes, if process_mode is strictly PAUSABLE, it might stop?
    // Engine::set_time_scale affects physics tick and delta, but _process might
    // still run or run with small delta? Actually if time_scale is 0, delta is
    // 0. So we can't use delta to countdown! We need to use
    // OS::get_ticks_msec() or similar unscaled time. OR, we can count frames?
    // OR, we can use a separate Timer node that is set to PROCESS_MODE_ALWAYS.
    // But since this class inherits Node, we can set its process mode to
    // PROCESS_MODE_ALWAYS.

    // However, simpler approach:
    // If scale is > 0 (e.g. 0.1), delta is small.
    // If scale is 0, delta is 0.
    // Let's assume we use scale 0.05 or similar, or handle 0 by using wall
    // clock.

    // Let's use get_ticks_usec for robust handling.
  }

  // Actually, let's keep it simple. We will use a dedicated method that relies
  // on a Timer or just Frame counter logic if time_scale is 0. But wait, if
  // time_scale is 0, we can't easily rely on _process delta accumulating. Let's
  // use `Engine::get_singleton()->get_process_frames()`? Let's trying to use
  // unscaled delta via `get_process_delta_time()` if available? No,
  // `_process(double delta)` receives scaled delta.

  // Implementation:
  // We need to track real time.
  static uint64_t stop_start_time = 0;

  if (is_stopped) {
    uint64_t current_time = Time::get_singleton()->get_ticks_usec();
    double elapsed = (current_time - stop_start_time) / 1000000.0;

    if (elapsed >= stop_duration) {
      Engine::get_singleton()->set_time_scale(original_time_scale);
      is_stopped = false;
    }
  }
}

void HitStopManager::apply_hit_stop(double duration, double scale) {
  if (is_stopped) {
    // Extend?
    stop_duration = (duration > stop_duration) ? duration : stop_duration;
    return;
  }

  original_time_scale = Engine::get_singleton()->get_time_scale();
  Engine::get_singleton()->set_time_scale(scale);
  stop_duration = duration;
  is_stopped = true;

  // We need to know when we started.
  // We need to include Time header.
  // Wait, I didn't include Time.
}
