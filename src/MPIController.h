//
// Created by thijs on 25-04-22.
//

#ifndef GATSP_MPICONTROLLER_H
#define GATSP_MPICONTROLLER_H


#include <mpi.h>
#include <cmath>
#include <vector>

enum Neighbour : bool {
    left,
    right,
};

class TSPRoute;

class MPIController {
private:
    const int tag = 50;
    int id, leftID, rightID, nTasks, rc, pnLength;
    MPI_Status status{};
    char pName[MPI_MAX_PROCESSOR_NAME]{};

    unsigned long nMigrate;
    int mpiBufferSize;
    char* mpiBuffer;
    unsigned long nPoints;

    unsigned long cout;
    FILE* file;

    /**
    * @brief print the path with the specified generation, route length and path order (non-root processes just return)
    */
    void printPathToFile(unsigned long generation, double routeLength, unsigned long* order);

public:
    MPIController(int argc, char** argv, unsigned long nMigrate_);

    [[nodiscard]] int getID() const;

    [[nodiscard]] unsigned long getNMigrate() const;

    [[nodiscard]] int getNTasks() const;

    /**
    * @brief broadcast two arrays of points with length nPoints from process 0 to all processes
    */
    void pointsBroadcast(double* xPoints, double* yPoints);

    /**
    * @brief send a buffer containing nMigrate path orders to the left or right neighbour
    */
    void orderBufferSend(unsigned long* data, Neighbour neighbour);

    /**
    * @brief receive a buffer containing nMigrate path orders from the left or right neighbour
    */
    void orderBufferReceive(unsigned long* data, Neighbour neighbour);

    /**
    * @brief force the buffered messages to be send and received by resetting the buffer
    */
    void sendBufferedMessages();

    /**
    * @brief print the x- and y-points to the file (non-root processes just return)
    */
    void printPointsToFile(unsigned long populationSize, unsigned long generations,
                           double xSize, double ySize, double* xPoints, double* yPoints) const;

    /**
    * @brief gather the best path from all processes to the root process, which prints the best global path to file
    */
    void printBestPathToFile(unsigned long generation, double bestRouteLength, unsigned long* bestOrder);

    /**
    * @brief detach and delete buffer, close file and run MPI_Finalize()
    */
    void finalize();
};


#endif //GATSP_MPICONTROLLER_H
