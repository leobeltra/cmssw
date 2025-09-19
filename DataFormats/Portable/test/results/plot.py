#!/usr/bin/env python3
import csv
import math
import matplotlib.pyplot as plt

CSV_PATH = "results_avg_std_cpu.csv"   # update path if needed
OUT_PNG  = "performance_with_std_cpu.png"

sizes = []
mean_pointers = []
std_pointers  = []
mean_spans = []
std_spans  = []

with open(CSV_PATH, "r", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        sizes.append(int(row["element_size"]))
        mean_pointers.append(float(row["mean_pointers"]))
        std_pointers.append(float(row["std_pointers"]))
        mean_spans.append(float(row["mean_spans"]))
        std_spans.append(float(row["std_spans"]))

plt.figure(figsize=(9,5))
# error bars
plt.errorbar(sizes, mean_pointers, yerr=std_pointers, marker="o", linestyle="-", capsize=3, label="Kernels with pointers")
plt.errorbar(sizes, mean_spans, yerr=std_spans, marker="s", linestyle="-", capsize=3, label="Kernels with spans")

plt.xscale("log")
# If your y range spans >10x, uncomment the next line:
plt.yscale("log")

plt.xlabel("Problem size (elements)")
plt.ylabel("Execution time (ms)")
plt.title("Pointers vs Spans — CPU (20 runs, 2 warm-up)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(OUT_PNG, dpi=300)
print(f"Saved: {OUT_PNG}")
# plt.show()   # uncomment if you want an interactive window