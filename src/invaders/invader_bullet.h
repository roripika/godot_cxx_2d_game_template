#ifndef INVADER_BULLET_H
#define INVADER_BULLET_H

#include <godot_cpp/classes/area2d.hpp>

namespace invaders {

class InvaderBullet : public godot::Area2D {
  GDCLASS(InvaderBullet, godot::Area2D)

private:
  float speed = -400.0f;

protected:
  static void _bind_methods();

public:
  InvaderBullet();
  ~InvaderBullet();

  void _physics_process(double delta) override;

  void set_speed(float p_speed);
  float get_speed() const;
};

} // namespace invaders

#endif // INVADER_BULLET_H
