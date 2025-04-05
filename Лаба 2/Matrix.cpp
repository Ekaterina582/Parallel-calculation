#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <locale>
#include <windows.h>
#include <cstdlib> // для srand

using namespace std;
using namespace std::chrono;

/*
 * Создает подматрицу, исключая указанные строку и столбец
 * matrix - исходная матрица
 * excludeRow - исключаемая строка
 * excludeCol - исключаемый столбец
 */
vector<vector<double>> getSubmatrix(const vector<vector<double>>& matrix, int excludeRow, int excludeCol) {
    int n = matrix.size() - 1;
    vector<vector<double>> submatrix(n, vector<double>(n));

    for (int i = 0, row = 0; row < n; i++, row++) {
        if (i == excludeRow) i++;
        for (int j = 0, col = 0; col < n; j++, col++) {
            if (j == excludeCol) j++;
            submatrix[row][col] = matrix[i][j];
        }
    }
    return submatrix;
}

/*
 * Последовательное вычисление определителя (рекурсивный метод)
 */
double determinantSequential(const vector<vector<double>>& matrix) {
    int n = matrix.size();
    if (n == 1) return matrix[0][0];
    if (n == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;
    for (int col = 0; col < n; col++) {
        auto submatrix = getSubmatrix(matrix, 0, col);
        det += matrix[0][col] * ((col % 2 == 0) ? 1 : -1) * determinantSequential(submatrix);
    }
    return det;
}

/*
 * Параллельное вычисление определителя с использованием OpenMP
 */
double determinantParallel(const vector<vector<double>>& matrix) {
    int n = matrix.size();
    if (n == 1) return matrix[0][0];
    if (n == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;
#pragma omp parallel for reduction(+:det)
    for (int col = 0; col < n; col++) {
        auto submatrix = getSubmatrix(matrix, 0, col);
        det += matrix[0][col] * ((col % 2 == 0) ? 1 : -1) * determinantSequential(submatrix);
    }
    return det;
}

/*
 * Генерация случайной квадратной матрицы заданного размера
 * size - размер матрицы
 */
vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    srand(static_cast<unsigned>(time(nullptr)));
    for (auto& row : matrix) {
        for (auto& elem : row) {
            elem = rand() % 10;
        }
    }
    return matrix;
}

int main() {
    // Настройка консоли для корректного отображения кириллицы
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    int size;
    cout << "Введите размер квадратной матрицы: ";
    cin >> size;

    auto matrix = generateRandomMatrix(size);

    // Вывод матрицы (для размеров <= 10)
    if (size <= 10) {
        cout << "\nСгенерированная матрица:\n";
        for (const auto& row : matrix) {
            for (double val : row) {
                cout << val << " ";
            }
            cout << "\n";
        }
    }

    // Последовательное вычисление
    auto start = high_resolution_clock::now();
    double detSeq = determinantSequential(matrix);
    auto durationSeq = duration_cast<milliseconds>(high_resolution_clock::now() - start);

    // Параллельное вычисление
    start = high_resolution_clock::now();
    double detPar = determinantParallel(matrix);
    auto durationPar = duration_cast<milliseconds>(high_resolution_clock::now() - start);

    // Вывод результатов
    cout << "\nРезультаты:\n";
    cout << "Последовательный определитель: " << detSeq << "\n";
    cout << "Параллельный определитель: " << detPar << "\n";
    cout << "Время последовательного вычисления: " << durationSeq.count() << " мс\n";
    cout << "Время параллельного вычисления: " << durationPar.count() << " мс\n";
    cout << "Ускорение: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x\n";

    return 0;
}
