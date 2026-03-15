#include "kernel_clock.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

KernelClock *KernelClock::singleton_ = nullptr;

KernelClock::KernelClock() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

KernelClock::~KernelClock() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

KernelClock *KernelClock::get_singleton() {
  return singleton_;
}

void KernelClock::_bind_methods() {
  ClassDB::bind_static_method("KernelClock", D_METHOD("get_singleton"), &KernelClock::get_singleton);
  ClassDB::bind_method(D_METHOD("advance", "delta"), &KernelClock::advance);
  ClassDB::bind_method(D_METHOD("now"), &KernelClock::now);
  ClassDB::bind_method(D_METHOD("reset"), &KernelClock::reset);
  ClassDB::bind_method(D_METHOD("set_time", "t"), &KernelClock::set_time);
}

void KernelClock::advance(double delta) {
  time_ += delta;
}

double KernelClock::now() const {
  return time_;
}

void KernelClock::reset() {
  time_ = 0.0;
}

void KernelClock::set_time(double t) {
  time_ = t;
}

} // namespace karakuri
