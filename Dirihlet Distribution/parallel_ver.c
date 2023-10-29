#include <omp.h>

#include "base.h"

#define MAX_FILE_NAME sizeof("./parallel_results/10000_Net")

double un[N][N];

int main(int argC, char *argV[]) {
    int numberOfThreads = strtol(argV[1], NULL, 10);
    omp_set_dynamic(0);
    omp_set_num_threads(numberOfThreads);

    int nx, i, j;
    double d = 0, dm;
    double temp, diff, dmax;

    fillF();
    fillU();

    int cnt = 0;
    double st, en;

    omp_lock_t dmax_lock;
    omp_init_lock(&dmax_lock);

    st = omp_get_wtime();
    do {
        dmax = 0;  // максимальное изменение значений u
#pragma omp parallel for shared(u, un, dmax) private(i, j, temp, d, dm)
        for (i = 1; i < N - 1; i++) {
            dm = 0;
            for (j = 1; j < N - 1; j++) {
                temp = u[i][j];
                un[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] +
                                   u[i][j - 1] + u[i][j + 1] - h * h * F[i][j]);
                d = fabs(temp - un[i][j]);
                if (dm < d) dm = d;
            }

            omp_set_lock(&dmax_lock);
            if (dmax < dm) dmax = dm;
            omp_unset_lock(&dmax_lock);
        }

        // #pragma omp parallel for shared(u, un)
        for (i = 1; i < N - 1; i++) {  // обновление данных
            for (j = 1; j < N - 1; j++) {
                u[i][j] = un[i][j];
            }
        }

        // printf("%lf\n", dmax);
        cnt++;
    } while (dmax > EPS);
    en = omp_get_wtime();

    printf("Iteration Num: %d\n", cnt);

    char fName[MAX_FILE_NAME];
    sprintf(fName, "./parallel_results/%d_Net_%d_Threads", N, numberOfThreads);

    FILE *resF = fopen(fName, "w");
    fprintf(resF, "%lf", (en - st) * 1000);
    fclose(resF);

    return 0;
}