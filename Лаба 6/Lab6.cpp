#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

// ������� ��� ��������� ������� ��������� �����
std::vector<int> generate_random_array(size_t size, int min_val, int max_val) {
    std::vector<int> array(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min_val, max_val);

    for (size_t i = 0; i < size; ++i) {
        array[i] = dist(gen);
    }

    return array;
}

// ���������������� ���������� �����
long long sum(const std::vector<int>& array) {
    long long total = 0;
    for (int num : array) {
        total += num;
    }
    return total;
}

// ������������ ���������� ����� � �������������� OpenMP
long long sum_parallel(const std::vector<int>& array) {
    long long total = 0;

#pragma omp parallel for reduction(+:total)
    for (size_t i = 0; i < array.size(); ++i) {
        total += array[i];
    }

    return total;
}

int main() {
    // ������ ������� ����� �������� ��� ������������
    const size_t array_size = 100000000;
    const int min_val = 1;
    const int max_val = 100;

    std::cout << "��������� ������� �� " << array_size << " ���������..." << std::endl;
    std::vector<int> numbers = generate_random_array(array_size, min_val, max_val);

    // ���������������� ����������
    std::cout << "���������������� ���������� �����..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    long long seq_sum = sum(numbers);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_time = end - start;

    std::cout << "����� (���������������): " << seq_sum << std::endl;
    std::cout << "����� ����������: " << seq_time.count() << " ������" << std::endl;

    // ������������ ����������
    std::cout << "\n������������ ���������� �����..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    long long par_sum = sum_parallel(numbers);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_time = end - start;

    std::cout << "����� (�����������): " << par_sum << std::endl;
    std::cout << "����� ����������: " << par_time.count() << " ������" << std::endl;

    // ��������� �����������
    std::cout << "\n���������� ���������:" << std::endl;
    std::cout << "������� � ������: " << std::abs(seq_sum - par_sum) << std::endl;
    std::cout << "���������: " << seq_time.count() / par_time.count() << "x" << std::endl;

    return 0;
}