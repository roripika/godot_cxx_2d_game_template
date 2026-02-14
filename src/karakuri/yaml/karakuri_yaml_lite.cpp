#include "karakuri_yaml_lite.h"

/**
 * @file karakuri_yaml_lite.cpp
 * @brief See karakuri_yaml_lite.h
 */

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {
namespace {

static int count_indent_spaces(const String &line, String &err) {
  int i = 0;
  while (i < line.length()) {
    const char32_t c = line[i];
    if (c == U' ') {
      i++;
      continue;
    }
    if (c == U'\t') {
      err = "Tabs are not supported for indentation.";
      return -1;
    }
    break;
  }
  return i;
}

static String strip_comment(const String &line) {
  // Very small rule: everything after '#' is treated as a comment if not quoted.
  // This is sufficient for our scenario YAML where values are simple.
  const int idx = line.find("#");
  if (idx < 0) {
    return line;
  }
  return line.substr(0, idx);
}

static bool is_blank(const String &line) {
  return line.strip_edges().is_empty();
}

static Variant parse_scalar(const String &raw) {
  const String s = raw.strip_edges();
  if (s.is_empty()) {
    return Variant();
  }

  // Quoted string (very small subset, no escapes).
  if ((s.begins_with("\"") && s.ends_with("\"") && s.length() >= 2) ||
      (s.begins_with("'") && s.ends_with("'") && s.length() >= 2)) {
    return s.substr(1, s.length() - 2);
  }

  const String lower = s.to_lower();
  if (lower == "true") {
    return true;
  }
  if (lower == "false") {
    return false;
  }

  // Int.
  bool is_int = true;
  int start = 0;
  if (s.begins_with("-")) {
    start = 1;
  }
  for (int i = start; i < s.length(); i++) {
    const char32_t c = s[i];
    if (c < U'0' || c > U'9') {
      is_int = false;
      break;
    }
  }
  if (is_int && start < s.length()) {
    return s.to_int();
  }

  // Float (simple, one dot).
  bool is_float = false;
  int dot_count = 0;
  bool ok = true;
  start = 0;
  if (s.begins_with("-")) {
    start = 1;
  }
  for (int i = start; i < s.length(); i++) {
    const char32_t c = s[i];
    if (c == U'.') {
      dot_count++;
      continue;
    }
    if (c < U'0' || c > U'9') {
      ok = false;
      break;
    }
  }
  if (ok && dot_count == 1 && start < s.length()) {
    is_float = true;
  }
  if (is_float) {
    return s.to_float();
  }

  return s;
}

static bool split_key_value(const String &line_no_indent, String &out_key,
                            String &out_value, String &err) {
  const int colon = line_no_indent.find(":");
  if (colon < 0) {
    err = "Expected ':' in mapping line.";
    return false;
  }

  out_key = line_no_indent.substr(0, colon).strip_edges();
  if (out_key.is_empty()) {
    err = "Empty key in mapping line.";
    return false;
  }
  out_value = line_no_indent.substr(colon + 1).strip_edges();
  return true;
}

static Variant parse_block(const PackedStringArray &lines, int &io_index,
                           int indent, String &err);

static Variant parse_sequence(const PackedStringArray &lines, int &io_index,
                              int indent, String &err) {
  Array arr;
  while (io_index < lines.size()) {
    String line = strip_comment(lines[io_index]);
    if (is_blank(line)) {
      io_index++;
      continue;
    }

    const int cur_indent = count_indent_spaces(line, err);
    if (cur_indent < 0) {
      return Variant();
    }
    if (cur_indent < indent) {
      break;
    }
    if (cur_indent != indent) {
      err = "Unexpected indentation in sequence.";
      return Variant();
    }

    String trimmed = line.substr(cur_indent).strip_edges();
    if (!trimmed.begins_with("-")) {
      // End of this sequence block.
      break;
    }
    trimmed = trimmed.substr(1).strip_edges(); // after '-'

    if (trimmed.is_empty()) {
      io_index++;
      const Variant child = parse_block(lines, io_index, indent + 2, err);
      if (!err.is_empty()) {
        return Variant();
      }
      arr.append(child);
      continue;
    }

    // Inline map item: "- key: value" or "- key:".
    if (trimmed.find(":") >= 0) {
      String key, value;
      if (!split_key_value(trimmed, key, value, err)) {
        return Variant();
      }
      Dictionary d;
      if (value.is_empty()) {
        io_index++;
        const Variant child = parse_block(lines, io_index, indent + 4, err);
        if (!err.is_empty()) {
          return Variant();
        }
        d[key] = child;
      } else {
        d[key] = parse_scalar(value);
        io_index++;
      }
      arr.append(d);
      continue;
    }

    // Scalar item.
    arr.append(parse_scalar(trimmed));
    io_index++;
  }

  return arr;
}

static Variant parse_mapping(const PackedStringArray &lines, int &io_index,
                             int indent, String &err) {
  Dictionary d;
  while (io_index < lines.size()) {
    String line = strip_comment(lines[io_index]);
    if (is_blank(line)) {
      io_index++;
      continue;
    }

    const int cur_indent = count_indent_spaces(line, err);
    if (cur_indent < 0) {
      return Variant();
    }
    if (cur_indent < indent) {
      break;
    }
    if (cur_indent != indent) {
      err = "Unexpected indentation in mapping.";
      return Variant();
    }

    const String no_indent = line.substr(cur_indent);
    String key, value;
    if (!split_key_value(no_indent, key, value, err)) {
      return Variant();
    }

    if (value.is_empty()) {
      io_index++;
      const Variant child = parse_block(lines, io_index, indent + 2, err);
      if (!err.is_empty()) {
        return Variant();
      }
      d[key] = child;
    } else {
      d[key] = parse_scalar(value);
      io_index++;
    }
  }
  return d;
}

static Variant parse_block(const PackedStringArray &lines, int &io_index,
                           int indent, String &err) {
  // Skip blanks.
  while (io_index < lines.size()) {
    String line = strip_comment(lines[io_index]);
    if (!is_blank(line)) {
      break;
    }
    io_index++;
  }
  if (io_index >= lines.size()) {
    return Dictionary();
  }

  String line = strip_comment(lines[io_index]);
  const int cur_indent = count_indent_spaces(line, err);
  if (cur_indent < 0) {
    return Variant();
  }
  if (cur_indent < indent) {
    // Empty block.
    return Dictionary();
  }
  if (cur_indent != indent) {
    err = "Unexpected indentation at block start.";
    return Variant();
  }

  const String trimmed = line.substr(cur_indent).strip_edges();
  if (trimmed.begins_with("-")) {
    return parse_sequence(lines, io_index, indent, err);
  }
  return parse_mapping(lines, io_index, indent, err);
}

} // namespace

bool KarakuriYamlLite::parse(const String &yaml_text, Variant &out_root,
                             String &out_error) {
  out_error = "";
  PackedStringArray lines = yaml_text.split("\n", false);
  int idx = 0;
  Variant v = parse_block(lines, idx, 0, out_error);
  if (!out_error.is_empty()) {
    return false;
  }
  out_root = v;
  return true;
}

} // namespace karakuri

