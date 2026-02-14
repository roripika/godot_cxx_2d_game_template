#ifndef KARAKURI_YAML_LITE_H
#define KARAKURI_YAML_LITE_H

/**
 * @file karakuri_yaml_lite.h
 * @brief Basic Game Karakuri: YAML Lite parser (subset) for planner-authored scenario files.
 *
 * This is intentionally a small YAML subset parser suitable for game scenario configuration:
 * - Maps: `key: value` and `key:` (nested block)
 * - Sequences: `- value`, `- key: value`, `- key:` (nested block)
 * - Scalars: string, int, float, bool
 *
 * Not supported:
 * - anchors/aliases, multi-line scalars, complex quoting rules, tabs for indentation, etc.
 *
 * The goal is to keep planner-authored YAML readable without pulling a full YAML dependency
 * into the GDExtension build yet.
 */

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

/**
 * @brief Minimal YAML parser for a restricted subset used by scenarios.
 */
class KarakuriYamlLite final {
public:
  /**
   * @brief Parse YAML text into a Godot Variant tree (Dictionary/Array/scalars).
   *
   * @param yaml_text YAML source text.
   * @param out_root Parsed root. Usually Dictionary.
   * @param out_error Filled when parsing fails.
   * @return true on success.
   */
  static bool parse(const godot::String &yaml_text, godot::Variant &out_root,
                    godot::String &out_error);
};

} // namespace karakuri

#endif // KARAKURI_YAML_LITE_H

