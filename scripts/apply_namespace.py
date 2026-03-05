#!/usr/bin/env python3
"""Apply namespace karakuri {} to 14 Core header/cpp file pairs."""
import re
import os
import sys

WORKSPACE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

header_files = [
    "src/core/universal_world_data.h",
    "src/core/interaction_manager.h",
    "src/core/dialogue_ui.h",
    "src/core/adventure_game_state.h",
    "src/core/entities/base_entity.h",
    "src/core/items/game_item.h",
    "src/core/items/inventory.h",
    "src/core/components/hitbox_component.h",
    "src/core/components/hurtbox_component.h",
    "src/core/components/raycast_component.h",
    "src/core/cycles/roguelike_generator.h",
    "src/core/cycles/roguelike_manager.h",
    "src/core/ui/inventory_ui.h",
    "src/core/ui/inventory_slot_ui.h",
]

cpp_files = [
    "src/core/universal_world_data.cpp",
    "src/core/interaction_manager.cpp",
    "src/core/dialogue_ui.cpp",
    "src/core/adventure_game_state.cpp",
    "src/core/entities/base_entity.cpp",
    "src/core/items/game_item.cpp",
    "src/core/items/inventory.cpp",
    "src/core/components/hitbox_component.cpp",
    "src/core/components/hurtbox_component.cpp",
    "src/core/components/raycast_component.cpp",
    "src/core/cycles/roguelike_generator.cpp",
    "src/core/cycles/roguelike_manager.cpp",
    "src/core/ui/inventory_ui.cpp",
    "src/core/ui/inventory_slot_ui.cpp",
]

def patch_header(path):
    full = os.path.join(WORKSPACE, path)
    with open(full) as f:
        src = f.read()

    if "namespace karakuri" in src:
        print(f"  SKIP (already patched): {path}")
        return

    # Insert 'namespace karakuri {' after 'using namespace godot;'
    src = re.sub(
        r'(using namespace godot;\n)',
        r'\1\nnamespace karakuri {\n',
        src, count=1
    )
    # Insert '} // namespace karakuri' before the final #endif
    # Handles both bare #endif and #endif // GUARD
    src = re.sub(
        r'(\};\n\n)(#endif)',
        r'\1} // namespace karakuri\n\n\2',
        src
    )
    with open(full, 'w') as f:
        f.write(src)
    print(f"  h patched: {path}")


def patch_cpp(path):
    full = os.path.join(WORKSPACE, path)
    if not os.path.exists(full):
        print(f"  SKIP (no cpp): {path}")
        return
    with open(full) as f:
        src = f.read()

    if "namespace karakuri" in src:
        print(f"  SKIP (already patched): {path}")
        return

    # Find the last 'using namespace' line and insert 'namespace karakuri {'
    # after it, then wrap entire rest of file until EOF
    # Strategy: insert after last 'using namespace xxx;' block
    match = None
    for m in re.finditer(r'^using namespace \w+;\n', src, re.MULTILINE):
        match = m
    if match is None:
        # No 'using namespace', wrap from start
        src = "namespace karakuri {\n\n" + src + "\n} // namespace karakuri\n"
    else:
        insert_pos = match.end()
        src = (src[:insert_pos]
               + "\nnamespace karakuri {\n\n"
               + src[insert_pos:].rstrip('\n')
               + "\n\n} // namespace karakuri\n")

    with open(full, 'w') as f:
        f.write(src)
    print(f"  cpp patched: {path}")


for h in header_files:
    patch_header(h)

for c in cpp_files:
    patch_cpp(c)

print("\nAll done.")
