#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    FILE* f = fopen("gen_input", "w");

    int pointNum = 100;
    int timeSteps = 20000;

    fprintf(f, "6.6743e-11 %d %d\n", pointNum, timeSteps);

    srand(time(NULL));   
    for (int i = 0; i < pointNum; ++i){
        int mass = rand() % 1000000;
        int posX = rand() % 100, posY = rand() % 100;

        fprintf(f, "%d\n%d %d\n0 0\n", mass, posX, posY);
    } 

    fclose(f);
}
