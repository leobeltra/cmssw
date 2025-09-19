#!/usr/bin/env python3
import sys, os, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

DEFAULT_CSVS = [
    "/data/user/mmichail/hackathon_19/CMSSW_15_1_0_pre5/src/DataFormats/Portable/test/results/results_cpu.csv",
    "/data/user/mmichail/hackathon_19/CMSSW_15_1_0_pre5/src/DataFormats/Portable/test/results/results_test_cpu.csv",
]

# DEFAULT_CSVS = [
#     "/data/user/mmichail/hackathon_19/CMSSW_15_1_0_pre5/src/DataFormats/Portable/test/results/results_gpu.csv",
#     "/data/user/mmichail/hackathon_19/CMSSW_15_1_0_pre5/src/DataFormats/Portable/test/results/results_test_gpu.csv",
# ]

REQUIRED_COLS = {"element_size", "mean", "std"}

def infer_label(path):
    name = os.path.splitext(os.path.basename(path))[0]
    return "Standard methods" if "test" in name else "SoA methods"

def read_csv(path):
    sizes, means, stds = [], [], []
    with open(path, newline="") as f:
        r = csv.DictReader(f)
        if not REQUIRED_COLS.issubset(r.fieldnames or []):
            raise ValueError(f"{os.path.basename(path)} must have columns: element_size, mean, std. Found: {r.fieldnames}")
        for row in r:
            sizes.append(int(row["element_size"]))
            means.append(float(row["mean"]))
            stds.append(float(row["std"]))
    # sort by element size to make lines meaningful
    order = sorted(range(len(sizes)), key=lambda i: sizes[i])
    sizes  = [sizes[i] for i in order]
    means  = [means[i] for i in order]
    stds   = [stds[i]  for i in order]
    return sizes, means, stds

def main():
    csv_paths = sys.argv[1:] if len(sys.argv) > 1 else DEFAULT_CSVS

    # Read all datasets
    datasets = []
    for p in csv_paths:
        sizes, means, stds = read_csv(p)
        label = os.path.splitext(os.path.basename(p))[0]  # e.g., results_avg_std_cpu
        datasets.append((sizes, means, stds, infer_label(p)))

    if not datasets:
        print("No CSVs provided.", file=sys.stderr)
        sys.exit(1)

    fig, ax = plt.subplots()
    ax.set_xscale("log", base=10)
   

    # distinct markers per series (matplotlib will pick colors)
    markers = ["o", "s", "^", "D", "v", "P", "X", "*"]

    # Build union of xticks across all series
    all_sizes = sorted({s for sizes,_,_,_ in datasets for s in sizes})
    xticks = [1] + all_sizes  # keep the “fake 0” at x=1 like before
    ax.set_xticks(xticks)
    ax.set_xticklabels(["0"] + [str(x) for x in all_sizes])

    # Plot all series
    for idx, (sizes, means, stds, label) in enumerate(datasets):
        marker = markers[idx % len(markers)]
        ax.errorbar(sizes, means, yerr=stds, fmt=f"-{marker}", capsize=4, label=label)

    pad = 10
    ax.set_xlim(1, max(all_sizes) * pad)

    ax.set_xlabel("Element No")
    ax.set_ylabel("Average time (ms)")
    ax.set_title("Standard methods vs SoA methods in CPU")
    # ax.set_title("Standard methods vs SoA methods in GPU")
    ax.grid(True, which="major")
    ax.minorticks_off()
    ax.legend()

    fig.tight_layout()

    # Name output by joining base names
    out_png = os.path.join(os.path.dirname(csv_paths[0]), "Standard_vs_SoA_methods_cpu.png")
    # out_png = os.path.join(os.path.dirname(csv_paths[0]), "Standard_vs_SoA_methods_gpu.png")
    fig.savefig(out_png, dpi=150)
    print(f"Saved plot: {out_png}")

if __name__ == "__main__":
    main()
