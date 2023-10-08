#include <time.h>

typedef struct{
    int measureNums;
    double time;

    clock_t start;
} timing;

void initiateTiming(timing* T){
    T->measureNums = 0;
    T->time = 0;
}

void startTime(timing* T){
    T->start = clock();
}

void endTime(timing* T){
    T->time += (double)(clock() - T->start);
    T->measureNums++;
}

double result(timing T){
    return (double)(T.time / T.measureNums);
}