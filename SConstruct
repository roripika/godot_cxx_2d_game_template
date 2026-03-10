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
env.Append(CPPPATH=["src", "src/thirdparty/jolt"])

# Jolt Physics Defines
# JPH_USE_NEON は意図的に外す — apple-m4 AArch64 でも DVec3.inl の
# float64x2x2_t.val[] コンパイルエラーを回避するためスカラー fallback を使用
env.Append(CPPDEFINES=[
    "JPH_USE_CUSTOM_STL",
    "JPH_PLATFORM_MACOS",
    "JPH_CPU_ARM",
])

# Jolt 専用コンパイル環境（-mcpu=apple-m4 を外して NEON intrinsics を無効化）
jolt_env = env.Clone()
jolt_env['CCFLAGS'] = [f for f in list(jolt_env.get('CCFLAGS', [])) if '-mcpu' not in str(f) and '-O' not in str(f)]

# ── Layer 1 (Core) 専用コンパイル環境 (物理的防波堤) ──────────────────────────
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
invaders_sources = get_recursive_cpp("src/invaders")
othello_sources = get_recursive_cpp("src/othello")
billiards_sources = get_recursive_cpp("src/billiards")
jolt_sources = get_recursive_cpp("src/thirdparty/jolt/Jolt")
plugin_sources = get_recursive_cpp("src/plugins")
root_sources = Glob("src/*.cpp")

# Jolt ソースは jolt_env（NEON なし）でコンパイル
jolt_objects   = jolt_env.SharedObject(jolt_sources + ["src/thirdparty/jolt_shim.cpp"])
other_sources  = mystery_sources + invaders_sources + othello_sources + billiards_sources + plugin_sources + root_sources
other_objects  = env.SharedObject(other_sources)

sources = core_objects + other_objects + jolt_objects

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/isometric_sandbox.{}.{}.framework/isometric_sandbox.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
    
    # Use Apple's libc++
    env.Append(CXXFLAGS=["-stdlib=libc++", "-std=c++17"])
    env.Append(LINKFLAGS=["-stdlib=libc++", "-lc++"])

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
