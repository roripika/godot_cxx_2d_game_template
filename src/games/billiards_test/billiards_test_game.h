#ifndef KARAKURI_GAMES_BILLIARDS_TEST_BILLIARDS_TEST_GAME_H
#define KARAKURI_GAMES_BILLIARDS_TEST_BILLIARDS_TEST_GAME_H

#include <godot_cpp/classes/node.hpp>

namespace karakuri::games::billiards_test {

// Entry point for the billiards_test Kernel Fitness Test module.
// Place this Node in your test scene. Its _ready() call registers
// all four billiards task factories with ActionRegistry so that
// ScenarioRunner can instantiate them from YAML scenario files.
class BilliardsTestGame : public godot::Node {
    GDCLASS(BilliardsTestGame, godot::Node)

protected:
    static void _bind_methods();

public:
    BilliardsTestGame() = default;
    ~BilliardsTestGame() override = default;

    void _ready() override;
};

} // namespace karakuri::games::billiards_test

#endif // KARAKURI_GAMES_BILLIARDS_TEST_BILLIARDS_TEST_GAME_H
