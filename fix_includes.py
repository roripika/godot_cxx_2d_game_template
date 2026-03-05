import glob
import os
import re

print("Starting bulk include fix and namespace injection...")

replacements = [
    # Renamed files
    ('karakuri_scenario_runner.h', 'scenario_runner.h'),
    ('karakuri_localization_service.h', 'localization_service.h'),
    ('karakuri_save_service.h', 'save_service.h'),
    ('karakuri_yaml_lite.h', 'yaml_lite.h'),
    ('game_entity.h', 'base_entity.h'),
    # Path updates
    ('src/karakuri/', 'src/core/'),
    ('src/features/mystery/', 'src/mystery/'),
    ('src/entities/', 'src/core/entities/'),
    ('src/items/', 'src/core/items/'),
]

# Specifically for register_types.cpp and others that might have old paths
path_fixes = [
    ('core/scenario/karakuri_scenario_runner.h', 'core/scenario/scenario_runner.h'),
    ('core/services/karakuri_localization_service.h', 'core/services/localization_service.h'),
    ('core/services/karakuri_save_service.h', 'core/services/save_service.h'),
    ('core/yaml/karakuri_yaml_lite.h', 'core/yaml/yaml_lite.h'),
    ('core/entities/game_entity.h', 'core/entities/base_entity.h'),
]

def process_file(filepath, ns=None):
    print(f"Processing: {filepath}")
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    new_content = content
    # Apply replacements
    for old, new in replacements:
        new_content = new_content.replace(old, new)
    for old, new in path_fixes:
        new_content = new_content.replace(old, new)
        
    # Inject namespace if provided and not present
    if ns:
        if f"namespace {ns}" not in new_content:
            # Simple injection after includes
            lines = new_content.splitlines()
            last_include_idx = -1
            for i, line in enumerate(lines):
                if line.startswith('#include') or line.startswith('#pragma'):
                    last_include_idx = i
            
            if last_include_idx != -1:
                lines.insert(last_include_idx + 1, f'\nnamespace {ns} {{\n')
                lines.append(f'\n}} // namespace {ns}\n')
                new_content = '\n'.join(lines)
            else:
                new_content = f'namespace {ns} {{\n\n{new_content}\n\n}} // namespace {ns}\n'

    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)

# Target core files
core_files = glob.glob('src/core/**/*.cpp', recursive=True) + \
             glob.glob('src/core/**/*.h', recursive=True) + \
             glob.glob('src/core/**/*.hpp', recursive=True)

for f in core_files:
    process_file(f, ns='karakuri')

# Target mystery files
mystery_files = glob.glob('src/mystery/**/*.cpp', recursive=True) + \
                glob.glob('src/mystery/**/*.h', recursive=True) + \
                glob.glob('src/mystery/**/*.hpp', recursive=True)

for f in mystery_files:
    # Some mystery files might already have the namespace or need it
    # MysteryManager already has it, but others might not
    process_file(f, ns='mystery')

# General files (register_types.cpp, etc.)
for f in ['src/register_types.cpp', 'src/register_types.h', 'src/world_generator.cpp', 'src/world_generator.h']:
    if os.path.exists(f):
        process_file(f)

# SConstruct was already updated but let's be double sure about paths if needed
# No, SConstruct is fine.

print("Bulk fix completed successfully.")
