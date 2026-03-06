#include "mystery_object.h"

#include "../core/services/item_service.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MysteryObject::MysteryObject() {}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void MysteryObject::_bind_methods() {
  // Properties
  ClassDB::bind_method(D_METHOD("set_scenario_id", "id"),
                       &MysteryObject::set_scenario_id);
  ClassDB::bind_method(D_METHOD("get_scenario_id"),
                       &MysteryObject::get_scenario_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "scenario_id"), "set_scenario_id",
               "get_scenario_id");

  ClassDB::bind_method(D_METHOD("set_interaction_label_prop", "label"),
                       &MysteryObject::set_interaction_label_prop);
  ClassDB::bind_method(D_METHOD("get_interaction_label_prop"),
                       &MysteryObject::get_interaction_label_prop);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "interaction_label"),
               "set_interaction_label_prop", "get_interaction_label_prop");

  // GDScript からも on_interact() を直接呼べるよう公開する
  ClassDB::bind_method(D_METHOD("on_interact"), &MysteryObject::on_interact);
  ClassDB::bind_method(D_METHOD("get_interaction_label"),
                       &MysteryObject::get_interaction_label);

  ClassDB::bind_method(D_METHOD("set_item_id", "id"),
                       &MysteryObject::set_item_id);
  ClassDB::bind_method(D_METHOD("get_item_id"), &MysteryObject::get_item_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "item_id"), "set_item_id",
               "get_item_id");

  // Signals
  ADD_SIGNAL(
      MethodInfo("interacted", PropertyInfo(Variant::STRING, "scenario_id")));
  ADD_SIGNAL(
      MethodInfo("item_collected", PropertyInfo(Variant::STRING, "item_id")));
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void MysteryObject::_ready() {
  // "interactable" グループに登録することで
  // InteractionComponent のスキャン対象になる
  add_to_group("interactable");
}

// ---------------------------------------------------------------------------
// IInteractable 実装
// ---------------------------------------------------------------------------

void MysteryObject::on_interact() {
  // 1. item_id が設定されていれば ItemService に追加し発火
  if (!item_id_.is_empty()) {
    auto *svc = karakuri::ItemService::get_singleton();
    if (svc) {
      bool added = svc->add_item(item_id_);
      UtilityFunctions::print(
          String("[MysteryObject] item ") + item_id_ +
          (added ? String(" collected.") : String(" already owned, skipped.")));
      if (added) {
        emit_signal("item_collected", item_id_);
      }
    }
  }

  // 2. シナリオ ID をシグナルに乗せて発火。
  //    具体的なシナリオ起動ロジックはシーン側 (GDScript) が担う。
  emit_signal("interacted", scenario_id_);

  UtilityFunctions::print(String("[MysteryObject] interacted: scenario_id=") +
                          scenario_id_);
}

godot::String MysteryObject::get_interaction_label() const {
  return interaction_label_;
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void MysteryObject::set_scenario_id(const godot::String &id) {
  scenario_id_ = id;
}

godot::String MysteryObject::get_scenario_id() const { return scenario_id_; }

void MysteryObject::set_interaction_label_prop(const godot::String &label) {
  interaction_label_ = label;
}

godot::String MysteryObject::get_interaction_label_prop() const {
  return interaction_label_;
}

void MysteryObject::set_item_id(const godot::String &id) { item_id_ = id; }
godot::String MysteryObject::get_item_id() const { return item_id_; }

} // namespace mystery
