#include "base.h"

#define MAX_FILE_NAME sizeof("./iterative_results/10000_Net")

int main() {
    timing time;
    initiateTiming(&time);

    int nx, i, j;
    double d = 0;
    double temp, diff, dmax;

    fillF();
    fillU();

    int cnt = 0;

    startTime(&time);
    do {
        cnt++;
        dmax = 0;  // максимальное изменение значений и
        for (i = 1; i < N - 1; i++)
            for (j = 1; j < N - 1; j++) {
                temp = u[i][j];
                u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] +
                                  u[i][j - 1] + u[i][j + 1] - h * h * F[i][j]);
                d = fabs(temp - u[i][j]);
                if (dmax < d) dmax = d;
            }
        printf("%lf\n", dmax);
    } while (dmax > EPS);
    endTime(&time);

    printf("Iteration Num: %d\n", cnt);
    // print(u);

    char fName[MAX_FILE_NAME];
    sprintf(fName, "./iterative_results/%d_Net", N);

    FILE *resF = fopen(fName, "w");
    fprintf(resF, "%lf", result(time));
    fclose(resF);

    return 0;
}