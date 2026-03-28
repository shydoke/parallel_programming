import numpy as np
import sys

def generate_matrices(n):
    """генерирует две матрицы размера n x n"""
    mat1 = np.random.randn(n, n)
    mat2 = np.random.randn(n, n)
    
    with open(f"mat_{n}_1.txt", 'w') as f:
        f.write(f"{n}\n")
        for row in mat1:
            f.write(' '.join(f"{x:.6f}" for x in row) + '\n')
    
    with open(f"mat_{n}_2.txt", 'w') as f:
        f.write(f"{n}\n")
        for row in mat2:
            f.write(' '.join(f"{x:.6f}" for x in row) + '\n')
    
    print(f"Сгенерированы матрицы {n}x{n}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        n = int(sys.argv[1])
        generate_matrices(n)
    else:
        generate_matrices(10)