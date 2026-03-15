#ifndef KARAKURI_TASK_SPEC_H
#define KARAKURI_TASK_SPEC_H

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief Typed Scenario IR wrapper for task setup.
 * Raw Dictionaries are parsed into TaskSpec at load time to prevent
 * weak validation and typeless data passing.
 */
struct TaskSpec {
    godot::String action;
    const godot::Dictionary payload;

    TaskSpec() : action(""), payload() {}
    TaskSpec(const godot::String& a, const godot::Dictionary& p) : action(a), payload(p) {}
};

} // namespace karakuri

#endif // KARAKURI_TASK_SPEC_H
