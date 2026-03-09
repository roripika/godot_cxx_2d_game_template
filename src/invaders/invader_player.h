#ifndef INVADER_PLAYER_H
#define INVADER_PLAYER_H

#include <godot_cpp/classes/area2d.hpp>

namespace invaders {

class InvaderPlayer : public godot::Area2D {
  GDCLASS(InvaderPlayer, godot::Area2D)

private:
  float move_speed = 300.0f;

protected:
  static void _bind_methods();

public:
  InvaderPlayer();
  ~InvaderPlayer();

  void _physics_process(double delta) override;
  void shoot();

  void set_move_speed(float p_speed);
  float get_move_speed() const;
};

} // namespace invaders

#endif // INVADER_PLAYER_H
