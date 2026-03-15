#ifndef KARAKURI_KERNEL_CLOCK_H
#define KARAKURI_KERNEL_CLOCK_H

#include <godot_cpp/classes/object.hpp>

namespace karakuri {

/**
 * @brief Deterministic time controller for Karakuri Kernel.
 * Ensures tasks and scenarios execute based on a predictable, replayable clock
 * rather than raw frame deltas directly.
 */
class KernelClock : public godot::Object {
  GDCLASS(KernelClock, godot::Object)

public:
  KernelClock();
  ~KernelClock() override;

  static KernelClock *get_singleton();

  void advance(double delta);
  double now() const;

  void reset();
  void set_time(double t);

protected:
  static void _bind_methods();

private:
  static KernelClock *singleton_;
  double time_ = 0.0;
};

} // namespace karakuri

#endif // KARAKURI_KERNEL_CLOCK_H
