#include <iostream>
#include <Windows.h>
#include <process.h>

const int ROWS = 10;
const int COLS = 10;
const int NTHREADS = 4;

struct INFORM {
    int** matrix;
    int startRow, endRow;
    int minValue;
    int threadId;
};


bool startsWithEvenDigit(int num) {
    if (num == 0) return false;
    if (num < 0) num = -num;
    while (num >= 10) {
        num /= 10;
    }
    return (num % 2 == 0);
}


unsigned __stdcall findMin(void* arg) {
    INFORM* inform = (INFORM*)arg;
    inform->minValue = INT_MAX;

    std::cout << "Поток " << inform->threadId << " проверяет строки с "
        << inform->startRow << " по " << inform->endRow - 1 << std::endl;

    for (int i = inform->startRow; i < inform->endRow; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (startsWithEvenDigit(inform->matrix[i][j])) {
                if (inform->matrix[i][j] < inform->minValue) {
                    inform->minValue = inform->matrix[i][j];
                }
            }
        }
    }

    std::cout << "Поток " << inform->threadId << " завершил работу, min = ";
    if (inform->minValue == INT_MAX) {
        std::cout << "не найдено";
    }
    else {
        std::cout << inform->minValue;
    }
    std::cout << std::endl;

    return 0;
}


int findMinParallel(int** matrix) {
    HANDLE thr[NTHREADS - 1];
    INFORM informs[NTHREADS];

    int rowsPerThread = ROWS / NTHREADS;
    int extraRows = ROWS % NTHREADS;
    int currentRow = 0;

    for (int i = 0; i < NTHREADS - 1; ++i) {
        informs[i].matrix = matrix;
        informs[i].startRow = currentRow;
        informs[i].threadId = i + 1;

        int rowsForThisThread = rowsPerThread;
        if (i < extraRows) {
            rowsForThisThread++;
        }

        informs[i].endRow = currentRow + rowsForThisThread;
        currentRow = informs[i].endRow;

        thr[i] = (HANDLE)_beginthreadex(NULL, 0, &findMin, &informs[i], 0, NULL);
    }


    informs[NTHREADS - 1].matrix = matrix;
    informs[NTHREADS - 1].startRow = currentRow;
    informs[NTHREADS - 1].endRow = ROWS;
    informs[NTHREADS - 1].threadId = 0;

    findMin(&informs[NTHREADS - 1]);

    WaitForMultipleObjects(NTHREADS - 1, thr, TRUE, INFINITE);

    int globalMin = INT_MAX;
    bool found = false;

    for (int i = 0; i < NTHREADS; ++i) {
        if (informs[i].minValue != INT_MAX) {
            if (informs[i].minValue < globalMin) {
                globalMin = informs[i].minValue;
                found = true;
            }
        }
    }

    for (int i = 0; i < NTHREADS - 1; ++i) {
        CloseHandle(thr[i]);
    }

    return found ? globalMin : INT_MAX;
}

void initMatrix(int** matrix) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            matrix[i][j] = rand() % 2000 - 1000;
        }
    }
}

void printMatrix(int** matrix) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "RUS");
    int** matrix = new int* [ROWS];
    for (int i = 0; i < ROWS; ++i) {
        matrix[i] = new int[COLS];
    }

    initMatrix(matrix);

    printMatrix(matrix);
    std::cout << std::endl;

    std::cout << "Начало параллельного поиска" << std::endl;

    int result = findMinParallel(matrix);
    std::cout << "Конец параллельного поиска" << std::endl;
    std::cout << std::endl;

    if (result == INT_MAX) {
        std::cout << "Числа, начинающиеся с четной цифры, не найдены" << std::endl;
    }
    else {
        std::cout << "Минимальное число, начинающееся с четной цифры: " << result << std::endl;
    }

    for (int i = 0; i < ROWS; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;

    return 0;
}
