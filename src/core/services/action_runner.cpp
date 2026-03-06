#include "services/action_runner.h"

#include <godot_cpp/classes/callback_tweener.hpp>
#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/interval_tweener.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

// ---------------------------------------------------------------------------
// _bind_methods
// ---------------------------------------------------------------------------
void ActionRunner::_bind_methods() {
  ClassDB::bind_method(D_METHOD("play_effect", "type", "params"),
                       &ActionRunner::play_effect);
  ClassDB::bind_method(D_METHOD("play_sequence", "steps"),
                       &ActionRunner::play_sequence);
  ClassDB::bind_method(D_METHOD("stop_all"), &ActionRunner::stop_all);
  ClassDB::bind_method(D_METHOD("get_sequence_running"),
                       &ActionRunner::get_sequence_running);

  ClassDB::bind_method(D_METHOD("set_camera_path", "path"),
                       &ActionRunner::set_camera_path);
  ClassDB::bind_method(D_METHOD("get_camera_path"),
                       &ActionRunner::get_camera_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path",
                             PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"),
               "set_camera_path", "get_camera_path");

  ClassDB::bind_method(D_METHOD("set_overlay_path", "path"),
                       &ActionRunner::set_overlay_path);
  ClassDB::bind_method(D_METHOD("get_overlay_path"),
                       &ActionRunner::get_overlay_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "overlay_path",
                             PROPERTY_HINT_NODE_PATH_VALID_TYPES, "ColorRect"),
               "set_overlay_path", "get_overlay_path");

  ADD_SIGNAL(MethodInfo("effect_sequence_finished"));
}

// ---------------------------------------------------------------------------
// ノード解決ヘルパー
// ---------------------------------------------------------------------------
Node *ActionRunner::resolve(const NodePath &path) const {
  if (path.is_empty())
    return nullptr;
  return get_node_or_null(path);
}

// ---------------------------------------------------------------------------
// _process — シーケンス進行
// ---------------------------------------------------------------------------
void ActionRunner::_process(double delta) {
  if (!sequence_running_)
    return;
  if (step_wait_ > 0.0) {
    step_wait_ -= delta;
    if (step_wait_ > 0.0)
      return;
  }
  run_next_step();
}

// ---------------------------------------------------------------------------
// 単発演出
// ---------------------------------------------------------------------------
void ActionRunner::play_effect(const String &type, const Dictionary &params) {
  if (type == "shake") {
    apply_shake(params);
  } else if (type == "zoom") {
    apply_zoom(params);
  } else if (type == "fade") {
    apply_fade(params);
  } else if (type == "flash") {
    apply_flash(params);
  } else if (type == "slow_motion") {
    apply_slow_motion(params);
  } else {
    UtilityFunctions::push_warning("ActionRunner: unknown effect type: " + type);
  }
}

// ---------------------------------------------------------------------------
// シーケンス実行
// ---------------------------------------------------------------------------
void ActionRunner::play_sequence(const Array &steps) {
  if (steps.is_empty()) {
    emit_signal("effect_sequence_finished");
    return;
  }
  sequence_steps_ = steps;
  sequence_index_ = 0;
  step_wait_ = 0.0;
  sequence_running_ = true;
  set_process(true);
  run_next_step();
}

void ActionRunner::run_next_step() {
  if (sequence_index_ >= sequence_steps_.size()) {
    sequence_running_ = false;
    set_process(false);
    emit_signal("effect_sequence_finished");
    return;
  }

  Variant raw = sequence_steps_[sequence_index_++];
  if (raw.get_type() != Variant::DICTIONARY) {
    run_next_step(); // スキップして次へ
    return;
  }

  Dictionary step = raw;
  String type = step.has("type") ? String(step["type"]) : "";
  Dictionary params = step.has("params") ? Dictionary(step["params"]) : Dictionary();
  double duration = step.has("duration") ? double(step["duration"]) : 0.0;

  play_effect(type, params);

  // duration が指定されていれば待機してから次へ、そうでなければ即次ステップ
  if (duration > 0.0) {
    step_wait_ = duration;
  } else {
    // 演出パラメータの duration をフォールバックに使う
    double param_dur = params.has("duration") ? double(params["duration"]) : 0.0;
    if (param_dur > 0.0) {
      step_wait_ = param_dur;
    } else {
      run_next_step();
    }
  }
}

void ActionRunner::stop_all() {
  sequence_running_ = false;
  set_process(false);
  step_wait_ = 0.0;

  // slow_motion リセット
  Engine *engine = Engine::get_singleton();
  if (engine)
    engine->set_time_scale(1.0);

  // overlay リセット
  Node *overlay = resolve(overlay_path_);
  CanvasItem *ci = Object::cast_to<CanvasItem>(overlay);
  if (ci)
    ci->set_modulate(Color(1, 1, 1, 0));
}

// ---------------------------------------------------------------------------
// 演出実装
// ---------------------------------------------------------------------------

void ActionRunner::apply_shake(const Dictionary &params) {
  Node *cam_node = resolve(camera_path_);
  Camera2D *cam = Object::cast_to<Camera2D>(cam_node);
  if (!cam) {
    UtilityFunctions::print("[ActionRunner] shake: no Camera2D set.");
    return;
  }

  float intensity = params.has("intensity") ? float(params["intensity"]) : 8.0f;
  float duration = params.has("duration") ? float(params["duration"]) : 0.3f;
  int steps = params.has("steps") ? int(params["steps"]) : 6;

  Ref<Tween> tween = get_tree()->create_tween();
  tween->set_loops(0);
  float step_dur = duration / float(steps * 2);
  Vector2 origin = cam->get_offset();

  for (int i = 0; i < steps; ++i) {
    float sign = (i % 2 == 0) ? 1.0f : -1.0f;
    float decay = 1.0f - float(i) / float(steps);
    Vector2 offset = Vector2(intensity * sign * decay, intensity * 0.5f * sign * decay);
    tween->tween_property(cam, "offset", origin + offset, step_dur);
  }
  tween->tween_property(cam, "offset", origin, step_dur);
}

void ActionRunner::apply_zoom(const Dictionary &params) {
  Node *cam_node = resolve(camera_path_);
  Camera2D *cam = Object::cast_to<Camera2D>(cam_node);
  if (!cam) {
    UtilityFunctions::print("[ActionRunner] zoom: no Camera2D set.");
    return;
  }

  float target = params.has("value") ? float(params["value"]) : 1.2f;
  float duration = params.has("duration") ? float(params["duration"]) : 0.25f;
  float back_duration = params.has("back_duration") ? float(params["back_duration"]) : 0.25f;

  Ref<Tween> tween = get_tree()->create_tween();
  tween->tween_property(cam, "zoom", Vector2(target, target), duration)
      ->set_trans(Tween::TRANS_CUBIC)
      ->set_ease(Tween::EASE_OUT);
  tween->tween_property(cam, "zoom", Vector2(1.0f, 1.0f), back_duration)
      ->set_trans(Tween::TRANS_CUBIC)
      ->set_ease(Tween::EASE_IN);
}

void ActionRunner::apply_fade(const Dictionary &params) {
  Node *overlay_node = resolve(overlay_path_);
  CanvasItem *ci = Object::cast_to<CanvasItem>(overlay_node);
  if (!ci) {
    UtilityFunctions::print("[ActionRunner] fade: no overlay ColorRect set.");
    return;
  }

  float target_alpha = params.has("alpha") ? float(params["alpha"]) : 1.0f;
  float duration = params.has("duration") ? float(params["duration"]) : 0.5f;
  Color col = ci->get_modulate();
  col.a = target_alpha;

  Ref<Tween> tween = get_tree()->create_tween();
  tween->tween_property(ci, "modulate:a", target_alpha, duration)
      ->set_trans(Tween::TRANS_LINEAR);
}

void ActionRunner::apply_flash(const Dictionary &params) {
  Node *overlay_node = resolve(overlay_path_);
  CanvasItem *ci = Object::cast_to<CanvasItem>(overlay_node);
  if (!ci) {
    UtilityFunctions::print("[ActionRunner] flash: no overlay ColorRect set.");
    return;
  }

  float r = params.has("r") ? float(params["r"]) : 1.0f;
  float g = params.has("g") ? float(params["g"]) : 1.0f;
  float b = params.has("b") ? float(params["b"]) : 1.0f;
  float peak_alpha = params.has("alpha") ? float(params["alpha"]) : 0.7f;
  float duration = params.has("duration") ? float(params["duration"]) : 0.3f;

  // フラッシュ色を ColorRect のカラーに反映（modulate は白で）
  ColorRect *cr = Object::cast_to<ColorRect>(overlay_node);
  if (cr) {
    cr->set_color(Color(r, g, b, 1.0f));
  }

  Ref<Tween> tween = get_tree()->create_tween();
  tween->tween_property(ci, "modulate:a", peak_alpha, duration * 0.2)
      ->set_trans(Tween::TRANS_EXPO)
      ->set_ease(Tween::EASE_OUT);
  tween->tween_property(ci, "modulate:a", 0.0f, duration * 0.8)
      ->set_trans(Tween::TRANS_EXPO)
      ->set_ease(Tween::EASE_IN);
}

void ActionRunner::apply_slow_motion(const Dictionary &params) {
  Engine *engine = Engine::get_singleton();
  if (!engine)
    return;

  float scale = params.has("scale") ? float(params["scale"]) : 0.3f;
  float duration = params.has("duration") ? float(params["duration"]) : 0.5f;
  float recover = params.has("recover") ? float(params["recover"]) : 0.5f;

  engine->set_time_scale(scale);

  // duration 秒後に 1.0 に戻す（Tween は物理時間ではなく TimeScale に依存するので
  // create_tween にPROCESS_PHYSICSを使う）
  Ref<Tween> tween = get_tree()->create_tween();
  tween->set_process_mode(Tween::TWEEN_PROCESS_PHYSICS);
  // time_scale が変わっているので実際の経過時間で割る
  tween->tween_interval(duration / scale);
  tween->tween_callback(Callable(engine, "set_time_scale").bind(1.0));
  (void)recover;
}

// ---------------------------------------------------------------------------
// プロパティ
// ---------------------------------------------------------------------------
void ActionRunner::set_camera_path(const NodePath &path) { camera_path_ = path; }
NodePath ActionRunner::get_camera_path() const { return camera_path_; }

void ActionRunner::set_overlay_path(const NodePath &path) { overlay_path_ = path; }
NodePath ActionRunner::get_overlay_path() const { return overlay_path_; }

} // namespace karakuri
