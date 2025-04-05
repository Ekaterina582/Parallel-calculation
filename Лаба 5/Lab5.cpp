#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <omp.h>

// Последовательная версия четно-нечетной сортировки
void oddEvenSortSequential(std::vector<int>& arr) {
    bool isSorted = false;
    int n = arr.size();

    while (!isSorted) {
        isSorted = true;

        // Нечетная фаза (нечетные индексы)
        for (int i = 1; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }

        // Четная фаза (четные индексы)
        for (int i = 0; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

// Параллельная версия четно-нечетной сортировки с OpenMP
void oddEvenSortParallel(std::vector<int>& arr) {
    bool isSorted = false;
    int n = arr.size();

    while (!isSorted) {
        isSorted = true;

        // Нечетная фаза (нечетные индексы)
#pragma omp parallel for shared(arr, isSorted)
        for (int i = 1; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
#pragma omp critical
                {
                    std::swap(arr[i], arr[i + 1]);
                    isSorted = false;
                }
            }
        }

        // Четная фаза (четные индексы)
#pragma omp parallel for shared(arr, isSorted)
        for (int i = 0; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
#pragma omp critical
                {
                    std::swap(arr[i], arr[i + 1]);
                    isSorted = false;
                }
            }
        }
    }
}

// Генерация случайного массива
std::vector<int> generateRandomArray(int size) {
    std::vector<int> arr(size);
    for (int i = 0; i < size; ++i) {
        arr[i] = rand() % 10000;
    }
    return arr;
}

// Проверка отсортированности массива
bool isSorted(const std::vector<int>& arr) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}

int main() {
    // Для корректного отображения русского языка в консоли Windows
    setlocale(LC_ALL, "Russian");

    const int arraySize = 10000;
    std::vector<int> arrSequential = generateRandomArray(arraySize);
    std::vector<int> arrParallel = arrSequential;

    // Замер времени для последовательной версии
    auto start = std::chrono::high_resolution_clock::now();
    oddEvenSortSequential(arrSequential);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequentialTime = end - start;

    // Замер времени для параллельной версии
    start = std::chrono::high_resolution_clock::now();
    oddEvenSortParallel(arrParallel);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelTime = end - start;

    // Проверка результатов
    bool sequentialCorrect = isSorted(arrSequential);
    bool parallelCorrect = isSorted(arrParallel);

    // Вывод результатов на русском языке
    std::cout << "Последовательная версия: " << sequentialTime.count() << " секунд, "
        << (sequentialCorrect ? "корректно" : "некорректно") << std::endl;
    std::cout << "Параллельная версия: " << parallelTime.count() << " секунд, "
        << (parallelCorrect ? "корректно" : "некорректно") << std::endl;
    std::cout << "Ускорение: " << sequentialTime.count() / parallelTime.count() << "x" << std::endl;

    return 0;
}
