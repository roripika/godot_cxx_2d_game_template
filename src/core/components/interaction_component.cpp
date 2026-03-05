#include "interaction_component.h"

#include "services/input_service.h"

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

InteractionComponent::InteractionComponent() : detection_radius_(64.0f) {}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void InteractionComponent::_bind_methods() {
  // Properties
  ClassDB::bind_method(D_METHOD("set_detection_radius", "radius"),
                       &InteractionComponent::set_detection_radius);
  ClassDB::bind_method(D_METHOD("get_detection_radius"),
                       &InteractionComponent::get_detection_radius);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detection_radius",
                            PROPERTY_HINT_RANGE, "8,512,1,suffix:px"),
               "set_detection_radius", "get_detection_radius");

  // State queries
  ClassDB::bind_method(D_METHOD("has_focus"),
                       &InteractionComponent::has_focus);
  ClassDB::bind_method(D_METHOD("get_focused_node"),
                       &InteractionComponent::get_focused_node);

  // Signals
  ADD_SIGNAL(MethodInfo("focus_changed",
                        PropertyInfo(Variant::OBJECT, "node",
                                     PROPERTY_HINT_NODE_TYPE, "Node"),
                        PropertyInfo(Variant::BOOL, "is_focused")));
  ADD_SIGNAL(MethodInfo("interacted",
                        PropertyInfo(Variant::OBJECT, "node",
                                     PROPERTY_HINT_NODE_TYPE, "Node")));
}

// ---------------------------------------------------------------------------
// Physics process — 毎フレーム呼ばれる本体ロジック
// ---------------------------------------------------------------------------

void InteractionComponent::_physics_process(double /*delta*/) {
  SceneTree *tree = get_tree();
  if (!tree) {
    return;
  }

  // --- 1. 親ノードの位置を取得 ---
  Node2D *parent = Object::cast_to<Node2D>(get_parent());
  if (!parent) {
    return; // InteractionComponent は Node2D 系の子として使う
  }
  const Vector2 origin = parent->get_global_position();

  // --- 2. "interactable" グループのノードをスキャン ---
  TypedArray<Node> candidates = tree->get_nodes_in_group("interactable");

  IInteractable *best_target = nullptr;
  Node *best_node = nullptr;
  float best_dist_sq = detection_radius_ * detection_radius_;

  const int count = candidates.size();
  for (int i = 0; i < count; ++i) {
    Node *n = Object::cast_to<Node>(candidates[i]);
    if (!n || n == parent) {
      continue;
    }

    // IInteractable かどうかを dynamic_cast で判定
    IInteractable *target = dynamic_cast<IInteractable *>(n);
    if (!target) {
      continue;
    }

    // Node2D として位置を取得
    Node2D *n2d = Object::cast_to<Node2D>(n);
    if (!n2d) {
      continue;
    }

    const float dist_sq =
        origin.distance_squared_to(n2d->get_global_position());
    if (dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      best_target = target;
      best_node = n;
    }
  }

  // --- 3. フォーカス状態の更新 ---
  if (best_node != focused_node_) {
    // 古いフォーカスを解除
    if (focused_node_ != nullptr) {
      emit_signal("focus_changed", focused_node_, false);
    }

    focused_ = best_target;
    focused_node_ = best_node;

    // 新しいフォーカスを通知
    if (focused_node_ != nullptr) {
      emit_signal("focus_changed", focused_node_, true);
    }
  }

  // --- 4. 入力チェック — interact アクションで on_interact() を起動 ---
  if (focused_ == nullptr) {
    return;
  }

  InputService *input_svc = InputService::get_singleton();
  if (!input_svc) {
    return;
  }

  if (input_svc->is_action_just_pressed("interact")) {
    focused_->on_interact();
    emit_signal("interacted", focused_node_);
  }
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void InteractionComponent::set_detection_radius(float radius) {
  detection_radius_ = radius;
}

float InteractionComponent::get_detection_radius() const {
  return detection_radius_;
}

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------

bool InteractionComponent::has_focus() const {
  return focused_node_ != nullptr;
}

godot::Node *InteractionComponent::get_focused_node() const {
  return focused_node_;
}

} // namespace karakuri
