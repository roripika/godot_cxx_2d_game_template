#ifndef HIT_STOP_MANAGER_H
#define HIT_STOP_MANAGER_H

#include <godot_cpp/classes/node.hpp>

using namespace godot;

class HitStopManager : public Node {
  GDCLASS(HitStopManager, Node)

private:
  static HitStopManager *singleton;
  double stop_duration = 0.0;
  bool is_stopped = false;
  double original_time_scale = 1.0;

protected:
  static void _bind_methods();

public:
  HitStopManager();
  ~HitStopManager();

  static HitStopManager *get_singleton();

  void _process(double delta) override;

  void apply_hit_stop(double duration, double scale = 0.0);
};

#endif // HIT_STOP_MANAGER_H
