#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DT 0.05
#define EPS 1e-10
#define l long
#define MAX_FILE_NAME sizeof("./results/1000000_Points_100_Threads")

typedef struct
{
    double x, y;
} vector;

int thread_count = 0;
int threadsToPrint = 0;
pthread_barrier_t barrier;
pthread_mutex_t blockPrint;
sem_t prePrint;
sem_t postPrint;

int bodies, timeSteps;
double *masses, GravConstant;   
vector *positions, *velocities, *accelerations;
vector *new_positions, *new_velocities, *new_accelerations;

vector addVectors(vector a, vector b)
{
    vector c = {a.x + b.x, a.y + b.y};

    return c;
}

vector scaleVector(double b, vector a)
{
    vector c = {b * a.x, b * a.y};

    return c;
}

vector subtractVectors(vector a, vector b)
{
    vector c = {a.x - b.x, a.y - b.y};

    return c;
}

double mod(vector a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

void initiateSystem(char *fileName)
{
    int i;
    FILE* fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

    new_positions = (vector *)malloc(bodies * sizeof(vector));
    new_velocities = (vector *)malloc(bodies * sizeof(vector));
    new_accelerations = (vector *)malloc(bodies * sizeof(vector));

    for (i = 0; i < bodies; i++)
    {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
    }

    fclose(fp);
}

void destructSystem(){
    free(masses);
    free(positions);
    free(velocities);
    free(accelerations);
}


void computeAccelerations(int st, int en)
{
    for (int i = st; i < en; i++)
        {
            new_accelerations[i].x = 0;
            new_accelerations[i].y = 0;
            for (int j = 0; j < bodies; j++)
            {
                if (i != j)
                {
                    double denominator = pow(mod(subtractVectors(positions[i], positions[j])), 3);
                    denominator  = denominator < EPS ? EPS : denominator;
                    new_accelerations[i] = addVectors(
                                            new_accelerations[i],
                                            scaleVector(GravConstant * masses[j] /
                                                denominator,
                                                subtractVectors(positions[j], positions[i])
                                            )
                                        );
                }
            }
        }
}


void computePositions(int st, int en)
{
    for (int i = st; i < en; i++)
        new_positions[i] = addVectors(positions[i], scaleVector(DT, velocities[i]));   
}


void computeVelocities(int st, int en)
{
    for (int i = st; i < en; i++)
        new_velocities[i] = addVectors(velocities[i], scaleVector(DT, new_accelerations[i]));
}

void placeIn(int st, int en){
    for (int i = st; i < en; ++i){
        positions[i] = new_positions[i];
        velocities[i] = new_velocities[i];
        accelerations[i] = new_accelerations[i];
    }
}

void* simulate(void* rank)
{
    l myRank = (l) rank;

    l rng = bodies / thread_count;
    
    l st = myRank * rng;
    l en = st + rng;

    for (int i = 0; i < timeSteps; ++i){
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
            if (threadsToPrint == thread_count){
                sem_post(&prePrint);
                threadsToPrint = 0;
                sem_wait(&postPrint);
            }
        pthread_mutex_unlock(&blockPrint);
        pthread_barrier_wait(&barrier);
    }
}

int main(int argC, char *argV[])
{
    l i, j;
    FILE *f = fopen("output", "w");

    double time = 0;
    int retryTimes = 5;

    if (argC != 3)
        fprintf(f, "Usage : %s <file name containing system configuration data>", argV[0]);
    else
    {
        for (int retryNum = 0; retryNum < retryTimes; ++retryNum){
            initiateSystem(argV[1]);
            
            thread_count = strtol(argV[2], NULL, 10);
            pthread_t* pthread_handles = malloc(thread_count * sizeof(pthread_t));
            pthread_barrier_init(&barrier, NULL, thread_count);

            sem_init(&prePrint, 0, 0);
            sem_init(&postPrint, 0, 0);

            pthread_mutex_init(&blockPrint, NULL);
            
            clock_t begin = clock();
            fprintf(f, "Body   :     x              y           vx              vy   ");

            for (l i = 0; i < thread_count; ++i) {
                pthread_create(&pthread_handles[i], NULL, simulate, (void*) i);
            }

            for (i = 0; i < timeSteps; i++)
            {
                sem_wait(&prePrint);
                fprintf(f, "\nCycle %d\n", i + 1);
                for (j = 0; j < bodies; j++)
                    fprintf(f, "Body %d : %lf\t%lf\t%lf\t%lf\n", j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
                sem_post(&postPrint);
            }
        
            for (l i = 0; i < thread_count; ++i ) {
                pthread_join(pthread_handles[i], NULL);
            }

            clock_t end = clock();
            time += (double)(end - begin);

            pthread_barrier_destroy(&barrier);
            pthread_mutex_destroy(&blockPrint);
            sem_destroy(&prePrint);
            sem_destroy(&postPrint);
        }
        
        char fName[MAX_FILE_NAME];
        sprintf(fName, "./results/%d_Points_%d_Threads", bodies, thread_count);
        
        FILE *resF = fopen(fName, "w");
        
        fprintf(resF, "%lf", time / retryTimes);

        fclose(resF);
    }
    
    fclose(f);

    return 0;
}