//
// Created by thijs on 25-04-22.
//

#ifndef GATSP_MPICONTROLLER_H
#define GATSP_MPICONTROLLER_H


#include <mpi.h>
#include "TSPRoute.h"

class MPIController {
private:
    const int tag = 50;
    int id, leftID, rightID, nTasks, rc, pnLength;
    MPI_Status status;
    char pName[MPI_MAX_PROCESSOR_NAME];

    unsigned long nMigrate;
    int mpiBufferSize;
    char* mpiBuffer;

    unsigned long length;
    unsigned long cout;
    FILE* file;
public:
    int getID() const;

    unsigned long getNMigrate() const;

    bool initialize(int argc, char** argv, unsigned long nMigrate);

    void orderBufferSend(unsigned long* data, bool left);

    void orderBufferReceive(unsigned long* data, bool left);

    void finalize();

    void printPointsToFile(unsigned long populationSize, unsigned long generations,
                           double xSize, double ySize, double* xPoints, double* yPoints) const;

    void printPathToFile(unsigned long generation, double routeLength, unsigned long* order);

    void pointsBroadcast(double* xPoints, double* yPoints);

    void sendBufferedMessages();

    void printBestPath(unsigned long generation, double bestRouteLength, unsigned long* bestOrder);
};


#endif //GATSP_MPICONTROLLER_H
