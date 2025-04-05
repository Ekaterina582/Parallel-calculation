#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <windows.h>
#include <locale>

// Константы программы
#define MATRIX_SIZE 500          // Размер квадратных матриц (N x N)
#define MIN_RAND_VALUE 1         // Минимальное значение элементов матрицы
#define MAX_RAND_VALUE 10        // Максимальное значение элементов матрицы

/**
 * Заполняет матрицу случайными числами в заданном диапазоне
 * @param matrix Указатель на матрицу
 * @param rows Количество строк
 * @param cols Количество столбцов
 */
void fill_matrix(int* matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Генерация случайного числа в диапазоне [MIN_RAND_VALUE, MAX_RAND_VALUE]
            matrix[i * cols + j] = MIN_RAND_VALUE + rand() % (MAX_RAND_VALUE - MIN_RAND_VALUE + 1);
        }
    }
}

/**
 * Выводит часть матрицы (срез) заданного размера
 * @param matrix Указатель на матрицу
 * @param rows Количество строк в матрице
 * @param cols Количество столбцов в матрице
 * @param slice_size Размер среза (например, 5 для вывода 5x5 элементов)
 */
void print_matrix_slice(int* matrix, int rows, int cols, int slice_size) {
    for (int i = 0; i < slice_size && i < rows; i++) {
        for (int j = 0; j < slice_size && j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    int rank, size;             // Ранг процесса и общее количество процессов
    double start_time, end_time; // Переменные для измерения времени выполнения

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получаем ранг текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получаем общее количество процессов

    // Проверка, что размер матрицы делится на количество процессов без остатка
    if (MATRIX_SIZE % size != 0) {
        if (rank == 0) { // Выводим сообщение только из главного процесса
            printf("Ошибка: размер матрицы (%d) должен делиться на количество процессов (%d) без остатка.\n",
                MATRIX_SIZE, size);
        }
        MPI_Finalize();
        return 1;
    }

    // Вычисляем количество строк матрицы A, обрабатываемых каждым процессом
    int rows_per_process = MATRIX_SIZE / size;

    // Объявление указателей на матрицы
    int* A = NULL;    // Исходная матрица A (только в процессе 0)
    int* B = NULL;    // Исходная матрица B (только в процессе 0)
    int* C = NULL;    // Результирующая матрица C (только в процессе 0)

    // Выделяем память для локальных частей матриц в каждом процессе
    int* local_A = (int*)malloc(rows_per_process * MATRIX_SIZE * sizeof(int)); // Локальная часть матрицы A
    int* local_C = (int*)malloc(rows_per_process * MATRIX_SIZE * sizeof(int)); // Локальная часть матрицы C

    // Процесс с рангом 0 инициализирует исходные матрицы A и B
    if (rank == 0) {
        // Выделяем память для полных матриц
        A = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
        B = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
        C = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));

        // Инициализация генератора случайных чисел
        srand(time(NULL));

        // Заполнение матриц случайными значениями
        fill_matrix(A, MATRIX_SIZE, MATRIX_SIZE);
        fill_matrix(B, MATRIX_SIZE, MATRIX_SIZE);

        // Засекаем время начала вычислений
        start_time = MPI_Wtime();
    }

    // Распределение данных между процессами:

    // 1. Разделяем матрицу A по строкам между процессами
    // MPI_Scatter разделяет массив на равные части и рассылает их всем процессам
    MPI_Scatter(A,                       // Исходный буфер (только в корневом процессе)
        rows_per_process * MATRIX_SIZE, // Количество элементов для каждого процесса
        MPI_INT,                 // Тип данных
        local_A,                 // Приемный буфер в каждом процессе
        rows_per_process * MATRIX_SIZE, // Количество получаемых элементов
        MPI_INT,                 // Тип данных
        0,                       // Ранг корневого процесса
        MPI_COMM_WORLD);         // Коммуникатор

    // 2. Рассылаем полную матрицу B всем процессам
    // MPI_Bcast рассылает данные из корневого процесса всем процессам
    MPI_Bcast(B,                        // Буфер с данными
        MATRIX_SIZE * MATRIX_SIZE, // Количество элементов
        MPI_INT,                  // Тип данных
        0,                        // Ранг корневого процесса
        MPI_COMM_WORLD);          // Коммуникатор

    // Параллельное умножение матриц:
    // Каждый процесс умножает свою часть матрицы A на матрицу B
    for (int i = 0; i < rows_per_process; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            local_C[i * MATRIX_SIZE + j] = 0; // Инициализация элемента результата

            // Вычисление скалярного произведения строки и столбца
            for (int k = 0; k < MATRIX_SIZE; k++) {
                local_C[i * MATRIX_SIZE + j] += local_A[i * MATRIX_SIZE + k] * B[k * MATRIX_SIZE + j];
            }
        }
    }

    // Сбор результатов в процессе 0:
    // MPI_Gather собирает части матрицы C со всех процессов в один массив
    MPI_Gather(local_C,                 // Отправляемые данные (локальная часть C)
        rows_per_process * MATRIX_SIZE, // Количество элементов
        MPI_INT,                 // Тип данных
        C,                       // Приемный буфер (только в корневом процессе)
        rows_per_process * MATRIX_SIZE, // Количество элементов от каждого процесса
        MPI_INT,                 // Тип данных
        0,                       // Ранг корневого процесса
        MPI_COMM_WORLD);         // Коммуникатор

    // Процесс 0 выводит результаты
    if (rank == 0) {
        // Засекаем время окончания вычислений
        end_time = MPI_Wtime();

        // Вывод результатов:
        printf("\n=== Результаты перемножения матриц ===\n");

        // 1. Выводим срез 5x5 результирующей матрицы для проверки
        printf("\nПервые 5x5 элементов результирующей матрицы:\n");
        print_matrix_slice(C, MATRIX_SIZE, MATRIX_SIZE, 5);

        // 2. Выводим параметры вычислений
        printf("\nПараметры выполнения:\n");
        printf("Размер матрицы: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
        printf("Количество процессов: %d\n", size);
        printf("Время выполнения: %.3f сек\n", end_time - start_time);

        // Освобождаем память, выделенную в процессе 0
        free(A);
        free(B);
        free(C);
    }

    // Освобождаем память, выделенную в каждом процессе
    free(local_A);
    free(local_C);

    // Завершение работы с MPI
    MPI_Finalize();
    return 0;
}