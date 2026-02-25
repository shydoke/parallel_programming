import sys
import numpy as np


def read_matrix(filename):
    """читаем матрицу из файла"""
    try:
        with open(filename, 'r') as f:
            n = int(f.readline().strip())
            matrix = []
            for _ in range(n):
                row = list(map(float, f.readline().strip().split()))
                matrix.append(row)
        return np.array(matrix), n
    except FileNotFoundError:
        print(f"ошибка: файл {filename} не найден")
        sys.exit(1)
    except Exception as e:
        print(f"ошибка при чтении {filename}: {e}")
        sys.exit(1)


def verify():
    if len(sys.argv) != 4:
        print("использование: python verify.py <файл1> <файл2> <результат>")
        return

    file1 = sys.argv[1]
    file2 = sys.argv[2]
    result_file = sys.argv[3]

    print("\n=== ВЕРИФИКАЦИЯ ===")

    # читаем входные матрицы
    mat1, n1 = read_matrix(file1)
    mat2, n2 = read_matrix(file2)

    if n1 != n2:
        print(f"ошибка: размеры матриц не совпадают ({n1} vs {n2})")
        return

    # читаем результат программы
    our_result, n_res = read_matrix(result_file)

    if n_res != n1:
        print(f"ошибка: размер результата ({n_res}) не совпадает с ожидаемым ({n1})")
        return

    # умножаем через numpy
    numpy_result = np.matmul(mat1, mat2)

    # сравниваем
    diff = np.abs(our_result - numpy_result)
    max_diff = np.max(diff)

    print(f"размер: {n1}x{n1}")
    print(f"макс. разница: {max_diff:.6e}")

    if max_diff < 1e-10:
        print("✓ ВСЕ ОК")
    else:
        print("✗ НЕ СОВПАДАЕТ")
        print("\nпервые элементы:")
        print("наша:", our_result[0][:3])
        print("numpy:", numpy_result[0][:3])


if __name__ == "__main__":
    verify()