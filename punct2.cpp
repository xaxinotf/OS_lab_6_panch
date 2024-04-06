#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <atomic>

const int n = 3; // Розміри матриці A
const int m = 4; // Спільний розмір
const int k = 2; // Розміри матриці B

std::vector<std::vector<int>> A(n, std::vector<int>(m));
std::vector<std::vector<int>> B(m, std::vector<int>(k));
std::vector<std::vector<int>> C(n, std::vector<int>(k));

std::mutex io_mutex;

const long long iterations = 1e9; // 10^9 ітерацій
long long sharedVariable = 0;
std::mutex sharedVariableMutex;
std::atomic<long long> atomicVariable(0);

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

void matrixMultiply(int num_threads) {
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        for (int row = i; row < n; row += num_threads) {
            for (int col = 0; col < k; ++col) {
                threads.emplace_back(multiplyRowByColumn, row, col);
            }
        }
    }

    for (auto &thread : threads) {
        thread.join();
    }
}

void increaseWithMutex() {
    for (long long i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(sharedVariableMutex);
        ++sharedVariable;
    }
}

void increaseWithoutMutex() {
    for (long long i = 0; i < iterations; ++i) {
        ++sharedVariable;
    }
}

void increaseAtomic() {
    for (long long i = 0; i < iterations; ++i) {
        atomicVariable.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    fillMatrixRandomly(A);
    fillMatrixRandomly(B);

    for (int num_threads = 1; num_threads <= n * k; ++num_threads) {
        auto start = std::chrono::high_resolution_clock::now();
        matrixMultiply(num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "Threads: " << num_threads << " - Elapsed time: " << elapsed.count() << "ms\n";
    }

    // Тести зі спільною змінною
    sharedVariable = 0;
    std::thread t1(increaseWithMutex);
    std::thread t2(increaseWithMutex);
    t1.join();
    t2.join();
    std::cout << "With mutex: " << sharedVariable << std::endl;

    sharedVariable = 0;
    std::thread t3(increaseWithoutMutex);
    std::thread t4(increaseWithoutMutex);
    t3.join();
    t4.join();
    std::cout << "Without mutex: " << sharedVariable << std::endl;

    atomicVariable = 0;
    std::thread t5(increaseAtomic);
    std::thread t6(increaseAtomic);
    t5.join();
    t6.join();
    std::cout << "With atomic: " << atomicVariable << std::endl;

    return 0;
}
