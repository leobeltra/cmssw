#!/usr/bin/env python3
import csv
import math
import matplotlib.pyplot as plt

CSV_PATH = "results_avg_cuda.csv"   # update path if needed
OUT_PNG  = "performance_cuda.png"  # updated output filename

sizes = []
mean_soa = []
std_soa  = []
mean_blk = []
std_blk  = []

with open(CSV_PATH, "r", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        sizes.append(int(row["element_size"]))
        mean_soa.append(float(row["mean_soas"]))
        std_soa.append(float(row["std_soas"]))
        mean_blk.append(float(row["mean_genericview"]))
        std_blk.append(float(row["std_genericview"]))

plt.figure(figsize=(9,5))
# error bars
plt.errorbar(sizes, mean_soa, yerr=std_soa, marker="o", linestyle="-", capsize=3, label="Manual copies")
plt.errorbar(sizes, mean_blk, yerr=std_blk, marker="s", linestyle="-", capsize=3, label="deepCopy")

plt.xscale("log")
# If your y range spans >10x, uncomment the next line:
# plt.yscale("log")

plt.xlabel("Problem size (elements)")
plt.ylabel("Execution time (ms)")
plt.title("Manual copies vs deepCopy — GPU (20 runs, 2 warm-up)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(OUT_PNG, dpi=300)
print(f"Saved: {OUT_PNG}")
# plt.show()   # uncomment if you want an interactive window