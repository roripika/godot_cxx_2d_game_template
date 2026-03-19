#pragma once
#include <godot_cpp/classes/node.hpp>

namespace karakuri::games::roguelike_test {

// Entry point for the roguelike_test Kernel Fitness Test module.
// Place this Node in your test scene. Its _ready() call registers
// all seven roguelike task factories with ActionRegistry so that
// ScenarioRunner can instantiate them from YAML scenario files.
class RoguelikeTestGame : public godot::Node {
    GDCLASS(RoguelikeTestGame, godot::Node)

protected:
    static void _bind_methods();

public:
    RoguelikeTestGame() = default;
    ~RoguelikeTestGame() override = default;

    void _ready() override;
};

} // namespace karakuri::games::roguelike_test
