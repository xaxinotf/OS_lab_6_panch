#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

const int n = 3; // Розміри матриці A
const int m = 4; // Спільний розмір
const int k = 2; // Розміри матриці B

std::vector<std::vector<int>> A(n, std::vector<int>(m));
std::vector<std::vector<int>> B(m, std::vector<int>(k));
std::vector<std::vector<int>> C(n, std::vector<int>(k));

std::mutex io_mutex;

void fillMatrixRandomly(std::vector<std::vector<int>>& matrix) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10);

    for (auto &row : matrix) {
        for (auto &elem : row) {
            elem = dis(gen);
        }
    }
}

void multiplyRowByColumn(int row, int col) {
    int result = 0;
    for (int i = 0; i < m; ++i) {
        result += A[row][i] * B[i][col];
    }

    {
        std::lock_guard<std::mutex> guard(io_mutex);
        std::cout << "[" << row << "," << col << "]=" << result << std::endl;
    }

    C[row][col] = result;
}

void matrixMultiply() {
    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            threads.emplace_back(multiplyRowByColumn, i, j);
        }
    }

    for (auto &thread : threads) {
        thread.join();
    }
}

int main() {
    fillMatrixRandomly(A);
    fillMatrixRandomly(B);

    auto start = std::chrono::high_resolution_clock::now();
    matrixMultiply();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << "ms\n";

    return 0;
}
