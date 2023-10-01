#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    FILE* f = fopen("gen_input", "w");

    int pointNum = 1000;
    int timeSteps = 1000;

    fprintf(f, "6.6743e-11 %d %d\n", pointNum, timeSteps);

    srand(time(NULL));   
    for (int i = 0; i < pointNum; ++i){
        int mass = rand() % 1000000;
        int posX = rand() % 10, posY = rand() % 10;

        fprintf(f, "%d\n%d %d\n0 0\n", mass, posX, posY);
    } 

    fclose(f);
}
