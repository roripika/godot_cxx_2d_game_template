#include "setup_rhythm_round_task.h"

#include "core/kernel_clock.h"
#include "core/world_state.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kMinNotes = 3;
constexpr int kMaxNotes = 5;
constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

godot::String note_time_key(int index) {
    return godot::String("chart:note_") + godot::String::num_int64(index) + ":time_ms";
}

godot::String tap_time_key(int index) {
    return godot::String("chart:tap_") + godot::String::num_int64(index) + ":time_ms";
}

bool is_numeric_variant(const godot::Variant &value) {
    return value.get_type() == godot::Variant::INT || value.get_type() == godot::Variant::FLOAT;
}

} // namespace

void SetupRhythmRoundTask::_bind_methods() {}

godot::Error SetupRhythmRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("notes") || !spec.payload.has("taps")) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] 'notes' and 'taps' are required.");
        return godot::ERR_INVALID_DATA;
    }

    notes_ = godot::Array(spec.payload["notes"]);
    taps_ = godot::Array(spec.payload["taps"]);
    if (notes_.size() < kMinNotes || notes_.size() > kMaxNotes) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] note count must be in [3, 5].");
        return godot::ERR_INVALID_DATA;
    }
    if (taps_.size() != notes_.size()) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] 'taps' length must match 'notes'.");
        return godot::ERR_INVALID_DATA;
    }

    int previous_note_time = -1;
    for (int i = 0; i < notes_.size(); i++) {
        if (!is_numeric_variant(notes_[i])) {
            godot::UtilityFunctions::push_error(
                "[SetupRhythmRoundTask] note times must be numeric.");
            return godot::ERR_INVALID_DATA;
        }
        const int note_time = static_cast<int>(notes_[i]);
        if (note_time <= 0) {
            godot::UtilityFunctions::push_error(
                "[SetupRhythmRoundTask] note times must be > 0.");
            return godot::ERR_INVALID_DATA;
        }
        if (previous_note_time >= 0 && note_time <= previous_note_time) {
            godot::UtilityFunctions::push_error(
                "[SetupRhythmRoundTask] note times must be strictly ascending.");
            return godot::ERR_INVALID_DATA;
        }
        previous_note_time = note_time;

        if (!is_numeric_variant(taps_[i])) {
            godot::UtilityFunctions::push_error(
                "[SetupRhythmRoundTask] tap times must be numeric.");
            return godot::ERR_INVALID_DATA;
        }
        const int tap_time = static_cast<int>(taps_[i]);
        if (tap_time < -1) {
            godot::UtilityFunctions::push_error(
                "[SetupRhythmRoundTask] tap times must be >= -1.");
            return godot::ERR_INVALID_DATA;
        }
    }

    if (spec.payload.has("advance_ms")) {
        advance_ms_ = static_cast<int>(spec.payload["advance_ms"]);
    }
    if (spec.payload.has("perfect_window_ms")) {
        perfect_window_ms_ = static_cast<int>(spec.payload["perfect_window_ms"]);
    }
    if (spec.payload.has("good_window_ms")) {
        good_window_ms_ = static_cast<int>(spec.payload["good_window_ms"]);
    }
    if (spec.payload.has("clear_hit_count")) {
        clear_hit_count_ = static_cast<int>(spec.payload["clear_hit_count"]);
    }
    if (spec.payload.has("max_miss_count")) {
        max_miss_count_ = static_cast<int>(spec.payload["max_miss_count"]);
    }

    if (advance_ms_ <= 0 || perfect_window_ms_ <= 0 || good_window_ms_ < perfect_window_ms_) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] invalid timing parameters.");
        return godot::ERR_INVALID_DATA;
    }
    if (clear_hit_count_ <= 0 || clear_hit_count_ > notes_.size()) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] clear_hit_count must be in [1, note_count].");
        return godot::ERR_INVALID_DATA;
    }
    if (max_miss_count_ < 0) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] max_miss_count must be >= 0.");
        return godot::ERR_INVALID_DATA;
    }

    return godot::OK;
}

TaskResult SetupRhythmRoundTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    auto *clock = karakuri::KernelClock::get_singleton();
    if (!ws || !clock) {
        godot::UtilityFunctions::push_error(
            "[SetupRhythmRoundTask] WorldState or KernelClock singleton is null.");
        return TaskResult::Failed;
    }

    clock->reset();

    ws->set_state("rhythm_test", kScopeSession, "chart:index", 0);
    ws->set_state("rhythm_test", kScopeSession, "chart:note_count", notes_.size());
    ws->set_state("rhythm_test", kScopeSession, "judge:last_result", godot::String("none"));
    ws->set_state("rhythm_test", kScopeSession, "judge:perfect_count", 0);
    ws->set_state("rhythm_test", kScopeSession, "judge:good_count", 0);
    ws->set_state("rhythm_test", kScopeSession, "judge:miss_count", 0);
    ws->set_state("rhythm_test", kScopeSession, "tap:last_time_ms", -1);
    ws->set_state("rhythm_test", kScopeSession, "round:result", godot::String(""));
    ws->set_state("rhythm_test", kScopeSession, "round:status", godot::String("playing"));
    ws->set_state("rhythm_test", kScopeSession, "clock:now_ms", 0);

    ws->set_state("rhythm_test", kScopeSession, "config:advance_ms", advance_ms_);
    ws->set_state("rhythm_test", kScopeSession, "config:perfect_window_ms", perfect_window_ms_);
    ws->set_state("rhythm_test", kScopeSession, "config:good_window_ms", good_window_ms_);
    ws->set_state("rhythm_test", kScopeSession, "config:clear_hit_count", clear_hit_count_);
    ws->set_state("rhythm_test", kScopeSession, "config:max_miss_count", max_miss_count_);

    for (int i = 0; i < kMaxNotes; i++) {
        const int note_time = i < notes_.size() ? static_cast<int>(notes_[i]) : -1;
        const int tap_time = i < taps_.size() ? static_cast<int>(taps_[i]) : -1;
        ws->set_state("rhythm_test", kScopeSession, note_time_key(i), note_time);
        ws->set_state("rhythm_test", kScopeSession, tap_time_key(i), tap_time);
    }

    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test