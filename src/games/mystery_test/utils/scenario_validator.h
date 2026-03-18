#ifndef KARAKURI_GAMES_MYSTERY_TEST_SCENARIO_VALIDATOR_H
#define KARAKURI_GAMES_MYSTERY_TEST_SCENARIO_VALIDATOR_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <godot_cpp/classes/ref_counted.hpp>

namespace karakuri {

class ScenarioValidator : public godot::RefCounted {
    GDCLASS(ScenarioValidator, godot::RefCounted)

protected:
    static void _bind_methods();

public:
    ScenarioValidator() = default;
    ~ScenarioValidator() override = default;

    /**
     * @brief シナリオデータを静的に検証する。
     * @param scenario_data YAMLからパースされた Dictionary
     * @return Array エラーメッセージ（文字列）の配列。空なら正常。
     */
    godot::Array validate(const godot::Dictionary &scenario_data);

    /**
     * @brief ファイルパスからシナリオを読み込んで検証する。
     * @param path ファイル名 (res:// 等)
     */
    godot::Array validate_file(const godot::String &path);

private:
    void validate_block(const godot::Array &block, const godot::Array &valid_labels, godot::Array &errors);
    void validate_payload(const godot::String &action, const godot::Dictionary &payload, godot::Array &errors);
    void validate_world_state_key(const godot::String &key, godot::Array &errors);
    
    godot::Dictionary get_action_schemas();
};

} // namespace karakuri

#endif // KARAKURI_GAMES_MYSTERY_TEST_SCENARIO_VALIDATOR_H
