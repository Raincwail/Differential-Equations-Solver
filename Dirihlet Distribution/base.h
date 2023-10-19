#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define EPS 0.1
#define N 6
#define h 1
#define rng 100

double F[N][N];
double u[N][N];

void fillF() {
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            F[y][x] = 2 * x * x + 2 * y * y;
        }
    }
}

void fillU() {
    srand(time(NULL));
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            if (y == 0 || x == 0 || y == N - 1 || x == N - 1) {
                u[y][x] = h * h * x * x * y * y;
                continue;
            }

            // u[y][x] = (double)(-rng + rand() % (rng * 2));
            u[y][x] = 0;
        }
    }
}

void print(double x[N][N]) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%lf ", x[i][j]);
        }
        printf("\n");
    }
}