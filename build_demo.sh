#!/bin/bash

# ==============================================================================
# Godot Demo App Distribution Builder (Multi-Platform)
# ==============================================================================
# Usage: ./build_demo.sh [demo_name] [platform]
# Example: ./build_demo.sh mystery mac
#          ./build_demo.sh main win
#          ./build_demo.sh mystery apk
#          ./build_demo.sh main ipa
# ==============================================================================

DEMO=$1
PLATFORM=$2

if [ -z "$DEMO" ] || [ -z "$PLATFORM" ]; then
    echo "Usage: ./build_demo.sh [demo_name] [platform]"
    echo "Available demos:"
    echo "  mystery  - Apparition at the Warehouse"
    echo "  main     - Main Menu"
    echo ""
    echo "Available platforms:"
    echo "  mac  - macOS Application (.app)"
    echo "  win  - Windows Executable (.exe)"
    echo "  apk  - Android Package (.apk)"
    echo "  ipa  - iOS Archive (.ipa) (Requires Xcode and Mac environment)"
    exit 1
fi

APP_NAME=""
SCENE_PATH=""

if [ "$DEMO" == "mystery" ]; then
    APP_NAME="Mystery_Demo"
    SCENE_PATH="res://samples/mystery/mystery_title.tscn"
elif [ "$DEMO" == "main" ]; then
    APP_NAME="Main_Demo"
    SCENE_PATH="res://samples/main_menu.tscn"
else
    echo "Unknown demo: $DEMO"
    exit 1
fi

GODOT_BIN="/Applications/Godot.app/Contents/MacOS/Godot"
if [ ! -f "$GODOT_BIN" ]; then
    echo "ERROR: Godot binary not found at $GODOT_BIN"
    exit 1
fi

# ==============================================================================
# Auto Template Download (Detect current version and fetch from Github)
# ==============================================================================
GODOT_VERSION=$($GODOT_BIN --version | awk '{print $1}')
echo "Detected Godot Version: $GODOT_VERSION"

TEMPLATE_DIR="$HOME/Library/Application Support/Godot/export_templates/$GODOT_VERSION"
if [ ! -d "$TEMPLATE_DIR" ]; then
    echo "=========================================================="
    echo " ERROR: Export templates not found for Godot $GODOT_VERSION"
    echo " Because your Godot version ($GODOT_VERSION) is a custom or pre-release build,"
    echo " standard GitHub release templates cannot be automatically downloaded."
    echo ""
    echo " Please open the Godot Editor, go to: "
    echo "   [Editor > Manage Export Templates...]"
    echo " and Download/Install the required templates."
    echo "=========================================================="
    exit 1
fi

# ==============================================================================
# Platform Configuration
# ==============================================================================
PRESET_NAME=""
EXT=""
BUILD_DIR=""

if [ "$PLATFORM" == "mac" ]; then
    PRESET_NAME="macOS"
    EXT="zip"
    BUILD_DIR="builds/macOS"
elif [ "$PLATFORM" == "win" ]; then
    PRESET_NAME="Windows Desktop"
    EXT="exe"
    BUILD_DIR="builds/Windows"
elif [ "$PLATFORM" == "apk" ]; then
    PRESET_NAME="Android"
    EXT="apk"
    BUILD_DIR="builds/Android"
elif [ "$PLATFORM" == "ipa" ]; then
    PRESET_NAME="iOS"
    EXT="ipa"
    BUILD_DIR="builds/iOS"
else
    echo "Unknown platform: $PLATFORM"
    exit 1
fi

mkdir -p "$BUILD_DIR"

# Ensure export_presets.cfg covers all targets
cat << EOF > export_presets.cfg
[preset.0]
name="macOS"
platform="macOS"
runnable=true
custom_features=""
export_filter="all_resources"
export_path="builds/macOS/App.zip"

[preset.0.options]
texture_format/bptc=true
texture_format/s3tc=true
texture_format/etc=false
texture_format/etc2=false
application/name="\$APP_NAME"

[preset.1]
name="Windows Desktop"
platform="Windows Desktop"
runnable=true
custom_features=""
export_filter="all_resources"
export_path="builds/Windows/\$APP_NAME.exe"

[preset.1.options]
texture_format/bptc=true
texture_format/s3tc=true
texture_format/etc=false
texture_format/etc2=false

[preset.2]
name="Android"
platform="Android"
runnable=true
custom_features=""
export_filter="all_resources"
export_path="builds/Android/\$APP_NAME.apk"

[preset.2.options]
texture_format/bptc=false
texture_format/s3tc=false
texture_format/etc=true
texture_format/etc2=true
architectures/arm64-v8a=true
architectures/armeabi-v7a=true
architectures/x86=false
architectures/x86_64=false

[preset.3]
name="iOS"
platform="iOS"
runnable=true
custom_features=""
export_filter="all_resources"
export_path="builds/iOS/\$APP_NAME.ipa"

[preset.3.options]
texture_format/bptc=false
texture_format/s3tc=false
texture_format/etc=true
texture_format/etc2=true
architecture/arm64=true
application/name="\$APP_NAME"
EOF

echo "=========================================================="
echo " Building $APP_NAME for $PRESET_NAME..."
echo " Targeting Main Scene: $SCENE_PATH"
echo "=========================================================="

cp project.godot project.godot.bak
sed -i '' "s|^run/main_scene=.*|run/main_scene=\"$SCENE_PATH\"|" project.godot

# Run Godot CLI export
TARGET_FILE="${BUILD_DIR}/${APP_NAME}.${EXT}"
$GODOT_BIN --headless --export-release "$PRESET_NAME" "$TARGET_FILE"
EXPORT_RESULT=$?

mv project.godot.bak project.godot

if [ $EXPORT_RESULT -ne 0 ]; then
    echo "=========================================================="
    echo " ERROR: Godot export failed for $PRESET_NAME."
    echo " Ensure Android SDK/Keystore, or Xcode iOS provisioning profiles"
    echo " are correctly configured in Editor Settings if targeting Mobile."
    echo "=========================================================="
    exit 1
fi

if [ "$PLATFORM" == "mac" ]; then
    echo "Extracting the macOS Application Bundle..."
    cd "$BUILD_DIR"
    unzip -q -o "${APP_NAME}.zip"
    APP_BUNDLE=$(ls -d *.app 2>/dev/null | head -n 1)
    if [ -n "$APP_BUNDLE" ]; then
        if [ "$APP_BUNDLE" != "${APP_NAME}.app" ]; then
            mv "$APP_BUNDLE" "${APP_NAME}.app"
        fi
        echo "SUCCESS: Distribution file created at: ${BUILD_DIR}/${APP_NAME}.app"
    else
        echo "ERROR: Failed to extract .app bundle."
    fi
    rm -f "${APP_NAME}.zip"
    cd ../..
else
    echo "SUCCESS: Distribution file created at: $TARGET_FILE"
fi

echo "Build process finished."
