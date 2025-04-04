#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <locale>
#include <windows.h>

// Функция для генерации случайной матрицы
std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);

    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }

    return matrix;
}

// Последовательное умножение матриц
std::vector<std::vector<double>> matrix_multiply_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {

    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();

    std::vector<std::vector<double>> C(rows_A, std::vector<double>(cols_B, 0.0));

    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            for (int k = 0; k < cols_A; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return C;
}

// Параллельное умножение матриц с OpenMP
std::vector<std::vector<double>> matrix_multiply_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {

    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();

    std::vector<std::vector<double>> C(rows_A, std::vector<double>(cols_B, 0.0));

#pragma omp parallel for shared(A, B, C)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
#pragma omp simd reduction(+:sum)
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    return C;
}

// Проверка корректности результатов (сравнение двух матриц)
bool verify_results(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B,
    double epsilon = 1e-6) {

    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }

    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (std::abs(A[i][j] - B[i][j]) > epsilon) {
                return false;
            }
        }
    }

    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    // Размеры матриц
    const int rows_A = 500;
    const int cols_A = 500;
    const int cols_B = 500;

    std::cout << "Генерация матриц..." << std::endl;
    auto A = generate_random_matrix(rows_A, cols_A);
    auto B = generate_random_matrix(cols_A, cols_B);

    // Последовательное умножение
    std::cout << "Последовательное умножение..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto C_seq = matrix_multiply_sequential(A, B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_time = end_seq - start_seq;

    // Параллельное умножение
    std::cout << "Параллельное умножение..." << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto C_par = matrix_multiply_parallel(A, B);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_time = end_par - start_par;

    // Проверка результатов
    std::cout << "Проверка корректности..." << std::endl;
    bool is_correct = verify_results(C_seq, C_par);

    // Вывод результатов
    std::cout << "\nРезультаты:" << std::endl;
    std::cout << "Последовательное время: " << std::fixed << std::setprecision(4)
        << seq_time.count() << " сек" << std::endl;
    std::cout << "Параллельное время:     " << std::fixed << std::setprecision(4)
        << par_time.count() << " сек" << std::endl;
    std::cout << "Ускорение: " << std::fixed << std::setprecision(2)
        << seq_time.count() / par_time.count() << "x" << std::endl;
    std::cout << "Результаты " << (is_correct ? "совпадают" : "не совпадают") << std::endl;

    return 0;
}