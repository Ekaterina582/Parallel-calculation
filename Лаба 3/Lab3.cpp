#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <windows.h>

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
    if (name == "Без синхронизации") {
        std::cout << name << ": " << shared_value_no_sync;
    }
    else if (name == "С мьютексом") {
        std::cout << name << ": " << shared_value_mutex;
    }
    else {
        std::cout << name << ": " << shared_value_atomic;
    }

    std::cout << " | Время выполнения: " << duration.count() << " сек.\n";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    std::cout << "Тестирование различных подходов синхронизации с "
        << THREADS_COUNT << " потоками и "
        << ITERATIONS << " итерациями на каждый поток...\n\n";

    // Тестируем подход без синхронизации
    test_approach(increment_no_sync, "Без синхронизации");

    // Тестируем подход с мьютексом
    test_approach(increment_mutex, "С мьютексом");

    // Тестируем подход с атомарными операциями
    test_approach(increment_atomic, "Атомарные операции");

    return 0;
}
