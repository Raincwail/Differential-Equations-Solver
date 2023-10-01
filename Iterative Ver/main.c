#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DT 0.5
#define EPS 1e-10
#define MAX_FILE_NAME sizeof("./results/1000000_Points")

typedef struct
{
    double x, y;
} vector;

int bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;

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
    FILE *fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

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

void computeAccelerations()
{
    int i, j;

    for (i = 0; i < bodies; i++)
    {
        accelerations[i].x = 0;
        accelerations[i].y = 0;
        for (j = 0; j < bodies; j++)
        {
            if (i != j)
            {
                double denomin = pow(mod(subtractVectors(positions[i], positions[j])), 3);
                denomin = denomin < EPS ? EPS : denomin;
                accelerations[i] = addVectors(accelerations[i], scaleVector(GravConstant * masses[j] / denomin, subtractVectors(positions[j], positions[i])));
            }
        }
    }
}

void computeVelocities()
{
    int i;

    for (i = 0; i < bodies; i++){
        velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
    }
}

void computePositions()
{
    int i;

    for (i = 0; i < bodies; i++)
        positions[i] = addVectors(positions[i], scaleVector(DT,velocities[i]));
}

void simulate()
{
    computeAccelerations();
    computePositions();
    computeVelocities();
}

int main(int argC, char *argV[])
{
    int i, j;
    FILE *f = fopen("output", "w");

    double time = 0;
    int retryTimes = 5;

    if (argC != 2)
        fprintf(f, "Usage : %s <file name containing system configuration data>", argV[0]);
    else
    {
        for (int retryNum = 0; retryNum < retryTimes; ++retryNum){
            initiateSystem(argV[1]);

            clock_t begin = clock();
            fprintf(f, "Body   :     x              y           vx              vy   ");
            for (i = 0; i < timeSteps; i++)
            {
                fprintf(f, "\nCycle %d\n", i + 1);
                simulate();
                for (j = 0; j < bodies; j++)
                    fprintf(f, "Body %d : %lf\t%lf\t%lf\t%lf\n", j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
            }

            clock_t end = clock();
            time += (double)(end - begin);
            
            destructSystem();
        }

        char fName[MAX_FILE_NAME];
        sprintf(fName, "./results/%d_Points", bodies);
        
        FILE *resF = fopen(fName, "w");
        
        fprintf(resF, "%lf", time / retryTimes);

        fclose(resF);
    }

    fclose(f);
    
    return 0;
}
