#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <chrono>
#include <cuda_runtime.h>

using namespace std;
using namespace chrono;

#define TILE 32

#define CUDA_CHECK(call) do { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        cerr << "CUDA_ERROR: " << cudaGetErrorString(err) << endl; \
        exit(1); \
    } \
} while(0)

global
void multiply_kernel(const double* a,
                     const double* b,
                     double* c,
                     int n) {

    shared double As[TILE][TILE];
    shared double Bs[TILE][TILE];

    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;

    double sum = 0.0;

    for (int t = 0; t < (n + TILE - 1) / TILE; t++) {

        int tiledCol = t * TILE + threadIdx.x;
        int tiledRow = t * TILE + threadIdx.y;

        As[threadIdx.y][threadIdx.x] =
            (row < n && tiledCol < n) ? a[row * n + tiledCol] : 0.0;

        Bs[threadIdx.y][threadIdx.x] =
            (tiledRow < n && col < n) ? b[tiledRow * n + col] : 0.0;

        __syncthreads();

        #pragma unroll
        for (int k = 0; k < TILE; k++) {
            sum += As[threadIdx.y][k] * Bs[k][threadIdx.x];
        }

        __syncthreads();
    }

    if (row < n && col < n) {
        c[row * n + col] = sum;
    }
}

vector<double> read_matrix(const string& file, int& n) {
    ifstream f(file);
    if (!f.is_open()) {
        cerr << "Cannot open file " << file << endl;
        exit(1);
    }

    f >> n;
    vector<double> m(n * n);

    for (int i = 0; i < n * n; i++)
        f >> m[i];

    return m;
}

void write_matrix(const string& file, const vector<double>& m, int n) {
    ofstream f(file);
    f << n << "\n";

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            f << fixed << setprecision(6)
              << m[i * n + j] << " ";
        }
        f << "\n";
    }
}

vector<double> multiply_cuda(const vector<double>& a,
                             const vector<double>& b,
                             int n,
                             double& ms) {

    vector<double> c(n * n, 0.0);

    double *da, *db, *dc;
    size_t size = n * n * sizeof(double);

    CUDA_CHECK(cudaMalloc(&da, size));
    CUDA_CHECK(cudaMalloc(&db, size));
    CUDA_CHECK(cudaMalloc(&dc, size));

    CUDA_CHECK(cudaMemcpy(da, a.data(), size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(db, b.data(), size, cudaMemcpyHostToDevice));

    dim3 threads(TILE, TILE);
    dim3 blocks((n + TILE - 1) / TILE,
                (n + TILE - 1) / TILE);

    auto start = high_resolution_clock::now();

    multiply_kernel<<<blocks, threads>>>(da, db, dc, n);

    CUDA_CHECK(cudaDeviceSynchronize());

    auto end = high_resolution_clock::now();

    ms = duration<double, milli>(end - start).count();

    CUDA_CHECK(cudaMemcpy(c.data(), dc, size, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(da));
    CUDA_CHECK(cudaFree(db));
    CUDA_CHECK(cudaFree(dc));

    return c;
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cerr << "Usage: main.exe <input1> <input2> <output>\n";
        return 1;
    }

    string A_file = argv[1];
    string B_file = argv[2];
    string OUT_file = argv[3];

    int n1, n2;

    auto A = read_matrix(A_file, n1);
    auto B = read_matrix(B_file, n2);

    if (n1 != n2) {
        cerr << "Matrix size mismatch\n";
        return 1;
    }

    double gpu_time = 0;

    auto C = multiply_cuda(A, B, n1, gpu_time);

    write_matrix(OUT_file, C, n1);

    cout << gpu_time << endl;

    return 0;
}
