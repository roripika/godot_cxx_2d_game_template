#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CSV="$ROOT_DIR/samples/mystery/translations/mystery_translations.csv"
SCENARIO="$ROOT_DIR/samples/mystery/scenario/mystery.yaml"

if [[ ! -f "$CSV" ]]; then
  echo "missing translation csv: $CSV" >&2
  exit 1
fi

if [[ ! -f "$SCENARIO" ]]; then
  echo "missing scenario yaml: $SCENARIO" >&2
  exit 1
fi

tmp_required="$(mktemp)"
tmp_csv="$(mktemp)"
tmp_missing="$(mktemp)"
trap 'rm -f "$tmp_required" "$tmp_csv" "$tmp_missing"' EXIT

# Keys referenced by YAML (text_key/speaker_key/shake_key).
rg -No '(text_key|speaker_key|shake_key):[[:space:]]*"[^"]+"' "$SCENARIO" \
  | sed -E 's/.*:[[:space:]]*"([^"]+)"/\1/' >"$tmp_required"

# Static tr("...") keys in Mystery scripts.
rg -No 'tr\("([^"]+)"\)' \
  "$ROOT_DIR/samples/mystery/scripts" \
  "$ROOT_DIR/samples/mystery" \
  -g'*.gd' \
  -g'*.tscn' \
  | sed -E 's/.*tr\("([^"]+)"\).*/\1/' >>"$tmp_required" || true

sort -u -o "$tmp_required" "$tmp_required"

# Keys declared in csv (ignore header/comment/blank lines).
tail -n +2 "$CSV" \
  | cut -d',' -f1 \
  | sed -E '/^[[:space:]]*$/d;/^[[:space:]]*#/d' \
  | sort -u >"$tmp_csv"

comm -23 "$tmp_required" "$tmp_csv" >"$tmp_missing"

if [[ -s "$tmp_missing" ]]; then
  echo "Missing translation keys:" >&2
  cat "$tmp_missing" >&2
  exit 1
fi

echo "Translation key check passed."
