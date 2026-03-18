#include "scenario_validator.h"
#include "core/action_registry.h"
#include "core/yaml/yaml_lite.h"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void ScenarioValidator::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("validate", "scenario_data"), &ScenarioValidator::validate);
    godot::ClassDB::bind_method(godot::D_METHOD("validate_file", "path"), &ScenarioValidator::validate_file);
}

godot::Array ScenarioValidator::validate(const godot::Dictionary &scenario_data) {
    godot::Array errors;
    
    if (!scenario_data.has("scenes")) {
        errors.append("Missing 'scenes' root key.");
        return errors;
    }

    godot::Dictionary scenes = scenario_data["scenes"];
    godot::Array valid_labels = scenes.keys();

    for (int i = 0; i < valid_labels.size(); ++i) {
        godot::String label = valid_labels[i];
        godot::Dictionary scene_def = scenes[label];

        if (scene_def.has("on_enter")) {
            validate_block(scene_def["on_enter"], valid_labels, errors);
        }
    }

    return errors;
}

godot::Array ScenarioValidator::validate_file(const godot::String &path) {
    godot::Array errors;
    godot::Ref<godot::FileAccess> f = godot::FileAccess::open(path, godot::FileAccess::READ);
    if (f.is_null()) {
        errors.append("Could not open file: " + path);
        return errors;
    }

    godot::Variant root;
    godot::String err;
    if (!YamlLite::parse(f->get_as_text(), root, err)) {
        errors.append("YAML Parse Error: " + err);
        return errors;
    }

    if (root.get_type() != godot::Variant::DICTIONARY) {
        errors.append("Scenario root must be a Dictionary.");
        return errors;
    }

    return validate(root);
}

void ScenarioValidator::validate_block(const godot::Array &block, const godot::Array &valid_labels, godot::Array &errors) {
    auto registry = ActionRegistry::get_singleton();
    if (!registry) return;

    for (int i = 0; i < block.size(); ++i) {
        godot::Dictionary task = block[i];
        if (!task.has("action")) {
            errors.append("Task missing 'action' key.");
            continue;
        }

        godot::String action = task["action"];
        if (!registry->has_action(action)) {
            errors.append("Unknown action: " + action);
        }

        godot::Dictionary payload = task.get("payload", godot::Dictionary());
        
        // Payload のキー検証
        validate_payload(action, payload, errors);

        // 遷移先の検証 (if_true, if_false, next_scene などの慣習的キー)
        godot::Array jump_keys;
        jump_keys.append("if_true");
        jump_keys.append("if_false");
        jump_keys.append("next_scene");

        for (int k = 0; k < jump_keys.size(); ++k) {
            godot::String key = jump_keys[k];
            if (payload.has(key)) {
                godot::String target = payload[key];
                bool found = false;
                for (int j = 0; j < valid_labels.size(); ++j) {
                    if (godot::String(valid_labels[j]) == target) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    errors.append("Invalid jump target '" + target + "' in action '" + action + "'");
                }
            }
        }

        // 再帰的なブロックの検証 (parallel, then 等)
        if (payload.has("tasks")) {
            validate_block(payload["tasks"], valid_labels, errors);
        }
        if (payload.has("then")) {
            validate_block(payload["then"], valid_labels, errors);
        }
    }
}

void ScenarioValidator::validate_payload(const godot::String &action, const godot::Dictionary &payload, godot::Array &errors) {
    godot::Dictionary schemas = get_action_schemas();
    if (!schemas.has(action)) return;

    godot::Dictionary schema = schemas[action];
    godot::Array required = schema.get("required", godot::Array());
    godot::Array optional = schema.get("optional", godot::Array());

    // 必須キーのチェック
    for (int i = 0; i < required.size(); ++i) {
        godot::String key = required[i];
        if (!payload.has(key)) {
            errors.append("Action '" + action + "' missing required payload key: " + key);
        }
    }

    // 未知のキーのチェック
    godot::Array payload_keys = payload.keys();
    for (int i = 0; i < payload_keys.size(); ++i) {
        godot::String key = payload_keys[i];
        if (key == "action") continue; // 共通キー

        bool is_valid = false;
        for (int j = 0; j < required.size(); ++j) {
            if (godot::String(required[j]) == key) { is_valid = true; break; }
        }
        if (!is_valid) {
            for (int j = 0; j < optional.size(); ++j) {
                if (godot::String(optional[j]) == key) { is_valid = true; break; }
            }
        }

        if (!is_valid) {
            errors.append("Action '" + action + "' has unknown payload key: " + key);
        }

        // WorldState Key の検証
        if (key == "id" || key == "evidence") {
            validate_world_state_key(payload[key], errors);
        }

        // Signal の検証 (whitelist チェック)
        if (key == "signal" && schema.has("approved_signals")) {
            godot::Array approved = schema["approved_signals"];
            godot::String signal_name = payload[key];
            bool found = false;
            for (int j = 0; j < approved.size(); ++j) {
                if (godot::String(approved[j]) == signal_name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                errors.append("Action '" + action + "' has unauthorized signal: " + signal_name);
            }
        }
    }
}

void ScenarioValidator::validate_world_state_key(const godot::String &key, godot::Array &errors) {
    // 形式: module:scope:key
    // 新規モジュール (billiards_test) に関しては厳格に REJECT する
    if (key.find(":") == -1) {
        errors.append("WorldState key '" + key + "' must follow 'module:scope:key' format.");
        return;
    }

    // billiards_test 関連のキーが名前空間を守っているか確認
    if (key.begins_with("billiards") && !key.begins_with("billiards_test:")) {
        errors.append("Invalid namespace for billiards task. Use 'billiards_test:scope:key'.");
    }
}

godot::Dictionary ScenarioValidator::get_action_schemas() {
    godot::Dictionary schemas;
    
    // Mystery Test Actions
    {
        godot::Dictionary d;
        godot::Array req; req.append("speaker"); req.append("text");
        d["required"] = req;
        schemas["show_dialogue"] = d;
    }
    {
        godot::Dictionary d;
        godot::Array req; req.append("id");
        d["required"] = req;
        schemas["add_evidence"] = d;
    }
    {
        godot::Dictionary d;
        godot::Array req; req.append("id"); req.append("if_true"); req.append("if_false");
        d["required"] = req;
        schemas["check_evidence"] = d;
    }
    {
        godot::Dictionary d;
        godot::Array req; req.append("all_of"); req.append("if_true"); req.append("if_false");
        d["required"] = req;
        schemas["check_condition"] = d;
    }
    {
        godot::Dictionary d;
        godot::Array req; req.append("id"); req.append("location");
        d["required"] = req;
        schemas["discover_evidence"] = d;
    }

    // Billiards Test Actions (Phase 2-C 準備)
    {
        godot::Dictionary d;
        godot::Array req; req.append("signal");
        godot::Array opt; opt.append("timeout");
        d["required"] = req;
        d["optional"] = opt;
        
        // whitelist of approved signals for Phase 2
        godot::Array approved;
        approved.append("shot_committed");
        approved.append("ball_pocketed");
        approved.append("cue_ball_pocketed");
        approved.append("balls_stopped");
        d["approved_signals"] = approved;

        schemas["wait_for_billiards_event"] = d;
        schemas["wait_for_signal"] = d; 
    }
    {
        godot::Dictionary d;
        godot::Array req; req.append("if_win"); req.append("if_lose"); req.append("if_continue");
        d["required"] = req;
        schemas["evaluate_billiards_round"] = d;
    }

    return schemas;
}

} // namespace karakuri
