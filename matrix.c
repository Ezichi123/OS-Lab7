#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// The template declared the arrays with MAX before MAX existed, and C does not
// allow global arrays whose size is only known at run time. So the arrays are
// sized by a fixed capacity, and MAX holds the size actually used (default 20).
#define CAPACITY 100
#define NUM_THREADS 10

int matA[CAPACITY][CAPACITY];
int matB[CAPACITY][CAPACITY];

int matSumResult[CAPACITY][CAPACITY];
int matDiffResult[CAPACITY][CAPACITY];
int matProductResult[CAPACITY][CAPACITY];

int MAX = 20;

void fillMatrix(int matrix[CAPACITY][CAPACITY]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[CAPACITY][CAPACITY]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Works out which rows belong to thread number threadNum.
// The rows are split as evenly as possible across NUM_THREADS threads.
// With MAX = 20 and 10 threads, each thread gets 2 rows.
// The thread handles rows from *startRow up to, but not including, *endRow.
void getRowRange(int threadNum, int* startRow, int* endRow) {
    int start = threadNum * MAX;
    start = start / NUM_THREADS;
    int end = (threadNum + 1) * MAX;
    end = end / NUM_THREADS;
    *startRow = start;
    *endRow = end;
}

// Fetches the thread number from the argument, finds the rows that thread
// owns, and sets each cell of matSumResult in those rows to the sum of the
// values at the same coordinates of matA and matB.
void* computeSum(void* args) { // pass in the number of the ith thread
    int* threadNumPtr = (int*) args;
    int threadNum = *threadNumPtr;
    free(threadNumPtr);

    int startRow;
    int endRow;
    getRowRange(threadNum, &startRow, &endRow);

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int a = matA[i][j];
            int b = matB[i][j];
            matSumResult[i][j] = a + b;
        }
    }
    return NULL;
}

// Fetches the thread number from the argument, finds the rows that thread
// owns, and sets each cell of matDiffResult in those rows to the difference
// of the values at the same coordinates of matA and matB (A - B).
void* computeDiff(void* args) { // pass in the number of the ith thread
    int* threadNumPtr = (int*) args;
    int threadNum = *threadNumPtr;
    free(threadNumPtr);

    int startRow;
    int endRow;
    getRowRange(threadNum, &startRow, &endRow);

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int a = matA[i][j];
            int b = matB[i][j];
            matDiffResult[i][j] = a - b;
        }
    }
    return NULL;
}

// Fetches the thread number from the argument, finds the rows that thread
// owns, and sets each cell of matProductResult in those rows to the inner
// product of row i of matA and column j of matB.
void* computeProduct(void* args) { // pass in the number of the ith thread
    int* threadNumPtr = (int*) args;
    int threadNum = *threadNumPtr;
    free(threadNumPtr);

    int startRow;
    int endRow;
    getRowRange(threadNum, &startRow, &endRow);

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int total = 0;
            for (int k = 0; k < MAX; k++) {
                int a = matA[i][k];
                int b = matB[k][j];
                total = total + a * b;
            }
            matProductResult[i][j] = total;
        }
    }
    return NULL;
}

// Creates NUM_THREADS threads that all run the same function, giving each one
// its own thread number, then waits for all of them to finish.
void runOperation(void* (*operation)(void*)) {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        int* threadNum = malloc(sizeof(int));
        if (threadNum == NULL) {
            perror("malloc");
            exit(1);
        }
        *threadNum = i;
        int status = pthread_create(&threads[i], NULL, operation, threadNum);
        if (status != 0) {
            fprintf(stderr, "pthread_create failed for thread %d\n", i);
            free(threadNum);
            exit(1);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Spawn NUM_THREADS threads for each matrix operation.
// Each thread fills a block of rows in the result matrix.
int main(int argc, char* argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX.
    // If no size is given, MAX stays at 20.
    if (argc > 1) {
        MAX = atoi(argv[1]);
    }
    if (MAX < 1 || MAX > CAPACITY) {
        fprintf(stderr, "Matrix size must be between 1 and %d\n", CAPACITY);
        return 1;
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3 to 5. Create the pthread_t objects, start NUM_THREADS threads for each
    // operation (each thread gets a malloc'd copy of its thread number, which
    // it frees), and wait for every thread to finish.
    runOperation(computeSum);
    runOperation(computeDiff);
    runOperation(computeProduct);

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    return 0;
}