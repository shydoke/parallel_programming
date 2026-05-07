import csv
import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

# =========================
# НАСТРОЙКИ
# =========================

EXECUTABLE = "main.exe"

# размеры матриц
MATRIX_SIZES = [200, 400, 800, 1200, 1600, 2000]

# количество MPI процессов
CORES = [1, 2, 4, 8]

CSV_FILE = "benchmark_results.csv"

# =========================
# ГЕНЕРАЦИЯ МАТРИЦ
# =========================

def generate_matrix_file(filename, n):
    matrix = np.random.rand(n, n)

    with open(filename, "w") as f:
        f.write(f"{n}\n")

        for row in matrix:
            f.write(" ".join(map(str, row)) + "\n")


# =========================
# ЗАПУСК MPI ПРОГРАММЫ
# =========================

def run_benchmark(size, cores):
    file1 = f"matA_{size}.txt"
    file2 = f"matB_{size}.txt"
    outfile = f"out_{size}_{cores}.txt"

    print(f"Generating matrices {size}x{size}...")
    generate_matrix_file(file1, size)
    generate_matrix_file(file2, size)

    command = [
        "mpiexec",
        "-n",
        str(cores),
        EXECUTABLE,
        file1,
        file2,
        outfile
    ]

    print(f"Running: size={size}, cores={cores}")

    start = time.perf_counter()

    result = subprocess.run(
        command,
        capture_output=True,
        text=True
    )

    end = time.perf_counter()

    elapsed = end - start

    print(result.stdout)

    if result.returncode != 0:
        print(result.stderr)
        raise RuntimeError("MPI program failed")

    return elapsed


# =========================
# BENCHMARK
# =========================

results = []

for size in MATRIX_SIZES:
    for cores in CORES:
        elapsed = run_benchmark(size, cores)

        results.append({
            "matrix_size": size,
            "cores": cores,
            "time_sec": elapsed
        })

        print(f"Done: {size}x{size}, cores={cores}, time={elapsed:.3f} sec\n")


# =========================
# СОХРАНЕНИЕ CSV
# =========================

with open(CSV_FILE, "w", newline="") as csvfile:
    writer = csv.DictWriter(
        csvfile,
        fieldnames=["matrix_size", "cores", "time_sec"]
    )

    writer.writeheader()

    for row in results:
        writer.writerow(row)

print(f"CSV saved: {CSV_FILE}")

# =========================
# ПОСТРОЕНИЕ ГРАФИКА
# =========================

plt.figure(figsize=(10, 6))

for cores in CORES:
    x = []
    y = []

    for row in results:
        if row["cores"] == cores:
            x.append(row["matrix_size"])
            y.append(row["time_sec"])

    plt.plot(
        x,
        y,
        marker='o',
        label=f"{cores} cores"
    )

plt.xlabel("Matrix size")
plt.ylabel("Execution time (sec)")
plt.title("MPI Matrix Multiplication Performance")
plt.grid(True)
plt.legend()

plt.savefig("benchmark_plot.png")
plt.show()

print("Graph saved: benchmark_plot.png")