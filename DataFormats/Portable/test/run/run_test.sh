#!/usr/bin/env bash
set -euo pipefail

BUILD_JOBS="${1:-20}"
GPU="${2:-0}"

export CUDA_VISIBLE_DEVICES="$GPU"
export HIP_VISIBLE_DEVICES="$GPU"

SIZE_LIST=(10 100 1000 10000 100000 250000 750000 1000000)

CMSSW_BASE="/data/user/mmichail/hackathon_19/CMSSW_15_1_0_pre5"
SRC_DIR="$CMSSW_BASE/src/DataFormats/Portable"
RUN_DIR="$CMSSW_BASE/test/el8_amd64_gcc12"
OUT_DIR="$SRC_DIR/test/results"
# OUT_CSV="$OUT_DIR/results_test_gpu.csv"
OUT_CSV="$OUT_DIR/results_test_cpu.csv"

mkdir -p "$OUT_DIR"

# Optional build (uncomment if you want to rebuild each time)
#cd "$SRC_DIR" && scram b -j "$BUILD_JOBS"

# Load CMSSW runtime
cd "$CMSSW_BASE" && eval "$(scram runtime -sh)"

# Run experiments
cd "$RUN_DIR" || exit 1
# [[ -x ./Device_test_methodsCudaAsync ]] || { echo "ERROR: Device_test_methodsCudaAsync not found"; exit 1; }
[[ -x ./Device_test_methodsSerialSync ]] || { echo "ERROR: Device_test_methodsSerialSync not found"; exit 1; }

echo "element_size,mean,std" > "$OUT_CSV"

for size in "${SIZE_LIST[@]}"; do
  tmp="$(mktemp)"
  for i in {0..10}; do
    # out="$(./Device_test_methodsCudaAsync "$size" 2>&1)"
    out="$(./Device_test_methodsSerialSync "$size" 2>&1)"
    val="$(grep -m1 -E 'Average execution time:' <<<"$out" | awk '{print $(NF-1)}')"
    [[ -n "${val:-}" ]] && echo "$val" >> "$tmp"
  done

  mean="$(tail -n +2 "$tmp" | awk '{s+=$1; n++} END{if(n) printf("%.9f", s/n); else print "NaN"}')"
  std="$(tail -n +2 "$tmp"  | awk '{s+=$1; ss+=$1*$1; n++} END{if(n>1) printf("%.9f", sqrt((ss - s*s/n)/(n-1))); else print "NaN"}')"

  echo "$size,$mean,$std" >> "$OUT_CSV"
  rm -f "$tmp"
done

echo "Wrote: $OUT_CSV"
