#!/bin/bash

# launch_game.sh
# Attempts to launch the project using the Godot Executable on macOS.

# Prefer a CLI-installed Godot first (e.g., via Homebrew cask which provides `godot`).
if command -v godot &> /dev/null; then
    echo "Launching Godot from PATH: $(command -v godot)"
    exec godot --path .
fi

# Common default install paths for Godot on macOS
GODOT_PATHS=(
    "/Applications/Godot.app/Contents/MacOS/Godot"
    "/Applications/Godot_mono.app/Contents/MacOS/Godot"
    "$HOME/Applications/Godot.app/Contents/MacOS/Godot"
)

CHOSEN_PATH=""

for path in "${GODOT_PATHS[@]}"; do
    if [ -f "$path" ]; then
        CHOSEN_PATH="$path"
        break
    fi
done

if [ -z "$CHOSEN_PATH" ]; then
    echo "Error: Godot.app not found in standard locations."
    echo "Please ensure 'Godot.app' is installed in /Applications or ~/Applications."
    echo "Alternatively, open the Godot Editor manually and import this folder."
    exit 1
fi

echo "Launching Godot from: $CHOSEN_PATH"
# --path . tells Godot to run the project in the current directory
"$CHOSEN_PATH" --path .
