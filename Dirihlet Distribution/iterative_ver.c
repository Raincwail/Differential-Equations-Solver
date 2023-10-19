#include "base.h"

int main() {
    int nx, i, j;
    double d = 0;
    double temp, diff, dmax;

    fillF();
    fillU();

    int cnt = 0;
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
    } while (dmax > EPS);

    printf("Iteration Num: %d\n", cnt);
    print(u);

    return 0;
}