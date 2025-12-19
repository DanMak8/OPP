#include <iostream>
#include <functional>
#include <future>
#include <Windows.h>

const size_t ROWS = 10;
const size_t COLS = 10;
const size_t NTHREADS = 4;

bool startsWithEvenDigit(int num) {
    if (num == 0) return false;
    if (num < 0) num = -num;
    while (num >= 10) {
        num /= 10;
    }
    return (num % 2 == 0);
}

int findMin(int** matrix, size_t startRow, size_t endRow)
{
    int result = INT_MAX;
    for (size_t i{ startRow }; i < endRow; ++i) {
        for (size_t j{}; j < COLS; ++j) {
            if (startsWithEvenDigit(matrix[i][j])) {
                if (matrix[i][j] < result) {
                    result = matrix[i][j];
                }
            }
        }
    }
    return result;
}

// Параллельный поиск минимума
int findMinParallel(int** matrix)
{
    std::future<int> thr[NTHREADS - 1]{};
    size_t chunk{ ROWS / NTHREADS };

    // Запускаем асинхронные задачи
    for (size_t i{}; i < NTHREADS - 1; ++i)
    {
        thr[i] = std::async(std::launch::async, findMin, matrix, chunk * i, chunk * (i + 1));
    };

    int global_min{ findMin(matrix, chunk * (NTHREADS - 1), ROWS) };

    for (int i{}; i < NTHREADS - 1; ++i) {
        int thread_min = thr[i].get();
        if (thread_min < global_min) {
            global_min = thread_min;
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
    int** matrix = new int* [ROWS];
    for (size_t i{}; i < ROWS; ++i) {
        matrix[i] = new int[COLS];
    }

    srand(GetTickCount());
    initMatrix(matrix);


    printMatrix(matrix);
    std::cout << '\n';

    std::cout << findMinParallel(matrix) << '\n';

    for (size_t i{}; i < ROWS; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;

    system("pause");
    return 0;
}