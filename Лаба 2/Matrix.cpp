#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <locale>
#include <windows.h>

using namespace std;
using namespace std::chrono;

// Функция для создания подматрицы (исключает строку и столбец)
vector<vector<double>> getSubmatrix(const vector<vector<double>>& matrix, int excludeRow, int excludeCol) {
    int n = matrix.size();
    vector<vector<double>> submatrix(n - 1, vector<double>(n - 1));

    int row = 0;
    for (int i = 0; i < n; i++) {
        if (i == excludeRow) continue;

        int col = 0;
        for (int j = 0; j < n; j++) {
            if (j == excludeCol) continue;

            submatrix[row][col] = matrix[i][j];
            col++;
        }
        row++;
    }

    return submatrix;
}

// Последовательное вычисление определителя (рекурсивный метод)
double determinantSequential(const vector<vector<double>>& matrix) {
    int n = matrix.size();

    // Базовые случаи
    if (n == 1) return matrix[0][0];
    if (n == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;

    // Разложение по первой строке
    for (int col = 0; col < n; col++) {
        vector<vector<double>> submatrix = getSubmatrix(matrix, 0, col);
        det += matrix[0][col] * pow(-1, col) * determinantSequential(submatrix);
    }

    return det;
}

// Параллельное вычисление определителя (с использованием OpenMP)
double determinantParallel(const vector<vector<double>>& matrix) {
    int n = matrix.size();

    if (n == 1) return matrix[0][0];
    if (n == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;

#pragma omp parallel for reduction(+:det)
    for (int col = 0; col < n; col++) {
        vector<vector<double>> submatrix = getSubmatrix(matrix, 0, col);
        det += matrix[0][col] * pow(-1, col) * determinantSequential(submatrix);
    }

    return det;
}

// Функция для генерации случайной матрицы
vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 10; // значения от 0 до 9
        }
    }
    return matrix;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    int size;
    std::cout << "Введите размер квадратной матрицы: ";
    cin >> size;

    // Генерация матрицы
    vector<vector<double>> matrix = generateRandomMatrix(size);

    // Вывод матрицы (для небольших размеров)
    if (size <= 10) {
        std::cout << "Матрица:" << endl;
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << endl;
        }
    }

    // Последовательное вычисление
    auto start = high_resolution_clock::now();
    double detSeq = determinantSequential(matrix);
    auto stop = high_resolution_clock::now();
    auto durationSeq = duration_cast<milliseconds>(stop - start);

    std::cout << "Последовательный определитель: " << detSeq << endl;
    std::cout << "Время последовательного вычисления: " << durationSeq.count() << " мс" << endl;

    // Параллельное вычисление
    start = high_resolution_clock::now();
    double detPar = determinantParallel(matrix);
    stop = high_resolution_clock::now();
    auto durationPar = duration_cast<milliseconds>(stop - start);

    std::cout << "Параллельный определитель: " << detPar << endl;
    std::cout << "Время параллельного вычисления: " << durationPar.count() << " мс" << endl;

    // Вычисление ускорения
    double speedup = static_cast<double>(durationSeq.count()) / durationPar.count();
    std::cout << "Ускорение: " << speedup << "x" << endl;

    return 0;
}