#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>

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

vector<vector<double>> multiply_matrices(const vector<vector<double>>& a,
    const vector<vector<double>>& b, int n) {
    vector<vector<double>> result(n, vector<double>(n, 0.0));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            result[i][j] = sum;
        }
    }

    return result;
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

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input_file1> <input_file2> <output_file>\n";
        return 1;
    }

    string file1 = argv[1];
    string file2 = argv[2];
    string outfile = argv[3];

    int n1, n2;

    auto start_read = high_resolution_clock::now();

    vector<vector<double>> mat1 = read_matrix(file1, n1);
    vector<vector<double>> mat2 = read_matrix(file2, n2);

    auto end_read = high_resolution_clock::now();
    auto read_time = duration_cast<microseconds>(end_read - start_read).count();

    if (n1 != n2) {
        cerr << "Error: matrix sizes don't match: " << n1 << " vs " << n2 << '\n';
        return 1;
    }

    int n = n1;

    auto start_mult = high_resolution_clock::now();

    vector<vector<double>> result = multiply_matrices(mat1, mat2, n);

    auto end_mult = high_resolution_clock::now();
    auto mult_time = duration_cast<microseconds>(end_mult - start_mult).count();
    auto total_time = duration_cast<microseconds>(end_mult - start_read).count();

    write_result(outfile, result, n);

    cout << "Read time: " << read_time / 1000.0 << " ms\n";
    cout << "Multiplication time: " << mult_time / 1000.0 << " ms\n";
    cout << "Total time: " << total_time / 1000.0 << " ms\n";
    cout << "Matrix size: " << n << "x" << n << '\n';
    cout << "Task volume: " << n * n * 3 << " elements\n";

    return 0;
}