#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <locale>
#include <windows.h>

using namespace std;
using namespace std::chrono;

// ������� ��� �������� ���������� (��������� ������ � �������)
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

// ���������������� ���������� ������������ (����������� �����)
double determinantSequential(const vector<vector<double>>& matrix) {
    int n = matrix.size();

    // ������� ������
    if (n == 1) return matrix[0][0];
    if (n == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;

    // ���������� �� ������ ������
    for (int col = 0; col < n; col++) {
        vector<vector<double>> submatrix = getSubmatrix(matrix, 0, col);
        det += matrix[0][col] * pow(-1, col) * determinantSequential(submatrix);
    }

    return det;
}

// ������������ ���������� ������������ (� �������������� OpenMP)
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

// ������� ��� ��������� ��������� �������
vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 10; // �������� �� 0 �� 9
        }
    }
    return matrix;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    int size;
    std::cout << "������� ������ ���������� �������: ";
    cin >> size;

    // ��������� �������
    vector<vector<double>> matrix = generateRandomMatrix(size);

    // ����� ������� (��� ��������� ��������)
    if (size <= 10) {
        std::cout << "�������:" << endl;
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << endl;
        }
    }

    // ���������������� ����������
    auto start = high_resolution_clock::now();
    double detSeq = determinantSequential(matrix);
    auto stop = high_resolution_clock::now();
    auto durationSeq = duration_cast<milliseconds>(stop - start);

    std::cout << "���������������� ������������: " << detSeq << endl;
    std::cout << "����� ����������������� ����������: " << durationSeq.count() << " ��" << endl;

    // ������������ ����������
    start = high_resolution_clock::now();
    double detPar = determinantParallel(matrix);
    stop = high_resolution_clock::now();
    auto durationPar = duration_cast<milliseconds>(stop - start);

    std::cout << "������������ ������������: " << detPar << endl;
    std::cout << "����� ������������� ����������: " << durationPar.count() << " ��" << endl;

    // ���������� ���������
    double speedup = static_cast<double>(durationSeq.count()) / durationPar.count();
    std::cout << "���������: " << speedup << "x" << endl;

    return 0;
}