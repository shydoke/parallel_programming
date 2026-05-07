#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <mpi.h>

using namespace std;
using namespace chrono;

vector<vector<double>> read_matrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: cannot open file " << filename << '\n';
        exit(1);
    }

    file >> n;
    if (n <= 0) {
        cerr << "Error: matrix size must be positive\n";
        exit(1);
    }

    vector<vector<double>> matrix(n, vector<double>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> matrix[i][j];
        }
    }

    file.close();
    return matrix;
}

void write_result(const string& filename, const vector<vector<double>>& matrix, int n) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: cannot create file " << filename << '\n';
        exit(1);
    }

    file << n << '\n';
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << fixed << setprecision(6) << matrix[i][j] << ' ';
        }
        file << '\n';
    }

    file.close();
}

vector<vector<double>> multiply_matrices_parallel(const vector<vector<double>>& a,
    const vector<vector<double>>& b, int n, int rank, int size) {

    vector<vector<double>> result(n, vector<double>(n, 0.0));

    int rows_per_proc = n / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? n : start_row + rows_per_proc;

    vector<vector<double>> local_result(end_row - start_row, vector<double>(n, 0.0));

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            local_result[i - start_row][j] = sum;
        }
    }

    if (rank == 0) {
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < n; j++) {
                result[i][j] = local_result[i - start_row][j];
            }
        }

        for (int p = 1; p < size; p++) {
            int p_start_row = p * rows_per_proc;
            int p_rows = (p == size - 1) ? n - p_start_row : rows_per_proc;

            vector<double> buffer(p_rows * n);
            MPI_Recv(buffer.data(), p_rows * n, MPI_DOUBLE, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < p_rows; i++) {
                for (int j = 0; j < n; j++) {
                    result[p_start_row + i][j] = buffer[i * n + j];
                }
            }
        }
    }
    else {
        vector<double> buffer(local_result.size() * n);
        for (size_t i = 0; i < local_result.size(); i++) {
            for (int j = 0; j < n; j++) {
                buffer[i * n + j] = local_result[i][j];
            }
        }
        MPI_Send(buffer.data(), buffer.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    return result;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0) {
            cerr << "Usage: mpirun -np <N> " << argv[0] << " <input_file1> <input_file2> <output_file>\n";
        }
        MPI_Finalize();
        return 1;
    }

    string file1 = argv[1];
    string file2 = argv[2];
    string outfile = argv[3];

    int n1, n2;
    vector<vector<double>> mat1, mat2;

    auto start_total = high_resolution_clock::now();
    auto start_read = high_resolution_clock::now();

    if (rank == 0) {
        mat1 = read_matrix(file1, n1);
        mat2 = read_matrix(file2, n2);

        if (n1 != n2) {
            cerr << "Error: matrix sizes don't match: " << n1 << " vs " << n2 << '\n';
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&n1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int n = n1;

    if (rank != 0) {
        mat1.resize(n, vector<double>(n));
        mat2.resize(n, vector<double>(n));
    }

    for (int i = 0; i < n; i++) {
        MPI_Bcast(mat1[i].data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(mat2[i].data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    auto end_read = high_resolution_clock::now();
    auto read_time = duration_cast<microseconds>(end_read - start_read).count();

    auto start_mult = high_resolution_clock::now();

    vector<vector<double>> result;
    if (rank == 0) {
        result.resize(n, vector<double>(n));
    }

    result = multiply_matrices_parallel(mat1, mat2, n, rank, size);

    auto end_mult = high_resolution_clock::now();
    auto mult_time = duration_cast<microseconds>(end_mult - start_mult).count();

    if (rank == 0) {
        auto end_total = high_resolution_clock::now();
        auto total_time = duration_cast<microseconds>(end_total - start_total).count();

        write_result(outfile, result, n);

        cout << "Read time: " << read_time / 1000.0 << " ms\n";
        cout << "Multiplication time: " << mult_time / 1000.0 << " ms\n";
        cout << "Total time: " << total_time / 1000.0 << " ms\n";
        cout << "Matrix size: " << n << "x" << n << '\n';
        cout << "Task volume: " << n * n * 3 << " elements\n";
        cout << "MPI processes: " << size << '\n';
    }

    MPI_Finalize();
    return 0;
}