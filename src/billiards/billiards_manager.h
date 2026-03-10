#ifndef BILLIARDS_MANAGER_H
#define BILLIARDS_MANAGER_H

#include <godot_cpp/classes/node.hpp>

namespace billiards {

class BilliardsManager : public godot::Node {
  GDCLASS(BilliardsManager, godot::Node)

private:
  // Opaque pointer to Jolt-specific data to avoid header leakage and name
  // conflicts
  struct JoltData;
  JoltData *jolt_data = nullptr;

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  BilliardsManager();
  ~BilliardsManager() override;

  void start_simulation();
  void spawn_ball(int p_id, godot::Vector3 p_position,
                  bool p_is_cue_ball = false);
  void strike_cue_ball(godot::Vector3 p_direction, float p_power);
  void respawn_cue_ball();
  void _physics_process(double delta) override;
};

} // namespace billiards

#endif // BILLIARDS_MANAGER_H
