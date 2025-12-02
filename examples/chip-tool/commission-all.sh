#!/usr/bin/env bash
set -euo pipefail

# Commission all uncommissioned Matter devices with one command.
#
# By default, this repeatedly invokes chip-tool "pairing onnetwork" until
# there are no more devices entering commissioning mode (detected by
# consecutive timeouts). Optionally, it can filter by discovered discriminators
# and attempt commissioning per discriminator.
#
# Requirements:
# - Run from connectedhomeip/examples/chip-tool (or adjust CHIP_TOOL path)
# - chip-tool must be built: out/host/chip-tool
# - Firebase-based credentials issuer is already integrated in this chip-tool
#
# Usage:
#   ./commission-all.sh [-p PIN] [-s START_NODE_ID] [-n NAME] [-t TIMEOUT]
#                       [-m MAX] [--use-discriminator] [--long|--short]
#                       [--verbose]
#
# Examples:
#   ./commission-all.sh -p 20202021 -s 10 -n alpha
#   ./commission-all.sh --use-discriminator --long
#

CHIP_TOOL=${CHIP_TOOL:-"out/host/chip-tool"}
SETUP_PIN=20202021
START_NODE_ID=10
COMMISSIONER_NAME="alpha"
TIMEOUT=45                # seconds per attempt (default tuned for 12 devices)
MAX_DEVICES=12            # default: commission 12 devices
USE_DISCRIMINATOR=1       # default: use discriminator-based pairing
DISC_KIND="long"          # or "short"
VERBOSE=0

log() { echo "[commission-all] $*"; }
vv()  { if [[ $VERBOSE -eq 1 ]]; then echo "[commission-all] $*" >&2; fi; }

usage() {
  cat <<EOF
Usage: $0 [options]
  -p, --pin PIN              Setup PIN code (default: $SETUP_PIN)
  -s, --start-id ID          Starting node id (default: $START_NODE_ID)
  -n, --name NAME            Commissioner name (default: $COMMISSIONER_NAME)
  -t, --timeout SEC          Per-attempt timeout seconds (default: $TIMEOUT)
  -m, --max N                Max devices to commission (default: $MAX_DEVICES)
      --use-discriminator    Use discovered discriminators for targeted pairing
      --long                 Use long discriminator (default)
      --short                Use short discriminator
      --verbose              Verbose logging
  -h, --help                 Show this help
EOF
}

# Parse args
while [[ $# -gt 0 ]]; do
  case "$1" in
    -p|--pin) SETUP_PIN="$2"; shift 2;;
    -s|--start-id) START_NODE_ID="$2"; shift 2;;
    -n|--name) COMMISSIONER_NAME="$2"; shift 2;;
    -t|--timeout) TIMEOUT="$2"; shift 2;;
    -m|--max) MAX_DEVICES="$2"; shift 2;;
    --use-discriminator) USE_DISCRIMINATOR=1; shift;;
    --long) DISC_KIND="long"; shift;;
    --short) DISC_KIND="short"; shift;;
    --verbose) VERBOSE=1; shift;;
    -h|--help) usage; exit 0;;
    *) echo "Unknown option: $1" >&2; usage; exit 2;;
  esac
done

if [[ ! -x "$CHIP_TOOL" ]]; then
  echo "chip-tool not found at $CHIP_TOOL. Build it first: ninja -C out/host chip-tool" >&2
  exit 1
fi

commissioned=0
node_id=$START_NODE_ID
consec_failures=0
max_failures=3
empty_passes=0
max_empty_passes=40


# Attempt to commission a single device using base onnetwork (no discriminator)
pair_one_plain() {
  local nid=$1
  vv "Pairing onnetwork node=$nid pin=$SETUP_PIN timeout=$TIMEOUT"
  if "$CHIP_TOOL" pairing onnetwork "$nid" "$SETUP_PIN" --commissioner-name "$COMMISSIONER_NAME" --timeout "$TIMEOUT" >/tmp/commission-one.log 2>&1; then
    if grep -qiE "Device commissioning completed with success|Received CommissioningComplete response, errorCode=0" /tmp/commission-one.log; then
      vv "Success: node $nid"
      return 0
    fi
  fi
  vv "Failed: node $nid (see /tmp/commission-one.log)"
  return 1
}

# Attempt to commission a single device filtered by discriminator
pair_one_disc() {
  local nid=$1
  local disc=$2
  local disc_flag
  if [[ "$DISC_KIND" == "short" ]]; then
    disc_flag="onnetwork-short"
  else
    disc_flag="onnetwork-long"
  fi
  vv "Pairing $disc_flag node=$nid disc=$disc pin=$SETUP_PIN timeout=$TIMEOUT"
  if "$CHIP_TOOL" pairing "$disc_flag" "$nid" "$SETUP_PIN" "$disc" --commissioner-name "$COMMISSIONER_NAME" --timeout "$TIMEOUT" >/tmp/commission-one.log 2>&1; then
    if grep -qiE "Device commissioning completed with success|Received CommissioningComplete response, errorCode=0" /tmp/commission-one.log; then
      vv "Success: node $nid (disc=$disc)"
      return 0
    fi
  fi
  # Fallback: if long discriminator failed, try short discriminator derived from long
  if [[ "$DISC_KIND" != "short" ]]; then
    local short_disc=$(( disc / 256 ))
    vv "Retry with onnetwork-short node=$nid short_disc=$short_disc"
    if "$CHIP_TOOL" pairing onnetwork-short "$nid" "$SETUP_PIN" "$short_disc" --commissioner-name "$COMMISSIONER_NAME" --timeout "$TIMEOUT" >/tmp/commission-one.log 2>&1; then
      if grep -qiE "Device commissioning completed with success|Received CommissioningComplete response, errorCode=0" /tmp/commission-one.log; then
        vv "Success (short fallback): node $nid (short_disc=$short_disc from long=$disc)"
        return 0
      fi
    fi
  fi
  vv "Failed: node $nid (disc=$disc) (see /tmp/commission-one.log)"
  return 1
}

# Extract unique discriminators from mDNS
list_discriminators() {
  if command -v avahi-browse >/dev/null 2>&1; then
    vv "Using avahi-browse for discovery"
    # Prefer CM=1 (open commissioning window). If none, fall back to any D= present.
    mapfile -t ds_cm1 < <(
      avahi-browse -p _matterc._udp -r -t 2>/dev/null \
        | awk -F';' '$1=="=" { txt=""; for (i=9;i<=NF;i++) txt=txt $i " "; if (txt ~ /"CM=1"/ && txt ~ /"D=[0-9]+"/) { match(txt, /"D=([0-9]+)"/, m); print m[1] } }' \
        | sort -n | uniq || true
    )
    if [[ ${#ds_cm1[@]} -eq 0 ]]; then
      mapfile -t ds_all < <(
        avahi-browse -p _matterc._udp -r -t 2>/dev/null \
          | awk -F';' '$1=="=" { txt=""; for (i=9;i<=NF;i++) txt=txt $i " "; if (txt ~ /"D=[0-9]+"/) { match(txt, /"D=([0-9]+)"/, m); print m[1] } }' \
          | sort -n | uniq || true
      )
      if [[ ${#ds_all[@]} -eq 0 ]]; then
        return 0
      fi
      if [[ "$DISC_KIND" == "short" ]]; then
        for d in "${ds_all[@]}"; do echo $(( d / 256 )); done | sort -n | uniq
      else
        printf "%s\n" "${ds_all[@]}"
      fi
      return 0
    fi
    # CM=1 found
    if [[ "$DISC_KIND" == "short" ]]; then
      for d in "${ds_cm1[@]}"; do echo $(( d / 256 )); done | sort -n | uniq
    else
      printf "%s\n" "${ds_cm1[@]}"
    fi
    return 0
  fi
  vv "Using chip-tool discover for discovery"
  "$CHIP_TOOL" discover commissionables 2>/dev/null |
    awk 'tolower($0) ~ /discriminator/ {
           for (i=1; i<=NF; i++) {
             if ($i ~ /^[0-9]+$/) { print $i; break }
           }
         }' | sort -n | uniq || true
}

if [[ $USE_DISCRIMINATOR -eq 0 ]]; then
  log "Commissioning using base onnetwork (no discriminator filtering)"
  while (( commissioned < MAX_DEVICES )); do
    if pair_one_plain "$node_id"; then
      commissioned=$((commissioned+1)); node_id=$((node_id+1)); consec_failures=0
      log "Commissioned: $commissioned (last node $((node_id-1)))"
      sleep 1
    else
      consec_failures=$((consec_failures+1))
      if (( consec_failures >= max_failures )); then
        log "No more commissionable devices (consecutive failures reached $max_failures). Stopping."
        break
      fi
      sleep 2
    fi
  done
else
  log "Commissioning using discovered discriminators ($DISC_KIND)"
  while (( commissioned < MAX_DEVICES )); do
    mapfile -t discs < <(list_discriminators)
    if (( ${#discs[@]} == 0 )); then
      empty_passes=$((empty_passes+1))
      if (( empty_passes >= max_empty_passes )); then
        log "No commissionable devices discovered after $max_empty_passes attempts. Stopping."
        break
      fi
      sleep 3
      continue
    fi
    empty_passes=0
    pass_success=0
    for d in "${discs[@]}"; do
      if pair_one_disc "$node_id" "$d"; then
        commissioned=$((commissioned+1)); node_id=$((node_id+1)); pass_success=1
        log "Commissioned: $commissioned (last node $((node_id-1))) via discriminator $d"
        sleep 1
      else
        vv "No device commissioned for discriminator $d on this attempt"
      fi
      if (( commissioned >= MAX_DEVICES )); then break; fi
    done
    if (( pass_success == 0 )); then
      log "No successes in this pass; likely done. Stopping."
      break
    fi
  done
fi

log "Commissioning complete. Total commissioned: $commissioned"
exit 0

