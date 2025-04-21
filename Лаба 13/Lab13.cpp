#include <opencv2/opencv.hpp>
#include <complex>
#include <vector>

using namespace cv;
using namespace std;

// Функция для вычисления количества итераций для точки
int mandelbrot(const complex<double>& c, int max_iter) {
    complex<double> z = 0;
    int n = 0;
    while (abs(z) <= 2 && n < max_iter) {
        z = z * z + c;
        n++;
    }
    return n;
}

// Функция для преобразования значения итерации в цвет
Vec3b get_color(int iterations, int max_iter) {
    if (iterations == max_iter) {
        return Vec3b(0, 0, 0);  // Черный цвет для точек множества
    }

    // Градиент от белого к синему
    float ratio = static_cast<float>(iterations) / max_iter;
    return Vec3b(
        255 * (1 - ratio),      // Красный канал (уменьшается)
        255 * (1 - ratio),      // Зеленый канал (уменьшается)
        255                     // Синий канал (максимальный)
    );
}

int main() {
    const int width = 800;
    const int height = 600;
    const int max_iter = 100;

    // Параметры отображения фрактала
    double x_min = -2.5;
    double x_max = 1.0;
    double y_min = -1.0;
    double y_max = 1.0;

    // Создаем изображение в формате BGR
    Mat mandelbrot_img(height, width, CV_8UC3, Scalar(255, 255, 255));

    // Генерация фрактала
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Преобразование координат пикселя в комплексную плоскость
            double x0 = x_min + (x_max - x_min) * x / width;
            double y0 = y_min + (y_max - y_min) * y / height;
            complex<double> c(x0, y0);

            // Вычисление количества итераций
            int iterations = mandelbrot(c, max_iter);

            // Установка цвета пикселя
            mandelbrot_img.at<Vec3b>(y, x) = get_color(iterations, max_iter);
        }
    }

    // Отображение изображения
    imshow("Фрактал Мандельброта", mandelbrot_img);

    // Сохранение изображения
    imwrite("mandelbrot_blue.png", mandelbrot_img);

    // Ожидание нажатия клавиши
    waitKey(0);

    return 0;
}