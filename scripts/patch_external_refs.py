#!/usr/bin/env python3
"""Prefix 14 Core class names with karakuri:: in external (non-core) files."""
import re
import os

WORKSPACE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Classes to prefix — exclude ones that are Inventory as a method variable name
CLASSES = [
    "AdventureGameStateBase",
    "UniversalWorldData",
    "RoguelikeGenerator",
    "RoguelikeManager",
    "InteractionManager",
    "DialogueUI",
    "GameEntity",
    "GameItem",
    "Inventory",
    "HitboxComponent",
    "HurtboxComponent",
    "RayCastComponent",
    "InventoryUI",
    "InventorySlotUI",
]

TARGET_FILES = [
    "src/views/side_scrolling_view.h",
    "src/views/side_scrolling_view.cpp",
    "src/views/isometric_view.h",
    "src/views/isometric_view.cpp",
    "src/mystery/mystery_manager.cpp",
    "src/mystery/mystery_manager.h",
    "src/scenes/office_scene_logic.h",
    "src/scenes/office_scene_logic.cpp",
    "src/scenes/haunted_spot_scene_logic.h",
    "src/scenes/haunted_spot_scene_logic.cpp",
    "src/plugins/features/sandbox/mining_component.h",
    "src/plugins/features/sandbox/mining_component.cpp",
    "src/plugins/features/sandbox/building_component.h",
    "src/plugins/features/sandbox/building_component.cpp",
]

def patch_file(rel_path, classes):
    full = os.path.join(WORKSPACE, rel_path)
    if not os.path.exists(full):
        print(f"  SKIP (missing): {rel_path}")
        return

    with open(full) as f:
        src = f.read()

    original = src

    # Build one combined pattern:
    # Match ClassName NOT already preceded by `karakuri::` or `::` or `"`
    # and NOT followed by `"` (string literal), `(` after method (set_inventory etc ignored via whitelist)
    for cls in classes:
        # Replace in C++ type contexts only:
        # Ref<Cls>, Cls *, Cls &, cast_to<Cls>, Cls::
        # Negative lookbehind: not preceded by karakuri:: or namespace:: or "
        pattern = r'(?<![a-zA-Z0-9_":])' + re.escape(cls) + r'(?![a-zA-Z0-9_"])'
        replacement = f'karakuri::{cls}'
        src = re.sub(pattern, replacement, src)

    # Safety: fix double-prefixes (karakuri::karakuri::)
    src = src.replace('karakuri::karakuri::', 'karakuri::')

    if src != original:
        with open(full, 'w') as f:
            f.write(src)
        print(f"  patched: {rel_path}")
    else:
        print(f"  no-change: {rel_path}")

for f in TARGET_FILES:
    patch_file(f, CLASSES)

print("\nDone.")
