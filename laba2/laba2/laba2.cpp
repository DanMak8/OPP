#include <iostream>
#include <Windows.h>
#include <thread>
#include <mutex>

const size_t ROWS = 10;
const size_t COLS = 10;
const size_t NTHREADS = 4;

std::mutex cout_mutex;

bool startsWithEvenDigit(int num) {
    if (num == 0) return false;
    if (num < 0) num = -num;
    while (num >= 10) {
        num /= 10;
    }
    return (num % 2 == 0);
}

void findMin(int** matrix, size_t startRow, size_t endRow, int& result)
{
    result = INT_MAX;

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "поток " << std::this_thread::get_id()
            << " проверяет строки " << startRow
            << " - " << endRow - 1 << '\n';
    }

    for (size_t i{ startRow }; i < endRow; ++i) {
        for (size_t j{}; j < COLS; ++j) {
            if (startsWithEvenDigit(matrix[i][j])) {
                if (matrix[i][j] < result) {
                    result = matrix[i][j];
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        if (result == INT_MAX) {
            std::cout << "поток " << std::this_thread::get_id()
                << " не нашел подходящих чисел\n";
        }
        else {
            std::cout << "поток " << std::this_thread::get_id()
                << " получил " << result << '\n';
        }
    }
}

int findMinParallel(int** matrix)
{
    std::thread thr[NTHREADS - 1]{};
    int min_thread[NTHREADS - 1]{};
    size_t chunk{ ROWS / NTHREADS };

    std::cout << "Главный поток " << std::this_thread::get_id()
        << " обрабатывает строки " << chunk * (NTHREADS - 1)
        << " - " << ROWS - 1 << "\n";

    for (size_t i{}; i < NTHREADS - 1; ++i) {
        thr[i] = std::thread(findMin, matrix, chunk * i, chunk * (i + 1),
            std::ref(min_thread[i]));
    }

    int global_min{};
    findMin(matrix, chunk * (NTHREADS - 1), ROWS, global_min);

    for (size_t i{}; i < NTHREADS - 1; ++i) {
        thr[i].join();
        if (min_thread[i] < global_min) {
            global_min = min_thread[i];
        }
    }

    return global_min;

}

void initMatrix(int** matrix)
{
    for (size_t i{}; i < ROWS; ++i) {
        for (size_t j{}; j < COLS; ++j) {
            matrix[i][j] = rand() % 2000 - 1000;
        }
    }
}

void printMatrix(int** matrix)
{
    for (size_t i{}; i < ROWS; ++i) {
        for (size_t j{}; j < COLS; ++j) {
            std::cout << matrix[i][j] << '\t';
        }
        std::cout << '\n';
    }
}


int main()
{
    setlocale(LC_ALL, "RUS");
    int** matrix = new int* [ROWS];
    for (size_t i{}; i < ROWS; ++i) {
        matrix[i] = new int[COLS];
    }

    initMatrix(matrix);

    printMatrix(matrix);
    std::cout << '\n';

    std::cout << "ПОСЛЕДОВАТЕЛЬНЫЙ ПОИСК\n";
    int min_nonparallel{};
    findMin(matrix, 0, ROWS, min_nonparallel);
    std::cout << "Минимум: " << min_nonparallel << "\n\n";

    std::cout << "ПАРАЛЛЕЛЬНЫЙ ПОИСК\n";
    int result = findMinParallel(matrix);
    std::cout << "Глобальный минимум: " << result << '\n';

    for (size_t i{}; i < ROWS; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;

    std::cin.get();
    return 0;
}
