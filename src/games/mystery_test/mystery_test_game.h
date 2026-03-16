#ifndef KARAKURI_MYSTERY_TEST_GAME_H
#define KARAKURI_MYSTERY_TEST_GAME_H

#include <godot_cpp/classes/node.hpp>

namespace karakuri {

class MysteryTestGame : public godot::Node {
    GDCLASS(MysteryTestGame, godot::Node)

protected:
    static void _bind_methods();

public:
    MysteryTestGame() = default;
    ~MysteryTestGame() override = default;

    void _ready() override;
};

} // namespace karakuri

#endif // KARAKURI_MYSTERY_TEST_GAME_H
