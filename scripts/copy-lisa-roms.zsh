#!/usr/bin/env zsh

set -euo pipefail

SRC_DIR="${1:-/Users/peter/GitHub/lisaem/bin/ROMS}"
DST_DIR="${2:-$HOME/Library/Application Support/LisaEm/ROMS}"

if [[ ! -d "$SRC_DIR" ]]; then
  echo "Source directory not found: $SRC_DIR" >&2
  exit 1
fi

mkdir -p "$DST_DIR"

typeset -a PART_PATTERNS
PART_PATTERNS=(
  # Lisa 2 boot ROM high byte
  '341-0?175'
  # Lisa 2 boot ROM low byte
  '341-0?176'
  # Lisa dual-parallel interface card ROM
  '341-0?193'
  # Lisa 2/10 disk controller ROM
  '341-0?281'
  # Lisa 2/5 disk controller ROM
  '341-0?290'
  # Mac XL boot ROM low byte
  '341-0?346'
  # Mac XL boot ROM high byte
  '341-0?347'
  # Sun Mac XL 800k disk ROM variant
  '341-0?8003'
)

match_lisa_part() {
  local upper_name="$1"
  local pattern
  for pattern in "$PART_PATTERNS[@]"; do
    if [[ "$upper_name" =~ ${pattern}([-.][A-Z0-9]+)? ]]; then
      return 0
    fi
  done
  return 1
}

is_candidate_rom() {
  local file_name="$1"
  local upper_name="${(U)file_name}"

  case "$upper_name" in
    *.ROM|*.BIN|*.IMAGE) ;;
    *) return 1 ;;
  esac

  if match_lisa_part "$upper_name"; then
    return 0
  fi

  # Fallback for ROM dumps that do not keep Apple part numbers in filename.
  if [[ "$upper_name" == *LISA* ]]; then
    return 0
  fi

  return 1
}

rom_role_dir() {
  local file_name="$1"
  local upper_name="${(U)file_name}"

  # Lisa 2 Boot ROM pair: 341-0175 (Hi), 341-0176 (Lo)
  if [[ "$upper_name" =~ 341-0?17(5|6)([-.][A-Z0-9]+)? ]]; then
    echo "Lisa2_BootROM"
    return 0
  fi

  # Lisa dual-parallel interface card ROM: 341-0193
  if [[ "$upper_name" =~ 341-0?193([-.][A-Z0-9]+)? ]]; then
    echo "DualParallel_CardROM"
    return 0
  fi

  # Lisa disk controller ROMs
  if [[ "$upper_name" =~ 341-0?281([-.][A-Z0-9]+)? ]]; then
    echo "Lisa2-10_DiskROM"
    return 0
  fi
  if [[ "$upper_name" =~ 341-0?290([-.][A-Z0-9]+)? ]]; then
    echo "Lisa2-5_DiskROM"
    return 0
  fi

  # Mac XL boot ROM pair: 341-0346 (Lo), 341-0347 (Hi)
  if [[ "$upper_name" =~ 341-0?34(6|7)([-.][A-Z0-9]+)? ]]; then
    echo "MacXL_BootROM"
    return 0
  fi

  # Sun Mac XL 800k disk ROM variant
  if [[ "$upper_name" =~ 341-0?8003([-.][A-Z0-9]+)? ]]; then
    echo "MacXL_800k_DiskROM"
    return 0
  fi

  # Fallback for Lisa-labeled dumps without Apple part numbers.
  if [[ "$upper_name" == *LISA* ]]; then
    echo "Lisa_Misc"
    return 0
  fi

  return 1
}

typeset -i copied=0
typeset -i skipped=0
typeset -i exists=0

while IFS= read -r -d '' file; do
  base="${file:t}"
  if ! is_candidate_rom "$base"; then
    continue
  fi

  if ! role_dir="$(rom_role_dir "$base")"; then
    ((skipped+=1))
    echo "SKIP (unclassified): $base"
    continue
  fi

  dest_dir="$DST_DIR/$role_dir"
  mkdir -p "$dest_dir"

  dest="$dest_dir/$base"
  if [[ -e "$dest" ]]; then
    ((exists+=1))
    echo "SKIP (exists): $base"
    continue
  fi

  cp "$file" "$dest"
  ((copied+=1))
  echo "COPY: $base -> $role_dir"
done < <(find "$SRC_DIR" -type f -print0)

echo "Done. Copied: $copied, skipped existing: $exists, skipped other: $skipped"
echo "Destination: $DST_DIR"
