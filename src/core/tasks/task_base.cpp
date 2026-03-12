#include "task_base.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void TaskBase::_bind_methods() {
  ClassDB::bind_method(D_METHOD("complete_instantly"), &TaskBase::complete_instantly);
}

// execute と validate_and_setup は純粋仮想関数なので実装なし。

} // namespace karakuri
