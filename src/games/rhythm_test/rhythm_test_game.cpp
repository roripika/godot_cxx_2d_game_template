#include "rhythm_test_game.h"

#include "core/action_registry.h"
#include "tasks/setup_rhythm_round_task.h"
#include "tasks/load_fake_tap_task.h"
#include "tasks/advance_rhythm_clock_task.h"
#include "tasks/judge_rhythm_note_task.h"
#include "tasks/resolve_rhythm_progress_task.h"
#include "tasks/evaluate_rhythm_round_task.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::rhythm_test {

void RhythmTestGame::_bind_methods() {}

void RhythmTestGame::_ready() {
    godot::UtilityFunctions::print("[RhythmTestGame] Bootstrapping...");

    if (auto reg = karakuri::ActionRegistry::get_singleton()) {
        reg->register_action_class<SetupRhythmRoundTask>("setup_rhythm_round");
        reg->register_action_class<LoadFakeTapTask>("load_fake_tap");
        reg->register_action_class<AdvanceRhythmClockTask>("advance_rhythm_clock");
        reg->register_action_class<JudgeRhythmNoteTask>("judge_rhythm_note");
        reg->register_action_class<ResolveRhythmProgressTask>("resolve_rhythm_progress");
        reg->register_action_class<EvaluateRhythmRoundTask>("evaluate_rhythm_round");

        godot::UtilityFunctions::print("[RhythmTestGame] Registered 6 rhythm test tasks.");
    }
}

} // namespace karakuri::games::rhythm_test