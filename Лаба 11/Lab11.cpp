#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <locale.h>
#include <limits.h> // Для LLONG_MAX

// Функция для генерации случайного массива
void generate_random_array(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

// Функция для последовательного вычисления суммы
long long sequential_sum(int* array, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        if (sum > LLONG_MAX - array[i]) {
            fprintf(stderr, "Ошибка: переполнение суммы!\n");
            return 0;
        }
        sum += array[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "rus");

    int rank, size;
    int* global_array = NULL;
    int* local_array = NULL;
    long long global_sum = 0;
    long long local_sum = 0;
    double seq_time = 0.0, par_time = 0.0;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int array_size = 1000000;
    int base_size = array_size / size;
    int remainder = array_size % size;
    int local_size = base_size + (rank < remainder ? 1 : 0);

    if (rank == 0) {
        global_array = (int*)malloc(array_size * sizeof(int));
        if (!global_array) {
            fprintf(stderr, "Ошибка выделения памяти для global_array\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand((unsigned int)time(NULL));
        generate_random_array(global_array, array_size);

        // Замер времени последовательного выполнения
        start_time = MPI_Wtime();
        long long seq_sum = sequential_sum(global_array, array_size);
        end_time = MPI_Wtime();
        seq_time = end_time - start_time;
        printf("Последовательное вычисление: сумма = %lld (время: %.6f сек)\n",
            seq_sum, seq_time);
    }

    local_array = (int*)malloc(local_size * sizeof(int));
    if (!local_array) {
        fprintf(stderr, "Ошибка выделения памяти в процессе %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(global_array, local_size, MPI_INT,
        local_array, local_size, MPI_INT,
        0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    local_sum = 0;
    for (int i = 0; i < local_size; i++) {
        if (local_sum > LLONG_MAX - local_array[i]) {
            fprintf(stderr, "Ошибка: переполнение суммы в процессе %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        local_sum += local_array[i];
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    par_time = end_time - start_time;

    if (rank == 0) {
        printf("Параллельное вычисление: сумма = %lld (время: %.6f сек)\n",
            global_sum, par_time);
        printf("Ускорение: %.2f раз\n", seq_time / par_time);
        free(global_array);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}