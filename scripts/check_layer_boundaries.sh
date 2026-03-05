#!/usr/bin/env bash
# check_layer_boundaries.sh
#
# Layer アーキテクチャ境界違反を検出する Lint スクリプト。
# docs/architecture/layer_migration_plan.md §5 Quality Assurance に基づく。
#
# Usage:
#   ./scripts/check_layer_boundaries.sh
#   exit 0 → 違反なし
#   exit 1 → 違反あり（CI でブロック）

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CORE_DIR="$REPO_ROOT/src/core"
MYSTERY_DIR="$REPO_ROOT/src/mystery"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

violations=0

echo "🔍 Checking layer boundary violations..."
echo "   Layer 1 (core)    : $CORE_DIR"
echo "   Layer 2 (mystery) : $MYSTERY_DIR"
echo ""

# ── Check 1: src/core/ に "mystery::" の使用がないか ──────────────────────────
echo "Check 1: mystery:: namespace in src/core/"
matches=$(grep -rn 'mystery::' "$CORE_DIR" --include="*.h" --include="*.cpp" 2>/dev/null || true)
if [[ -n "$matches" ]]; then
    echo -e "${RED}  ✗ VIOLATION: mystery:: found in src/core/${NC}"
    echo "$matches" | sed 's/^/    /'
    violations=$((violations + 1))
else
    echo -e "${GREEN}  ✓ OK${NC}"
fi

# ── Check 2: src/core/ に #include "mystery/" がないか ────────────────────────
echo "Check 2: #include \"mystery/...\" in src/core/"
matches=$(grep -rn '#include.*"mystery/' "$CORE_DIR" --include="*.h" --include="*.cpp" 2>/dev/null || true)
if [[ -n "$matches" ]]; then
    echo -e "${RED}  ✗ VIOLATION: #include \"mystery/...\" found in src/core/${NC}"
    echo "$matches" | sed 's/^/    /'
    violations=$((violations + 1))
else
    echo -e "${GREEN}  ✓ OK${NC}"
fi

# ── Check 3: src/mystery/ に "karakuri::" 以外の Core 型が使われていないか ────
# ※ karakuri:: の呼び出し自体は許可（Layer 2 は Layer 1 を利用してよい）
# ※ 禁止: src/mystery/ が src/core/ の型を名前空間なし（グローバル）で使う
echo "Check 3: bare (non-karakuri) Core includes from src/mystery/"
matches=$(grep -rn '#include.*"core/' "$MYSTERY_DIR" --include="*.h" --include="*.cpp" 2>/dev/null || true)
if [[ -n "$matches" ]]; then
    # src/mystery が src/core/ を include するのは合法（Layer 2 → Layer 1 は OK）
    echo -e "${YELLOW}  ℹ INFO: src/mystery includes from src/core/ (allowed):${NC}"
    echo "$matches" | sed 's/^/    /'
else
    echo -e "${GREEN}  ✓ OK (no core/ includes in mystery)${NC}"
fi

# ── Check 4: src/core/ が src/plugins/ 以外のジャンル固有パスを include していないか ─
echo "Check 4: genre-specific plugin includes in src/core/ (other than plugins/)"
matches=$(grep -rn '#include.*"views/rhythm/\|#include.*"plugins/features/fighting\|#include.*"plugins/features/sandbox' "$CORE_DIR" --include="*.h" --include="*.cpp" 2>/dev/null || true)
if [[ -n "$matches" ]]; then
    echo -e "${YELLOW}  ⚠ WARNING: Core may depend on genre-specific plugins:${NC}"
    echo "$matches" | sed 's/^/    /'
    # Warning のみ（違反カウントなし）
else
    echo -e "${GREEN}  ✓ OK${NC}"
fi

echo ""
if [[ $violations -eq 0 ]]; then
    echo -e "${GREEN}✅ All layer boundary checks passed.${NC}"
    exit 0
else
    echo -e "${RED}❌ $violations violation(s) found. Fix before merging.${NC}"
    exit 1
fi
