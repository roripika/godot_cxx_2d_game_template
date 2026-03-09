#ifndef INVADER_BARRIER_H
#define INVADER_BARRIER_H

#include <godot_cpp/classes/area2d.hpp>

namespace invaders {

class InvaderBarrier : public godot::Area2D {
  GDCLASS(InvaderBarrier, godot::Area2D)

  int health_ = 3;

protected:
  static void _bind_methods();

public:
  InvaderBarrier();
  ~InvaderBarrier() override;

  void on_area_entered(godot::Area2D *area);

  void set_health(int v);
  int get_health() const;
};

} // namespace invaders

#endif // INVADER_BARRIER_H
