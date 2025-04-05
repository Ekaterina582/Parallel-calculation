#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <omp.h>
#include <windows.h>
#include <locale>

/*
 * Класс для реализации игры "Жизнь" Конвея
 * с параллельными вычислениями через OpenMP
 */
class GameOfLife {
public:
    // Конструктор (размер поля и количество потоков)
    GameOfLife(int width, int height, int num_threads = 1)
        : width(width), height(height), generation(0), num_threads(num_threads) {
        current_grid.resize(height, std::vector<bool>(width, false));
        next_grid.resize(height, std::vector<bool>(width, false));
    }

    // Инициализация поля случайным образом
    // alive_prob - вероятность появления живой клетки (0.0-1.0)
    void random_init(double alive_prob = 0.3) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution dist(alive_prob);

#pragma omp parallel for num_threads(num_threads) collapse(2)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                current_grid[y][x] = dist(gen);
            }
        }
        generation = 0;
    }

    /*
     * Инициализация фигурой "Глайдер"
     * start_x, start_y - начальная позиция левого верхнего угла фигуры
     *
     * Глайдер - это фигура из 5 клеток, которая перемещается по диагонали
     * Форма глайдера:
     *   . # .
     *   . . #
     *   # # #
     */
    void pattern_init(int start_x, int start_y) {
        clear();
        // Создаем глайдер
        current_grid[start_y][start_x + 1] = true;
        current_grid[start_y + 1][start_x + 2] = true;
        current_grid[start_y + 2][start_x] = true;
        current_grid[start_y + 2][start_x + 1] = true;
        current_grid[start_y + 2][start_x + 2] = true;
        generation = 0;
    }

    // Очистка поля (все клетки становятся мертвыми)
    void clear() {
#pragma omp parallel for num_threads(num_threads) collapse(2)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                current_grid[y][x] = false;
            }
        }
        generation = 0;
    }

    // Выполнение одного шага эволюции (переход к следующему поколению)
    void step() {
#pragma omp parallel for num_threads(num_threads) collapse(2)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int neighbors = count_neighbors(x, y);

                // Правила игры:
                // 1. Живая клетка выживает с 2-3 соседями, иначе умирает
                // 2. Мертвая клетка оживает с ровно 3 соседями
                next_grid[y][x] = current_grid[y][x]
                    ? (neighbors == 2 || neighbors == 3)
                        : (neighbors == 3);
            }
        }

        // Обмен текущего и следующего поколения
        std::swap(current_grid, next_grid);
        generation++;
    }

    // Подсчет количества живых соседей для клетки (x,y)
    int count_neighbors(int x, int y) const {
        int count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue; // Пропускаем саму клетку

                // Тороидальная геометрия (границы соединены)
                int nx = (x + dx + width) % width;
                int ny = (y + dy + height) % height;

                if (current_grid[ny][nx]) {
                    count++;
                }
            }
        }
        return count;
    }

    // Отображение текущего состояния поля
    void render() const {
        system("clear"); // Для Linux/Mac. Для Windows: system("cls");

        std::cout << "Поколение: " << generation
            << " | Живых клеток: " << count_alive()
            << " | Потоков: " << num_threads << std::endl;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << (current_grid[y][x] ? "#" : ".");
            }
            std::cout << std::endl;
        }
    }

    // Подсчет количества живых клеток на поле
    int count_alive() const {
        int count = 0;
#pragma omp parallel for num_threads(num_threads) reduction(+:count) collapse(2)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (current_grid[y][x]) {
                    count++;
                }
            }
        }
        return count;
    }

private:
    int width;  // Ширина поля
    int height; // Высота поля
    int generation; // Номер текущего поколения
    int num_threads; // Количество потоков для OpenMP

    // Два поля для текущего и следующего поколения
    std::vector<std::vector<bool>> current_grid;
    std::vector<std::vector<bool>> next_grid;
};

/*
 * Основная функция программы
 * Демонстрирует работу игры "Жизнь" с выбором начальной конфигурации
 */
int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    const int width = 60;       // Ширина поля
    const int height = 30;      // Высота поля
    const int max_generations = 100; // Максимальное число поколений
    const int delay_ms = 100;   // Задержка между поколениями (мс)
    const int num_threads = 4;  // Количество потоков OpenMP

    GameOfLife game(width, height, num_threads);

    // Выбор типа инициализации
    std::cout << "Выберите тип инициализации:\n"
        << "1. Случайная\n"
        << "2. Фигура (глайдер)\n"
        << "Ваш выбор: ";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
        game.random_init(0.3); // 30% вероятность живой клетки
        std::cout << "Инициализация случайным образом (30% клеток живые)" << std::endl;
    }
    else {
        game.pattern_init(1, 1); // Глайдер в позиции (1,1)
        std::cout << "Инициализация фигурой (глайдер)" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Основной цикл игры
    for (int i = 0; i < max_generations; ++i) {
        game.render();
        game.step();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << "\nСимуляция завершена после " << max_generations << " поколений." << std::endl;
    return 0;
}