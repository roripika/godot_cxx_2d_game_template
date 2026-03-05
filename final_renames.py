import glob
import os

print("Applying final class renames and build fixes...")

# List of files to check for class renames
files = glob.glob('src/**/*.cpp', recursive=True) + \
        glob.glob('src/**/*.h', recursive=True) + \
        glob.glob('src/**/*.hpp', recursive=True)

class_renames = [
    ('KarakuriScenarioRunner', 'ScenarioRunner'),
    ('KarakuriLocalizationService', 'LocalizationService'),
    ('KarakuriSaveService', 'SaveService'),
    ('KarakuriLogger', 'Logger'),
    ('GameEntity', 'BaseEntity'),
    ('KarakuriTestimonySession', 'TestimonySession'),
    ('KarakuriYamlLite', 'YamlLite'),
]

for filepath in files:
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    new_content = content
    for old, new in class_renames:
        new_content = new_content.replace(old, new)
    
    # Fix MysteryFlag scope in MysteryManager.h and MysteryManager.cpp
    if 'mystery_manager' in filepath:
        new_content = new_content.replace('MysteryFlag', 'mystery::MysteryFlag')
        new_content = new_content.replace('EvidenceID', 'mystery::EvidenceID')
        # Fix redundant mystery::mystery::
        new_content = new_content.replace('mystery::mystery::', 'mystery::')
    
    # Office/Haunted scene logic fixes
    if 'scene_logic' in filepath:
        new_content = new_content.replace('MysteryManager', 'mystery::MysteryManager')
        new_content = new_content.replace('EvidenceManager', 'mystery::EvidenceManager')
        new_content = new_content.replace('mystery::mystery::', 'mystery::')

    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)

print("Final renames completed.")
