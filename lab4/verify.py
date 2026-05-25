import subprocess
import numpy as np
import time

sizes = [200, 400, 800, 1200, 1600, 2000]

exe = "main.exe"

def load_matrix(path):
    with open(path) as f:
        n = int(f.readline())
        data = [list(map(float, f.readline().split())) for _ in range(n)]
    return np.array(data, dtype=np.float64)

def run_cuda(a, b, out):
    result = subprocess.run(
        [exe, a, b, out],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print("CUDA ERROR:")
        print(result.stderr)
        raise RuntimeError("CUDA failed")

    out_text = result.stdout.strip()

    if not out_text:
        print("EMPTY OUTPUT")
        print(result.stderr)
        raise RuntimeError("No CUDA time output")

    return float(out_text)

for n in sizes:
    print(f"\nSize {n}")

    a_file = f"A_{n}.txt"
    b_file = f"B_{n}.txt"
    out_file = f"C_{n}.txt"

    try:
        gpu_time = run_cuda(a_file, b_file, out_file)
    except Exception as e:
        print("CUDA failed:", e)
        continue

    A = load_matrix(a_file)
    B = load_matrix(b_file)

    start = time.time()
    C_np = A @ B
    np_time = time.time() - start

    C_cuda = load_matrix(out_file)

    err = np.max(np.abs(C_np - C_cuda))

    print(f"CUDA time: {gpu_time:.3f} ms")
    print(f"NumPy time: {np_time*1000:.3f} ms")
    print(f"Max error: {err:.6e}")

    print("OK" if err < 1e-6 else "WRONG")
