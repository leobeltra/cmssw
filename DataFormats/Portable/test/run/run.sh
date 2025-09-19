#!/usr/bin/env bash
set -euo pipefail

# --------- params ----------
BUILD_JOBS="${1:-20}"
GPU="${2:-0}"

export CUDA_VISIBLE_DEVICES="$GPU"
export HIP_VISIBLE_DEVICES="$GPU"

# Problem sizes
SIZE_LIST=(10 100 1000 10000 100000 1000000)

# How many runs per size:
# We'll do TOTAL_RUNS = WARMUP + SAMPLES, and then discard the first (warm-up).
WARMUP=2
SAMPLES=20
TOTAL_RUNS=$((WARMUP + SAMPLES))

# Paths (adapt as needed)
CMSSW_BASE="/data/user/lebeltra/hackathon_blocks/CMSSW_15_1_0_pre5"
SRC_DIR="$CMSSW_BASE/src/DataFormats/Portable"
RUN_DIR="$CMSSW_BASE/test/el8_amd64_gcc12"
OUT_DIR="$SRC_DIR/test/results"
OUT_CSV="$OUT_DIR/results_avg_std_cuda.csv"

mkdir -p "$OUT_DIR"

# Load CMSSW runtime
cd "$CMSSW_BASE"
eval "$(scram runtime -sh)"

# Optional rebuild (uncomment if you need it)
# cd "$SRC_DIR" && scram b -j "$BUILD_JOBS"

# Go to run dir
cd "$RUN_DIR" || { echo "Run dir not found: $RUN_DIR"; exit 1; }

EXEC="./Device_blocksCudaAsync"
[[ -x "$EXEC" ]] || { echo "ERROR: $EXEC not found or not executable"; exit 1; }

# CSV header
echo "element_size,mean_soa,std_soa,mean_soablocks,std_soablocks" > "$OUT_CSV"

# Helper to compute mean/std from stdin
awk_mean_std='
  { s+=$1; ss+=$1*$1; n++ }
  END{
    if(n<1){ print "NaN NaN"; exit }
    m=s/n;
    if(n>1){ sd=sqrt((ss - s*s/n)/(n-1)); } else { sd=0.0; }
    printf("%.9f %.9f\n", m, sd);
  }
'

for size in "${SIZE_LIST[@]}"; do
  tmp_soa="$(mktemp)"
  tmp_blk="$(mktemp)"

  echo ">>> size = $size"
  for ((r=1; r<=TOTAL_RUNS; ++r)); do
    out="$("$EXEC" "$size" 2>&1 || true)"

    # Extract both timings from program output
    t_soa="$(grep -m1 -E 'Total execution time for SoAs:' <<<"$out" | awk '{print $(NF-1)}')"
    t_blk="$(grep -m1 -E 'Total execution time for SoABlocks:' <<<"$out" | awk '{print $(NF-1)}')"

    # Sanity check
    if [[ -z "${t_soa:-}" || -z "${t_blk:-}" ]]; then
      echo "WARN: could not parse timings for size=$size (run $r). Output was:"
      echo "$out"
      continue
    fi

    echo "$t_soa" >> "$tmp_soa"
    echo "$t_blk" >> "$tmp_blk"
  done

  # Discard first line (warm-up)
  soa_stats="$(tail -n +$((WARMUP+1)) "$tmp_soa" | awk "$awk_mean_std")"
  blk_stats="$(tail -n +$((WARMUP+1)) "$tmp_blk" | awk "$awk_mean_std")"

  mean_soa="$(awk '{print $1}' <<<"$soa_stats")"
  std_soa="$(awk '{print $2}' <<<"$soa_stats")"
  mean_blk="$(awk '{print $1}' <<<"$blk_stats")"
  std_blk="$(awk '{print $2}' <<<"$blk_stats")"

  echo "$size,$mean_soa,$std_soa,$mean_blk,$std_blk" >> "$OUT_CSV"

  rm -f "$tmp_soa" "$tmp_blk"
done

echo "Wrote: $OUT_CSV"
