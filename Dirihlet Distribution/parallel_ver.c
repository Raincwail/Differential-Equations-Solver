#include <omp.h>

#include "base.h"

int main() {
    int nx, i, j;
    double d[N];
    double temp, diff, dmax;

    for (int i = 0; i < N; ++i)
        d[i] = 0;

    fillF();
    fillU();

    omp_lock_t dmax_lock;
    omp_init_lock(&dmax_lock);

    int cnt = 0;
    do {
        cnt++;
        dmax = 0;  // максимальное изменение значений u
        // Нарастание Волны
        for (nx = 1; nx < N - 1; nx++) {
            d[nx] = 0;
#pragma omp parallel for shared(u, nx, d) private(i, j, temp, diff)
            for (i = 1; i < nx + 1; i++) {
                j = nx - i + 1;
                temp = u[i][j];
                u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] +
                                  u[i][j - 1] + u[i][j + 1] - h * h * F[i][j]);
                diff = fabs(temp - u[i][j]);
                if (d[i] < diff) d[i] = diff;
            }
        }

        // Затухание волны
        for (nx = N - 2; nx > 1; nx--) {
#pragma omp parallel for shared(u, nx, d) private(i, j, temp, diff)
            for (i = N - nx; i < N - 1; i++) {
                j = 2 * N - nx - i - 2;
                temp = u[i][j];
                u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] +
                                  u[i][j - 1] + u[i][j + 1] - h * h * F[i][j]);

                diff = fabs(temp - u[i][j]);
                if (d[i] < diff) d[i] = diff;
            }
        }

        // Определение погрешности вычислений
#pragma omp parallel for shared(nx, d, dmax) private(i)
        for (i = 0; i < N; i++) {
            omp_set_lock(&dmax_lock);
            if (dmax < d[i]) dmax = d[i];
            omp_unset_lock(&dmax_lock);
        }

    } while (dmax > EPS);

    printf("Iteration Num: %d\n", cnt);
    print(u);

    return 0;
}