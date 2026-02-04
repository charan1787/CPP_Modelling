
import csv
from collections import defaultdict
import matplotlib.pyplot as plt


# ---------------- CSV loading ----------------
def read_csv(path: str):
    rows = []
    with open(path, "r", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            # numeric conversions
            int_fields = ["N", "n_warps", "warp_cycles", "active_lane_cycles",
                          "mem_lane_ops", "divergent_branches", "reconverges"]
            float_fields = ["div_ratio", "param", "utilization",
                            "cycles_per_warp", "cycles_per_thread", "memops_per_cycle"]

            for k in int_fields:
                row[k] = int(float(row[k])) 
            for k in float_fields:
                row[k] = float(row[k])

            rows.append(row)
    return rows


rows = read_csv("results.csv")

# Group by workload
by_workload = defaultdict(list)
for r in rows:
    by_workload[r["workload"]].append(r)

# ---------------- styling: workload colors ----------------
workload_colors = {
    "branch_div":    "tab:blue",
    "nested_div":    "tab:purple",
    "compute_heavy": "tab:green",
    "memory_heavy":  "tab:red",
}

# In case some workloads are missing, only plot those that exist
available_workloads = [w for w in workload_colors.keys() if w in by_workload]


# ---------------- Figure 1: Combined 3-plot summary ----------------
fig, axes = plt.subplots(3, 1, figsize=(9, 12))
fig.suptitle("SIMT Analysis (All Workloads)", fontsize=14)

# ---- Plot 1: Divergence vs cycles_per_warp (only branch_div) ----
if "branch_div" in by_workload:
    branch = sorted(by_workload["branch_div"], key=lambda x: (x["N"], x["div_ratio"]))
    Ns = sorted(set(r["N"] for r in branch))
    for N in Ns:
        pts = [r for r in branch if r["N"] == N]
        xs = [r["div_ratio"] for r in pts]
        ys = [r["cycles_per_warp"] for r in pts]
        axes[0].plot(xs, ys, marker="o", label=f"N={N}")
    axes[0].set_title("Branch Divergence vs Cost (cycles per warp)")
    axes[0].set_xlabel("divergence ratio")
    axes[0].set_ylabel("cycles_per_warp")
    axes[0].grid(True)
    axes[0].legend()
else:
    axes[0].text(0.5, 0.5, "No branch_div data found", ha="center", va="center")
    axes[0].set_axis_off()

# ---- Plot 2: Utilization vs cycles_per_warp (color-coded by workload) ----
for w in available_workloads:
    pts = by_workload[w]
    axes[1].scatter(
        [r["utilization"] for r in pts],
        [r["cycles_per_warp"] for r in pts],
        alpha=0.75,
        label=w,
        color=workload_colors[w],
    )
axes[1].set_title("Utilization vs Cost (cycles per warp)")
axes[1].set_xlabel("utilization")
axes[1].set_ylabel("cycles_per_warp")
axes[1].grid(True)
axes[1].legend()

# ---- Plot 3: Memory intensity vs cycles_per_warp (color-coded by workload) ----
for w in available_workloads:
    pts = by_workload[w]
    axes[2].scatter(
        [r["memops_per_cycle"] for r in pts],
        [r["cycles_per_warp"] for r in pts],
        alpha=0.75,
        label=w,
        color=workload_colors[w],
    )
axes[2].set_title("Memory Intensity vs Cost (cycles per warp)")
axes[2].set_xlabel("memops_per_cycle (avg active mem ops per issued instr)")
axes[2].set_ylabel("cycles_per_warp")
axes[2].grid(True)
axes[2].legend()

plt.tight_layout(rect=[0, 0, 1, 0.96])
plt.savefig("simt_all_in_one.png", dpi=200)


# ---------------- Figure 2: Compute-heavy scaling ----------------
if "compute_heavy" in by_workload:
    ch = sorted(by_workload["compute_heavy"], key=lambda x: (x["N"], x["param"]))
    Ns = sorted(set(r["N"] for r in ch))

    plt.figure(figsize=(9, 5))
    for N in Ns:
        pts = [r for r in ch if r["N"] == N]
        xs = [r["param"] for r in pts]               # VADD reps
        ys = [r["cycles_per_thread"] for r in pts]   # normalized
        plt.plot(xs, ys, marker="o", label=f"N={N}")

    plt.title("Compute-heavy: cost per thread vs VADD repetitions")
    plt.xlabel("VADD repetitions (param)")
    plt.ylabel("cycles_per_thread")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig("compute_scaling.png", dpi=200)


# ---------------- Figure 3: Memory-heavy scaling ----------------
if "memory_heavy" in by_workload:
    mh = sorted(by_workload["memory_heavy"], key=lambda x: (x["N"], x["param"]))
    Ns = sorted(set(r["N"] for r in mh))

    plt.figure(figsize=(9, 5))
    for N in Ns:
        pts = [r for r in mh if r["N"] == N]
        xs = [r["param"] for r in pts]               # LD/ST pairs
        ys = [r["cycles_per_thread"] for r in pts]   # normalized
        plt.plot(xs, ys, marker="o", label=f"N={N}")

    plt.title("Memory-heavy: cost per thread vs LD/ST pairs")
    plt.xlabel("LD/ST pairs (param)")
    plt.ylabel("cycles_per_thread")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig("memory_scaling.png", dpi=200)

print("Saved:")
print(" - simt_all_in_one.png")
if "compute_heavy" in by_workload:
    print(" - compute_scaling.png")
if "memory_heavy" in by_workload:
    print(" - memory_scaling.png")
