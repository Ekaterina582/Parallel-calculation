#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>
#include <stdexcept>
#include <cmath>

using matrix = std::vector<std::vector<double>>;

// Генерация случайной матрицы размером r x c
matrix generate(int r, int c) {
    if (r <= 0 || c <= 0) {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    matrix result(r, std::vector<double>(c));
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            result[i][j] = dis(gen);
        }
    }
    return result;
}

// Проверка совместимости размеров матрицы и вектора
void check_dimensions(const matrix& a, const std::vector<double>& b) {
    if (a.empty() || b.empty()) {
        throw std::invalid_argument("Matrix or vector is empty");
    }
    size_t cols = a[0].size();
    for (const auto& row : a) {
        if (row.size() != cols) {
            throw std::invalid_argument("Matrix must be rectangular");
        }
    }
    if (cols != b.size()) {
        throw std::invalid_argument("Matrix columns must match vector size");
    }
}

// Последовательное умножение матрицы на вектор
std::vector<double> multiply(const matrix& a, const std::vector<double>& b) {
    check_dimensions(a, b);

    size_t rows = a.size();
    size_t cols = a[0].size();
    std::vector<double> result(rows, 0.0);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[i] += a[i][j] * b[j];
        }
    }

    return result;
}

// Параллельное умножение матрицы на вектор с использованием OpenMP
std::vector<double> multiply_parallel(const matrix& a, const std::vector<double>& b) {
    check_dimensions(a, b);

    size_t rows = a.size();
    size_t cols = a[0].size();
    std::vector<double> result(rows, 0.0);

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(rows); ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < cols; ++j) {
            sum += a[i][j] * b[j];
        }
        result[i] = sum;
    }

    return result;
}

// Сравнение производительности последовательной и параллельной версий
void compare(const matrix& a, const std::vector<double>& b) {
    try {
        // Последовательное умножение
        auto start_seq = std::chrono::high_resolution_clock::now();
        auto result_seq = multiply(a, b);
        auto end_seq = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> seq_time = end_seq - start_seq;

        // Параллельное умножение
        auto start_par = std::chrono::high_resolution_clock::now();
        auto result_par = multiply_parallel(a, b);
        auto end_par = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> par_time = end_par - start_par;

        // Проверка корректности результатов
        bool correct = true;
        for (size_t i = 0; i < result_seq.size(); ++i) {
            if (std::abs(result_seq[i] - result_par[i]) > 1e-6) {
                correct = false;
                break;
            }
        }

        // Вывод результатов
        std::cout << "Matrix size: " << a.size() << "x" << a[0].size() << "\n";
        std::cout << "Vector size: " << b.size() << "\n";
        std::cout << "Sequential time: " << seq_time.count() << " seconds\n";
        std::cout << "Parallel time: " << par_time.count() << " seconds\n";
        std::cout << "Speedup: " << seq_time.count() / par_time.count() << "x\n";
        std::cout << "Results are " << (correct ? "identical" : "different") << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error in comparison: " << e.what() << std::endl;
    }
}

int main() {
    try {
        const int size = 10000; // Размер матрицы и вектора

        // Генерация матрицы и вектора
        std::cout << "Generating matrix and vector..." << std::endl;
        matrix mat = generate(size, size);
        std::vector<double> vec(size, 1.0); // Простой вектор из единиц для теста

        // Сравнение производительности
        compare(mat, vec);

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}