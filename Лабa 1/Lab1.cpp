#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <cassert>
#include <windows.h>
#include <locale>

// Последовательное вычисление факториала
unsigned long long sequential_factorial(int n) {
    if (n < 0) return 0;
    unsigned long long result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// Параллельное вычисление факториала
unsigned long long parallel_factorial(int n, int num_threads) {
    if (n < 0) return 0;
    if (n < 2) return 1;

    unsigned long long result = 1;
    std::mutex result_mutex;
    std::vector<std::thread> threads;

    int chunk_size = n / num_threads;
    int remainder = n % num_threads;

    auto worker = [&result, &result_mutex](int start, int end) {
        unsigned long long partial = 1;
        for (int i = start; i <= end; ++i) {
            partial *= i;
        }

        std::lock_guard<std::mutex> lock(result_mutex);
        result *= partial;
        };

    int start = 1;
    for (int i = 0; i < num_threads; ++i) {
        int end = start + chunk_size - 1;
        if (i < remainder) {
            end += 1;
        }
        if (end > n) end = n;

        threads.emplace_back(worker, start, end);
        start = end + 1;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    const int n = 20; // Число для вычисления факториала
    const int num_threads = std::thread::hardware_concurrency(); // Количество потоков

    // Последовательное вычисление
    auto start_seq = std::chrono::high_resolution_clock::now();
    unsigned long long seq_result = sequential_factorial(n);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_duration = end_seq - start_seq;

    // Параллельное вычисление
    auto start_par = std::chrono::high_resolution_clock::now();
    unsigned long long par_result = parallel_factorial(n, num_threads);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_duration = end_par - start_par;

    // Вывод результатов
    std::cout << "Факториал числа " << n << ":\n";
    std::cout << "Последовательный результат: " << seq_result << "\n";
    std::cout << "Параллельный результат:    " << par_result << "\n\n";

    std::cout << "Сравнение времени выполнения:\n";
    std::cout << "Последовательный: " << seq_duration.count() << " секунд\n";
    std::cout << "Параллельный:     " << par_duration.count() << " секунд (" << num_threads << " потоков)\n\n";

    // Проверка корректности
    if (seq_result == par_result) {
        std::cout << "Результаты совпадают!\n";
    }
    else {
        std::cout << "Ошибка: результаты не совпадают!\n";
    }

    // Вычисление ускорения
    double speedup = seq_duration.count() / par_duration.count();
    std::cout << "Ускорение: " << speedup << "x\n";

    return 0;
}
