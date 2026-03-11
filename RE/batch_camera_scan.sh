#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

ROOT="${1:-$REPO_ROOT/proprietary}"
OUT="${2:-$REPO_ROOT/RE/out}"
MODE="${3:-quick}"   # quick | r2 | deep
SCOPE="${4:-camera}" # camera | all

mkdir -p "$OUT/exports" "$OUT/info" "$OUT/libs" "$OUT/sensor"

FILES=()
shopt -s nocasematch
while IFS= read -r path; do
  case "$SCOPE" in
    camera)
      case "$path" in
        */vendor/lib/*.so|*/vendor/lib64/*.so)
          case "$path" in
            *mmcamera*|*actuator*|*eeprom*|*chromatix*|*sensor*)
              FILES+=("$path")
              ;;
          esac
          ;;
      esac
      ;;
    all)
      case "$path" in
        */vendor/*.so|*/vendor/*/*.so|*/vendor/*/*/*.so)
          FILES+=("$path")
          ;;
      esac
      ;;
    *)
      echo "Unknown scope: $SCOPE (expected camera|all)" >&2
      exit 2
      ;;
  esac
done < <(rg --files "$ROOT")
shopt -u nocasematch

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No matching camera blobs under $ROOT" >&2
  exit 1
fi

for blob in "${FILES[@]}"; do
  base="$(basename "$blob")"
  rabin2 -I "$blob" > "$OUT/info/$base.info.txt" 2>/dev/null
  rabin2 -E "$blob" > "$OUT/exports/$base.exports.txt" 2>/dev/null
  rabin2 -l "$blob" > "$OUT/libs/$base.libs.txt" 2>/dev/null

  if rg -q "sensor_open_lib" "$OUT/exports/$base.exports.txt"; then
    python3 RE/dump_sensor_open_lib.py "$blob" > "$OUT/sensor/$base.sensor.txt" || true
    python3 RE/dump_output_info.py "$blob" > "$OUT/sensor/$base.output_info.txt" || true
  fi

  case "$MODE" in
    quick)
      ;;
    r2)
    r2 -q -e bin.relocs.apply=true -c "e scr.color=false; iI; iE; iS" "$blob" \
        > "$OUT/info/$base.r2.txt" 2> "$OUT/info/$base.r2.err.txt"
      ;;
    deep)
      r2 -q -e bin.relocs.apply=true -A -c "e scr.color=false; iI; iE; iS; izz; afl" "$blob" \
        > "$OUT/info/$base.r2.txt" 2> "$OUT/info/$base.r2.err.txt"
      ;;
    *)
      echo "Unknown mode: $MODE (expected quick|r2|deep)" >&2
      exit 2
      ;;
  esac
done

echo "Wrote outputs to $OUT"
