#include <iostream>
#include <cmath>
#include <chrono>
#include <cassert>
#include <omp.h>

// ��������� ��� �������� ����������
const double EPS = 1e-8;
const double M_PI = 3.14159265358979323846;

// �������, ������� ����������� (��������, ����������� ������� e^(-x^2))
double func(double x) {
    return exp(-x * x);
}

// ������������� ������� (������������� ������� ������)
double analytical_solution(double a, double b) {
    return sqrt(M_PI) / 2 * (erf(b) - erf(a));
}

// ���������������� �������������� ������� ������� ���������������
double integrate(double a, double b, int n) {
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; ++i) {
        double x = a + (i + 0.5) * h;
        sum += func(x);
    }

    return sum * h;
}

// ������������ �������������� ������� ������� ��������������� � OpenMP
double integrate_parallel(double a, double b, int n) {
    double h = (b - a) / n;
    double sum = 0.0;

#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        double x = a + (i + 0.5) * h;
        sum += func(x);
    }

    return sum * h;
}

int main() {
    // ������� ��������������
    double a = 0.0;
    double b = 1.0;

    // ���������� ���������� (������ ���� ���������� ������� ��� ��������)
    int n = 10000000;

    // ������������� ������� ��� ���������
    double analytical = analytical_solution(a, b);

    // ����� ������� ��� ���������������� ������
    auto start_seq = std::chrono::high_resolution_clock::now();
    double result_seq = integrate(a, b, n);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seq = end_seq - start_seq;

    // ����� ������� ��� ������������ ������
    auto start_par = std::chrono::high_resolution_clock::now();
    double result_par = integrate_parallel(a, b, n);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_par = end_par - start_par;

    // �������� ������������ ����� assert
    assert(fabs(result_seq - result_par) < EPS);

    // ����� �����������
    std::cout << "Analytical solution: " << analytical << std::endl;
    std::cout << "Sequential result:  " << result_seq << std::endl;
    std::cout << "Parallel result:    " << result_par << std::endl;
    std::cout << "Absolute error:     " << fabs(result_seq - analytical) << std::endl;
    std::cout << "Sequential time:    " << elapsed_seq.count() << " s" << std::endl;
    std::cout << "Parallel time:      " << elapsed_par.count() << " s" << std::endl;
    std::cout << "Speedup:            " << elapsed_seq.count() / elapsed_par.count() << std::endl;

    return 0;
}