#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Tweaks for the binding layout
env.Append(CPPPATH=["src"])
env.Append(CPPPATH=["src"])
sources = Glob("src/*.cpp") + Glob("src/core/*.cpp") + Glob("src/views/*.cpp") + Glob("src/entities/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/isometric_sandbox.{}.{}.framework/isometric_sandbox.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
    
    # M2 / Apple Silicon optimization
    if env["arch"] == "arm64":
        env.Append(CCFLAGS=["-mcpu=apple-m2", "-O3"])
        
    # Ensure correct SDK path is used if standard headers are missing
    try:
        import subprocess
        sdk_path = subprocess.check_output(["xcrun", "--show-sdk-path"]).decode("utf-8").strip()
        env.Append(CCFLAGS=["-isysroot", sdk_path])
        env.Append(LINKFLAGS=["-isysroot", sdk_path])
    except Exception as e:
        print(f"Warning: Could not detect SDK path: {e}")
else:
    library = env.SharedLibrary(
        "bin/isometric_sandbox{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
