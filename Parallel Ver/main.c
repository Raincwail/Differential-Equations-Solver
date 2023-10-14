#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "../labVector.h"
#include "../letsTimeIt.h"

#define DT 0.05
#define EPS 1e-10
#define MAX_FILE_NAME sizeof("./results_1000000/10000_Timesteps_100_Threads")

long bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;

long thread_count = 0;
long threadsToPrint = 0;

sem_t prePrint;
sem_t postPrint;
pthread_barrier_t barrier;
pthread_mutex_t blockPrint;

vector *new_positions, *new_velocities, *new_accelerations;

void initiateSystem(char *fileName) {
    long i;
    FILE *fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

    new_positions = (vector *)malloc(bodies * sizeof(vector));
    new_velocities = (vector *)malloc(bodies * sizeof(vector));
    new_accelerations = (vector *)malloc(bodies * sizeof(vector));

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

void initiatePThread() {
    sem_init(&prePrint, 0, 0);
    sem_init(&postPrint, 0, 0);
    pthread_barrier_init(&barrier, NULL, thread_count);
    pthread_mutex_init(&blockPrint, NULL);
}

void destructPThread() {
    pthread_mutex_destroy(&blockPrint);
    pthread_barrier_destroy(&barrier);
    sem_destroy(&postPrint);
    sem_destroy(&prePrint);
}

void computeAccelerations(long st, long en) {
    for (long i = st; i < en; i++) {
        new_accelerations[i].x = 0;
        new_accelerations[i].y = 0;

        for (long j = 0; j < bodies; j++) {
            if (i != j) {
                double denomin = pow(mod(subtractVectors(positions[i], positions[j])), 3);
                denomin = denomin < EPS ? EPS : denomin;

                new_accelerations[i] = addVectors(
                    new_accelerations[i],
                    scaleVector(GravConstant * masses[j] / denomin,
                                subtractVectors(positions[j], positions[i])));
            }
        }
    }
}

void computePositions(long st, long en) {
    for (long i = st; i < en; i++)
        new_positions[i] =
            addVectors(positions[i], scaleVector(DT, velocities[i]));
}

void computeVelocities(long st, long en) {
    for (long i = st; i < en; i++)
        new_velocities[i] =
            addVectors(velocities[i], scaleVector(DT, new_accelerations[i]));
}

void placeIn(long st, long en) {
    for (long i = st; i < en; ++i) {
        positions[i] = new_positions[i];
        velocities[i] = new_velocities[i];
        accelerations[i] = new_accelerations[i];
    }
}

void printIntoFile(FILE *f, long cycleNum) {
    fprintf(f, "\nCycle %d\n", cycleNum + 1);
    for (long bodyNum = 0; bodyNum < bodies; bodyNum++) {
        fprintf(f, "Body %d : %lf\t%lf\t%lf\t%lf\n", cycleNum + 1, positions[bodyNum].x, positions[bodyNum].y, velocities[bodyNum].x, velocities[bodyNum].y);
    }
}

void *simulate(void *rank) {
    long myRank = (long)rank;

    long rng = bodies / thread_count;
    long st = myRank * rng;
    long en = st + rng;

    if (myRank == thread_count - 1) en = bodies;

    for (long step = 0; step < timeSteps; ++step) {
        computeAccelerations(st, en);
        computePositions(st, en);
        computeVelocities(st, en);

        // Stop for other threads
        pthread_barrier_wait(&barrier);

        // Place in the values
        placeIn(st, en);

        // Stop for main thread to printf
        pthread_mutex_lock(&blockPrint);
        threadsToPrint++;
        if (threadsToPrint == thread_count) {
            sem_post(&prePrint);
            threadsToPrint = 0;
            sem_wait(&postPrint);
        }
        pthread_mutex_unlock(&blockPrint);

        pthread_barrier_wait(&barrier);
    }
}

int main(int argC, char *argV[]) {
    int retryTimes = 5;

    timing time;
    initiateTiming(&time);

    if (argC != 3) {
        FILE *f = fopen("output", "w");
        fprintf(f, "Usage : %s <file name containing system configuration data>", argV[0]);
    } else {
        for (int retryNum = 0; retryNum < retryTimes; ++retryNum) {
            FILE *f = fopen("output", "w");
            initiateSystem(argV[1]);

            thread_count = strtol(argV[2], NULL, 10);
            if (bodies < thread_count) thread_count = 1;

            initiatePThread();
            pthread_t *pthread_handles = malloc(thread_count * sizeof(pthread_t));

            startTime(&time);
            {
                for (long i = 0; i < thread_count; ++i) {
                    pthread_create(&pthread_handles[i], NULL, simulate, (void *)i);
                }

                for (long step = 0; step < timeSteps; step++) {
                    sem_wait(&prePrint);
                    printIntoFile(f, step);
                    sem_post(&postPrint);
                }

                for (long i = 0; i < thread_count; ++i) {
                    pthread_join(pthread_handles[i], NULL);
                }
            }
            endTime(&time);

            destructPThread();
            destructSystem();
            fclose(f);
        }

        char fName[MAX_FILE_NAME];
        sprintf(fName, "./results_%d/timeSteps_%d/%d_Threads", bodies, timeSteps, thread_count);

        FILE *resF = fopen(fName, "w");
        fprintf(resF, "%lf", result(time));
        fclose(resF);
    }

    return 0;
}