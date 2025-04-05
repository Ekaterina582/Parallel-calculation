#include <opencv2/opencv.hpp>
#include <omp.h>
#include <iostream>
#include <cmath>
#include <string>
#include <windows.h>
#include <locale>

using namespace cv;
using namespace std;

void drawSierpinskiCarpet(Mat& image, int x, int y, int size, int depth, int max_depth) {
    if (depth >= max_depth || size < 1) {
        rectangle(image, Point(x, y), Point(x + size, y + size), Scalar(0), FILLED);
        return;
    }

    int new_size = size / 3;

    // Закрашиваем центральный квадрат
    rectangle(image, Point(x + new_size, y + new_size),
        Point(x + 2 * new_size, y + 2 * new_size), Scalar(255), FILLED);

#pragma omp parallel for collapse(2)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i != 1 || j != 1) {  // Пропускаем центральный квадрат
                drawSierpinskiCarpet(image, x + i * new_size, y + j * new_size,
                    new_size, depth + 1, max_depth);
            }
        }
    }
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    // Параметры по умолчанию
    int size = 729;        // 3^6
    int depth = 5;
    int threads = omp_get_max_threads();
    string window_name = "Ковер Серпинского";

    // Обработка аргументов командной строки
    if (argc > 1) size = stoi(argv[1]);
    if (argc > 2) depth = stoi(argv[2]);
    if (argc > 3) threads = stoi(argv[3]);

    // Проверка, что размер является степенью 3
    int check = size;
    while (check % 3 == 0) check /= 3;
    if (check != 1) {
        cerr << "Размер должен быть степенью 3 (3^n)" << endl;
        return -1;
    }

    // Установка количества потоков
    omp_set_num_threads(threads);

    // Создание белого изображения
    Mat image(size, size, CV_8UC1, Scalar(255));

    // Замер времени выполнения
    double start = omp_get_wtime();

    // Рисование ковра Серпинского
    drawSierpinskiCarpet(image, 0, 0, size, 0, depth);

    double end = omp_get_wtime();
    cout << "Время выполнения: " << end - start << " секунд" << endl;

    // Создание окна и отображение изображения
    namedWindow(window_name, WINDOW_AUTOSIZE);
    imshow(window_name, image);

    // Сообщение для пользователя
    cout << "Изображение отображается в графическом окне." << endl;
    cout << "Нажмите любую клавишу для выхода..." << endl;

    // Ожидание нажатия клавиши
    waitKey(0);

    // Закрытие окна
    destroyAllWindows();

    return 0;
}