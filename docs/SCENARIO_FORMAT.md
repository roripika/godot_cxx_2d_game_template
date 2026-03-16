# Scenario Format Specification (v2.0)

This is the grammar specification for Karakuri Scenarios (YAML/JSON). AI assistants MUST produce output compatible with this format.

## 1. COMPILATION PRINCIPLE
Scenarios are **COMPILED** at load-time into a `CompiledScene` structure.
- **NEVER** assume the Kernel executes raw strings or dictionaries at runtime.
- **ALWAYS** ensure your output follows the explicit key structure required by `TaskSpec`.

## 2. BASIC STRUCTURE
A scenario file consists of scenes, each containing a sequence of actions.

```yaml
start_scene: prologue
scenes:
  prologue:
    on_enter:
      - action: dialogue
        payload:
          speaker: system
          text: "Welcome to Karakuri."
```

## 3. ACTION DEFINITIONS
AI MUST NOT invent new action names or payload keys. Use only registered ones.

### Common Actions:

#### `dialogue`
- **speaker**: (String) ID of the speaker.
- **text**: (String) Content of the dialogue.

#### `wait`
- **duration**: (Float/Int) Seconds to wait.
- **signal**: (String, Optional) Event name to wait for.

#### `set_flag` / `if_flag`
- **scope**: (String) "global", "session", or "scene".
- **key**: (String) Targeted variable key.
- **value**: (Any) Targeted value.

## 4. STRICT SYNTAX RULES
- **STRICT RULE**: Unsupported keys in `payload` MUST NOT be included.
- **STRICT RULE**: Action names MUST be lowercase and match the `ActionRegistry` Exactly.
- **STRICT RULE**: All triggers/signals MUST be explicitly defined.

## 5. REPRODUCIBLE SCENARIO EXAMPLES

### Correct Implementation
```yaml
# Good: Valid keys and structure
- action: wait
  payload:
    duration: 3.5
```

### Incorrect Implementation
```yaml
# Bad: Typo in action name
- action: WaitAction 
  payload:
    time: 3.5 # Bad: 'time' is not a valid key for wait task
```

## 6. AI GUIDELINES FOR SCENARIO GENERATION
1. **NEVER** hallucinate new action properties. Check `validate_and_setup` in C++ source if unsure.
2. **ALWAYS** use the correct scope for flags (Scene scope is for internal scene logic only).
3. **NEVER** use complex nested objects in `payload` unless the specific Task defines them.
4. **MUST** maintain alphabetical key order in `payload` where possible for better readability by other AI.

**FINAL RULE**: Undefined or malformed scenarios WILL cause a Kernel Panic during the `load_scenario` phase. Verify every key before outputting.
