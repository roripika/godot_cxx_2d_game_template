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

# ── Layer 1 (Core) 専用コンパイル環境 (物理的防波堤) ──────────────────────────
# src を CPPPATH から取り除き、Core 内部パスのみを追加する。
# これにより src/core/ 以下が #include "mystery/..." を書いた瞬間に
# コンパイルエラーが発生し、Layer 1 → Layer 2 の依存混入を物理的に検出できる。
core_env = env.Clone()
core_env['CPPPATH'] = [p for p in list(core_env['CPPPATH']) if str(p) != 'src']
core_env.Append(CPPPATH=[
    "src/core",    # Core internal header resolution
    "src/plugins/views", 
])

# 再帰的に .cpp を収集するヘルパー
def get_recursive_cpp(path):
    cpps = []
    for root, dirs, files in os.walk(path):
        for f in files:
            if f.endswith(".cpp"):
                cpps.append(os.path.join(root, f))
    return cpps

core_sources = get_recursive_cpp("src/core")
core_objects = core_env.SharedObject(core_sources)

# Layer 2+ / Mystery & Plugins
mystery_sources = get_recursive_cpp("src/mystery")
plugin_sources = get_recursive_cpp("src/plugins")
root_sources = Glob("src/*.cpp")

other_sources = mystery_sources + plugin_sources + root_sources
other_objects = env.SharedObject(other_sources)

sources = core_objects + other_objects

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/isometric_sandbox.{}.{}.framework/isometric_sandbox.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
    
    # Use Apple's libc++
    env.Append(CXXFLAGS=["-stdlib=libc++"])
    env.Append(LINKFLAGS=["-stdlib=libc++"])

    # M4 / Apple Silicon optimization
    if env["arch"] == "arm64":
        env.Append(CCFLAGS=["-mcpu=apple-m4", "-O3"])
        
    import multiprocessing
    env.SetOption('num_jobs', multiprocessing.cpu_count())

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
