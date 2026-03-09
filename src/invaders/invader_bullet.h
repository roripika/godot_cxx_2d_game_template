#ifndef INVADER_BULLET_H
#define INVADER_BULLET_H

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace invaders {

class InvaderBullet : public godot::Area2D {
  GDCLASS(InvaderBullet, godot::Area2D)

private:
  float speed_ = 400.0f;
  godot::Vector2 direction_ = godot::Vector2(0, -1);

protected:
  static void _bind_methods();

public:
  InvaderBullet();
  ~InvaderBullet();

  void _physics_process(double delta) override;

  void set_speed(float p_speed);
  float get_speed() const;

  void set_direction(const godot::Vector2 &p_direction);
  godot::Vector2 get_direction() const;
};

} // namespace invaders

#endif // INVADER_BULLET_H
