#!/bin/bash
# Scripts/build_extension.sh
# Builds the GDExtension using Homebrew LLVM to avoid macOS SDK issues.

# Ensure Homebrew LLVM bin is in PATH
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build command with explicit compilers
echo "Building GDExtension with Homebrew LLVM..."
/opt/homebrew/bin/scons platform=macos target=template_debug arch=arm64 CC=/opt/homebrew/opt/llvm/bin/clang CXX=/opt/homebrew/opt/llvm/bin/clang++ -j7

# Copy and sign for reliable loading (Framework path can be problematic)
echo "Copying and signing library..."
cp bin/isometric_sandbox.macos.template_debug.framework/libisometric_sandbox.macos.template_debug bin/libsandbox.dylib
codesign --force --deep --sign - bin/libsandbox.dylib

echo "Build complete."
