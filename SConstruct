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
# (ref: docs/architecture/layer_migration_plan.md §5 Quality Assurance)
core_env = env.Clone()
core_env['CPPPATH'] = [p for p in list(core_env['CPPPATH']) if str(p) != 'src']
core_env.Append(CPPPATH=[
    "src/core",    # Core internal header resolution
    "src/plugins/views", # For any shared view interfaces if necessary
])

# Layer 1 Sources (Core infrastructure)
core_source_files = (
    Glob("src/core/*.cpp") +
    Glob("src/core/components/*.cpp") +
    Glob("src/core/entities/*.cpp") +
    Glob("src/core/items/*.cpp") +
    Glob("src/core/logger/*.cpp") +
    Glob("src/core/scenario/*.cpp") +
    Glob("src/core/services/*.cpp") +
    Glob("src/core/yaml/*.cpp") +
    Glob("src/core/views/*.cpp")
)
core_objects = [core_env.SharedObject(f) for f in core_source_files]

# Layer 2+ / Mystery & Plugins
other_sources = (
    Glob("src/*.cpp") +
    # Layer 2: Mystery template
    Glob("src/mystery/*.cpp") +
    # Glob("src/mystery/entities/*.cpp") + # Placeholder if files are added later
    Glob("src/mystery/scenes/*.cpp") +
    # Plugins (genre-specific, isolated)
    Glob("src/plugins/features/fighting/*.cpp") +
    Glob("src/plugins/features/sandbox/*.cpp") +
    Glob("src/plugins/features/rhythm/*.cpp")
)

sources = core_objects + other_sources

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/isometric_sandbox.{}.{}.framework/isometric_sandbox.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
    
    # Use Apple's libc++ to avoid symbol mismatches with Homebrew LLVM
    env.Append(CXXFLAGS=["-stdlib=libc++"])
    env.Append(LINKFLAGS=["-stdlib=libc++"])

    # M4 / Apple Silicon optimization
    if env["arch"] == "arm64":
        env.Append(CCFLAGS=["-mcpu=apple-m4", "-O3"])
        
    # Parallel build optimization: using all logical cores
    import multiprocessing
    env.SetOption('num_jobs', multiprocessing.cpu_count())
    print(f"Building with {multiprocessing.cpu_count()} parallel jobs (M4 Optimized)")

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
