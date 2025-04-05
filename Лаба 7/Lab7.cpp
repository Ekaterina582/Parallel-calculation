#include <iostream>
#include <cmath>
#include <chrono>
#include <cassert>
#include <omp.h>
#include <windows.h>
#include <locale>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Константа для точности вычислений
const double EPS = 1e-8;

// Функция, которую интегрируем (например, гауссовская функция e^(-x^2))
double func(double x) {
    return exp(-x * x);
}

// Аналитическое решение (нормированная функция ошибок)
double analytical_solution(double a, double b) {
    return sqrt(M_PI) / 2 * (erf(b) - erf(a));
}

// Последовательное интегрирование методом средних прямоугольников
double integrate(double a, double b, int n) {
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; ++i) {
        double x = a + (i + 0.5) * h;
        sum += func(x);
    }

    return sum * h;
}

// Параллельное интегрирование методом средних прямоугольников с OpenMP
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
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    // Пределы интегрирования
    double a = 0.0;
    double b = 1.0;

    // Количество интервалов (должно быть достаточно большим для точности)
    int n = 10000000;

    // Аналитическое решение для сравнения
    double analytical = analytical_solution(a, b);

    // Замер времени для последовательной версии
    auto start_seq = std::chrono::high_resolution_clock::now();
    double result_seq = integrate(a, b, n);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seq = end_seq - start_seq;

    // Замер времени для параллельной версии
    auto start_par = std::chrono::high_resolution_clock::now();
    double result_par = integrate_parallel(a, b, n);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_par = end_par - start_par;

    // Проверка корректности через assert
    assert(fabs(result_seq - result_par) < EPS);

    // Вывод результатов на русском языке
    std::cout << "Аналитическое решение: " << analytical << std::endl;
    std::cout << "Последовательный результат:  " << result_seq << std::endl;
    std::cout << "Параллельный результат:    " << result_par << std::endl;
    std::cout << "Абсолютная погрешность:     " << fabs(result_seq - analytical) << std::endl;
    std::cout << "Время последовательного вычисления:    " << elapsed_seq.count() << " с" << std::endl;
    std::cout << "Время параллельного вычисления:      " << elapsed_par.count() << " с" << std::endl;
    std::cout << "Ускорение:            " << elapsed_seq.count() / elapsed_par.count() << std::endl;

    return 0;
}
