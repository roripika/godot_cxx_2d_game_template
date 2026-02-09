#!/bin/bash

# setup_demo.sh
# Switches the main scene in project.godot to the selected demo.

DEMO_TYPE=$1
PROJECT_FILE="project.godot"

if [ -z "$DEMO_TYPE" ]; then
    echo "Usage: ./scripts/setup_demo.sh [roguelike|platformer|mystery]"
    echo "Available demos:"
    echo "  - roguelike"
    echo "  - platformer"
    echo "  - mystery (coming soon)"
    exit 1
fi

SCENE_PATH=""

case "$DEMO_TYPE" in
    "roguelike")
        SCENE_PATH="res://samples/roguelike/demo_rogue.tscn"
        ;;
    "platformer")
        SCENE_PATH="res://samples/platformer/demo_side.tscn"
        ;;
    "mystery")
        SCENE_PATH="res://samples/mystery/demo_adv.tscn"
        ;;
    *)
        echo "Error: Unknown demo type '$DEMO_TYPE'"
        exit 1
        ;;
esac

# Check if project.godot exists
if [ ! -f "$PROJECT_FILE" ]; then
    echo "Error: project.godot not found in current directory."
    exit 1
fi

# Use sed to replace the run/main_scene line
# Works on both macOS (Generic BSD sed) and Linux (GNU sed) if handled carefully,
# but usually macOS needs empty string for extension backup with -i.
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s|run/main_scene=.*|run/main_scene=\"$SCENE_PATH\"|" "$PROJECT_FILE"
else
    sed -i "s|run/main_scene=.*|run/main_scene=\"$SCENE_PATH\"|" "$PROJECT_FILE"
fi

echo "Successfully switched main scene to: $SCENE_PATH"
echo "You can now run the project to play the $DEMO_TYPE demo."
