#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

const int iterations = 1000;
int sharedVariable = 0;
std::mutex mtx;

void increaseWithMutex() {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        sharedVariable++;
        std::cout << "Thread ID: " << std::this_thread::get_id() << " - Shared Variable: " << sharedVariable << std::endl;
    }
}

void increaseWithoutMutex() {
    for (int i = 0; i < iterations; ++i) {
        sharedVariable++;
        std::cout << "Thread ID: " << std::this_thread::get_id() << " - Shared Variable: " << sharedVariable << std::endl;
    }
}

void increaseSynchronously(std::atomic<int>& counter) {
    while (true) {
        int local = counter.load();
        if (local >= iterations) break;

        if (counter.compare_exchange_strong(local, local + 1)) {
            std::lock_guard<std::mutex> lock(mtx);
            sharedVariable++;
            std::cout << "Thread ID: " << std::this_thread::get_id() << " - Shared Variable: " << sharedVariable << std::endl;
        }
    }
}

int main() {
    // З використанням критичного сегменту (mutex)
    sharedVariable = 0;
    std::thread t1(increaseWithMutex);
    std::thread t2(increaseWithMutex);
    t1.join();
    t2.join();

    // Без використання критичного сегменту (mutex)
    sharedVariable = 0;
    t1 = std::thread(increaseWithoutMutex);
    t2 = std::thread(increaseWithoutMutex);
    t1.join();
    t2.join();

    // Синхронне збільшення
    sharedVariable = 0;
    std::atomic<int> counter(0);
    t1 = std::thread(increaseSynchronously, std::ref(counter));
    t2 = std::thread(increaseSynchronously, std::ref(counter));
    t1.join();
    t2.join();

    return 0;
}
