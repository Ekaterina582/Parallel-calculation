#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>

const int ITERATIONS = 1'000'000;
const int THREADS_COUNT = 10;

// Общий ресурс для всех подходов
int shared_value_no_sync = 0;
int shared_value_mutex = 0;
std::atomic<int> shared_value_atomic(0);

std::mutex mtx;

void increment_no_sync() {
    for (int i = 0; i < ITERATIONS; ++i) {
        ++shared_value_no_sync;
    }
}

void increment_mutex() {
    for (int i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++shared_value_mutex;
    }
}

void increment_atomic() {
    for (int i = 0; i < ITERATIONS; ++i) {
        ++shared_value_atomic;
    }
}

void test_approach(void (*func)(), const std::string& name) {
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();

    // Создаем и запускаем потоки
    for (int i = 0; i < THREADS_COUNT; ++i) {
        threads.emplace_back(func);
    }

    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Выводим результаты
    if (name == "No synchronization") {
        std::cout << name << " result: " << shared_value_no_sync;
    }
    else if (name == "With mutex") {
        std::cout << name << " result: " << shared_value_mutex;
    }
    else {
        std::cout << name << " result: " << shared_value_atomic;
    }

    std::cout << " | Time: " << duration.count() << "s\n";
}

int main() {
    std::cout << "Testing different synchronization approaches with "
        << THREADS_COUNT << " threads and "
        << ITERATIONS << " iterations per thread...\n\n";

    // Тестируем подход без синхронизации
    test_approach(increment_no_sync, "No synchronization");

    // Тестируем подход с мьютексом
    test_approach(increment_mutex, "With mutex");

    // Тестируем подход с атомарными операциями
    test_approach(increment_atomic, "With atomic");

    return 0;
}