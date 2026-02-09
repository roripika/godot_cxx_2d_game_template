#!/bin/bash
set -e

# Colors for output
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}Starting Environment Setup for Isometric Sandbox...${NC}"

# Check for git
if ! command -v git &> /dev/null; then
    echo "Error: git is not installed. Please install Xcode Command Line Tools."
    exit 1
fi

# Check for python3
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 is not installed."
    exit 1
fi

# Check for scons
if ! command -v scons &> /dev/null; then
    echo "Warning: scons is not found."
    if command -v brew &> /dev/null; then
        echo "Attempting to install scons via Homebrew..."
        brew install scons
    else
        echo "Attempting to install scons via pip..."
        # Utilizes --break-system-packages if necessary, but checks for venv first ideally.
        # For simplicity in this script context:
        pip3 install scons --user || pip3 install scons --break-system-packages --user
    fi

    if ! command -v scons &> /dev/null; then
        # SCons installed via pip --user might not be in PATH immediately
        if [ -d "$HOME/Library/Python/3.9/bin" ]; then
             export PATH="$HOME/Library/Python/3.9/bin:$PATH"
        elif [ -d "$HOME/.local/bin" ]; then
             export PATH="$HOME/.local/bin:$PATH"
        fi
    fi
    
    if ! command -v scons &> /dev/null; then
        echo "Error: Failed to install scons. Please install manually (e.g. brew install scons)."
        exit 1
    fi
fi

# Setup godot-cpp submodule
if [ ! -d "godot-cpp" ]; then
    echo -e "${GREEN}Cloning godot-cpp repository...${NC}"
    git clone -b 4.3 https://github.com/godotengine/godot-cpp.git
else
    echo "godot-cpp directory already exists."
fi

echo -e "${GREEN}Environment setup complete!${NC}"

# Generate env.sh for LLVM paths if needed
ENV_FILE="env.sh"
echo "# Source this file to set up build environment" > "$ENV_FILE"

if command -v brew &> /dev/null; then
    #Check for LLVM
    LLVM_PREFIX=$(brew --prefix llvm 2>/dev/null || echo "")
    if [ -n "$LLVM_PREFIX" ] && [ -d "$LLVM_PREFIX" ]; then
        echo "export PATH=\"$LLVM_PREFIX/bin:\$PATH\"" >> "$ENV_FILE"
        echo "export LDFLAGS=\"-L$LLVM_PREFIX/lib\"" >> "$ENV_FILE"
        echo "export CPPFLAGS=\"-I$LLVM_PREFIX/include\"" >> "$ENV_FILE"
        echo "export CC=\"$LLVM_PREFIX/bin/clang\"" >> "$ENV_FILE"
        echo "export CXX=\"$LLVM_PREFIX/bin/clang++\"" >> "$ENV_FILE"
        echo "Detected LLVM. Added configuration to $ENV_FILE."
    fi
fi

chmod +x "$ENV_FILE"
echo "You can now build the project using:"
echo "source ./env.sh && scons platform=macos target=template_debug arch=arm64"
