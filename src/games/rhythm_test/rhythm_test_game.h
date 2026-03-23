#pragma once

#include <godot_cpp/classes/node.hpp>

namespace karakuri::games::rhythm_test {

class RhythmTestGame : public godot::Node {
    GDCLASS(RhythmTestGame, godot::Node)

protected:
    static void _bind_methods();

public:
    RhythmTestGame() = default;
    ~RhythmTestGame() override = default;

    void _ready() override;
};

} // namespace karakuri::games::rhythm_test