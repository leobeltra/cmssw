#!/usr/bin/env python3
import csv
import math
import matplotlib.pyplot as plt

CSV_PATH = "results_mem_cuda.csv"   # update path if needed
OUT_PNG  = "performance_mem_with_cuda.png"

sizes = []
mean_soa = []
std_soa  = []
mean_aos = []
std_aos  = []

with open(CSV_PATH, "r", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        sizes.append(int(row["element_size"]))
        mean_soa.append(float(row["mean_soa"]))
        std_soa.append(float(row["std_soa"]))
        mean_aos.append(float(row["mean_aos"]))
        std_aos.append(float(row["std_aos"]))

plt.figure(figsize=(9,5))
# error bars
plt.errorbar(sizes, mean_soa, yerr=std_soa, marker="o", linestyle="-", capsize=3, label="Kernels with SoA")
plt.errorbar(sizes, mean_aos, yerr=std_aos, marker="s", linestyle="-", capsize=3, label="Kernels with AoS")

plt.xscale("log")
# If your y range AoS >10x, uncomment the next line:
# plt.yscale("log")

plt.xlabel("Problem size (elements)")
plt.ylabel("Execution time (ms)")
plt.title("SoA vs AoS — GPU (20 runs, 2 warm-up)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(OUT_PNG, dpi=300)
print(f"Saved: {OUT_PNG}")
# plt.show()   # uncomment if you want an interactive window