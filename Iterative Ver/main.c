#include <stdio.h>
#include <stdlib.h>

#include "../labVector.h"
#include "../letsTimeIt.h"

#define DT 0.05
#define EPS 1e-10
#define MAX_FILE_NAME sizeof("./results/1000000_Points_10000_Timesteps")

int bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;

void initiateSystem(char *fileName) {
    int i;
    FILE *fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

    for (i = 0; i < bodies; i++) {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
    }

    fclose(fp);
}

void destructSystem() {
    free(masses);
    free(positions);
    free(velocities);
    free(accelerations);
}

void computeAccelerations() {
    for (long i = 0; i < bodies; i++) {
        accelerations[i].x = 0;
        accelerations[i].y = 0;

        for (long j = 0; j < bodies; j++) {
            if (i != j) {
                double denomin = pow(mod(subtractVectors(positions[i], positions[j])), 3);
                denomin = denomin < EPS ? EPS : denomin;

                accelerations[i] = addVectors(
                    accelerations[i],
                    scaleVector(GravConstant * masses[j] / denomin,
                                subtractVectors(positions[j], positions[i])));
            }
        }
    }
}

void computeVelocities() {
    for (long i = 0; i < bodies; i++) {
        velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
    }
}

void computePositions() {
    for (long i = 0; i < bodies; i++)
        positions[i] = addVectors(positions[i], scaleVector(DT, velocities[i]));
}

void printIntoFile(FILE *f, long cycleNum) {
    fprintf(f, "\nCycle %d\n", cycleNum + 1);
    for (long bodyNum = 0; bodyNum < bodies; bodyNum++) {
        fprintf(f, "Body %d : %lf\t%lf\t%lf\t%lf\n", cycleNum + 1, positions[bodyNum].x, positions[bodyNum].y, velocities[bodyNum].x, velocities[bodyNum].y);
    }
}

void simulate() {
    computeAccelerations();
    computePositions();
    computeVelocities();
}

int main(int argC, char *argV[]) {
    int retryTimes = 5;

    timing time;
    initiateTiming(&time);

    if (argC != 2) {
        FILE *f = fopen("output", "w");
        fprintf(f, "Usage : %s <file name containing system configuration data>", argV[0]);
    } else {
        for (int retryNum = 0; retryNum < retryTimes; ++retryNum) {
            FILE *f = fopen("output", "w");
            initiateSystem(argV[1]);

            startTime(&time);
            {
                for (long step = 0; step < timeSteps; step++) {
                    simulate();
                    printIntoFile(f, step);
                }
            }
            endTime(&time);

            destructSystem();
            fclose(f);
        }

        char fName[MAX_FILE_NAME];
        sprintf(fName, "./results_%d/%d_Timesteps", bodies, timeSteps);

        FILE *resF = fopen(fName, "w");
        fprintf(resF, "%lf", result(time));
        fclose(resF);
    }

    return 0;
}
