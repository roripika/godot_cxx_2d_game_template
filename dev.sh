#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'EOF'
Usage: ./dev.sh <command> [args]

Commands:
  setup                 Run ./setup.sh (clone godot-cpp, generate env.sh)
  build                 Build GDExtension (defaults: macos/template_debug/arm64 on macOS)
  run   [demo]          Switch demo (optional), build, run game (CLI)
  edit  [demo]          Switch demo (optional), build, open Godot editor
  demo  <demo>          Switch main scene (no build/run)
  menu                  Set main scene to res://samples/main_menu.tscn
  status                Print current run/main_scene from project.godot
  godot-bin             Print which Godot binary path will be used

Demo values (examples):
  roguelike | platformer | mystery | sandbox | fighting | rhythm | gallery | menu

Environment variables:
  GODOT_BIN             Override Godot executable path
  PLATFORM/TARGET/ARCH  Override scons options (default is platform-specific)
  JOBS                  Override parallel build jobs
EOF
}

die() {
  echo "Error: $*" >&2
  exit 1
}

is_macos() {
  [[ "$(uname -s)" == "Darwin" ]]
}

ncpu() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif is_macos && command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu
  else
    echo 4
  fi
}

detect_godot_bin() {
  if [[ -n "${GODOT_BIN:-}" ]]; then
    echo "$GODOT_BIN"
    return 0
  fi

  if command -v godot >/dev/null 2>&1; then
    command -v godot
    return 0
  fi

  if is_macos; then
    local paths=(
      "/Applications/Godot.app/Contents/MacOS/Godot"
      "/Applications/Godot_mono.app/Contents/MacOS/Godot"
      "$HOME/Applications/Godot.app/Contents/MacOS/Godot"
      "$HOME/Applications/Godot_mono.app/Contents/MacOS/Godot"
    )
    local p
    for p in "${paths[@]}"; do
      if [[ -x "$p" ]]; then
        echo "$p"
        return 0
      fi
    done
  fi

  return 1
}

detect_llvm_prefix() {
  if command -v brew >/dev/null 2>&1; then
    local p
    p="$(brew --prefix llvm 2>/dev/null || true)"
    if [[ -n "$p" && -d "$p" ]]; then
      echo "$p"
      return 0
    fi
  fi
  return 1
}

build_ext() {
  local platform="${PLATFORM:-}"
  local target="${TARGET:-}"
  local arch="${ARCH:-}"
  local jobs="${JOBS:-}"

  if [[ -z "$jobs" ]]; then
    jobs="$(ncpu)"
  fi

  if is_macos; then
    platform="${platform:-macos}"
    target="${target:-template_debug}"
    arch="${arch:-arm64}"
  else
    platform="${platform:-linux}"
    target="${target:-template_debug}"
    arch="${arch:-x86_64}"
  fi

  command -v scons >/dev/null 2>&1 || die "scons not found. Run ./dev.sh setup (or install scons)."

  local llvm_prefix=""
  if llvm_prefix="$(detect_llvm_prefix)"; then
    # Use Homebrew LLVM if available (common fix for macOS SDK/header issues).
    PATH="$llvm_prefix/bin:$PATH" \
    CC="$llvm_prefix/bin/clang" \
    CXX="$llvm_prefix/bin/clang++" \
      scons "platform=$platform" "target=$target" "arch=$arch" "-j$jobs"
  else
    scons "platform=$platform" "target=$target" "arch=$arch" "-j$jobs"
  fi
}

switch_demo() {
  local demo="${1:-}"
  [[ -n "$demo" ]] || die "demo is required"

  if [[ "$demo" == "menu" ]]; then
    demo="main_menu"
  fi

  "$ROOT_DIR/scripts/setup_demo.sh" "$demo"
}

print_status() {
  local line
  line="$(grep -n '^run/main_scene=' "$ROOT_DIR/project.godot" | head -n 1 || true)"
  if [[ -z "$line" ]]; then
    die "run/main_scene not found in project.godot"
  fi
  echo "$line"
}

cmd="${1:-help}"
shift || true

cd "$ROOT_DIR"

case "$cmd" in
  help|-h|--help)
    usage
    ;;
  setup)
    ./setup.sh
    ;;
  build)
    build_ext
    ;;
  demo)
    switch_demo "${1:-}"
    ;;
  menu)
    switch_demo "main_menu"
    ;;
  status)
    print_status
    ;;
  godot-bin)
    detect_godot_bin || die "godot not found. Install Godot or set GODOT_BIN=/path/to/godot"
    ;;
  run)
    if [[ -n "${1:-}" ]]; then
      switch_demo "$1"
    fi
    build_ext
    GODOT="$(detect_godot_bin || true)"
    [[ -n "$GODOT" ]] || die "godot not found. Install Godot or set GODOT_BIN=/path/to/godot"
    exec "$GODOT" --path "$ROOT_DIR"
    ;;
  edit)
    if [[ -n "${1:-}" ]]; then
      switch_demo "$1"
    fi
    build_ext
    GODOT="$(detect_godot_bin || true)"
    [[ -n "$GODOT" ]] || die "godot not found. Install Godot or set GODOT_BIN=/path/to/godot"
    # Prefer explicit editor mode; fall back if option isn't supported by the chosen binary.
    if "$GODOT" --help 2>/dev/null | grep -q -- '--editor'; then
      exec "$GODOT" --path "$ROOT_DIR" --editor
    elif "$GODOT" --help 2>/dev/null | grep -q -- ' -e'; then
      exec "$GODOT" --path "$ROOT_DIR" -e
    else
      exec "$GODOT" --path "$ROOT_DIR"
    fi
    ;;
  *)
    usage
    die "unknown command: $cmd"
    ;;
esac
