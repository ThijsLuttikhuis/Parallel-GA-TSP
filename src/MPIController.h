//
// Created by thijs on 25-04-22.
//

#ifndef GATSP_MPICONTROLLER_H
#define GATSP_MPICONTROLLER_H


#include <mpi.h>
#include <cmath>
#include "TSPRoute.h"

class MPITimer {
private:
    double t1 = 0.0;
    double t2 = 0.0;
    std::vector<double> times = {};

public:
    MPITimer() = default;

    void start() {
        t1 = MPI_Wtime();
    }

    void stop() {
        t2 = MPI_Wtime();

        times.push_back(t2 - t1);
    }

    void printTimeStats() const {
        unsigned long n = times.size();
        double mean = 0.0;
        double std = 0.0;
        for (auto &t : times) {
            mean += t;
        }
        mean /= (double) n;
        for (auto &t : times) {
            std += (t - mean) * (t - mean);
        }
        std = sqrt(std) / (double) n;

        std::cout << n << " runs, time taken: " << mean << " +- " << std << std::endl;
    }
};


class MPIController {
private:
    const int tag = 50;
    int id, leftID, rightID, nTasks, rc, pnLength;
    MPI_Status status{};
    char pName[MPI_MAX_PROCESSOR_NAME]{};

    unsigned long nMigrate;
    int mpiBufferSize;
    char* mpiBuffer;

    unsigned long length;
    unsigned long cout;
    FILE* file;

public:
    MPIController(int argc, char** argv, unsigned long nMigrate_);

    [[nodiscard]] int getID() const;

    [[nodiscard]] unsigned long getNMigrate() const;

    [[nodiscard]] int getNTasks() const;

    void pointsBroadcast(double* xPoints, double* yPoints);

    void orderBufferSend(unsigned long* data, bool left);

    void orderBufferReceive(unsigned long* data, bool left);

    void sendBufferedMessages();

    void printPointsToFile(unsigned long populationSize, unsigned long generations,
                           double xSize, double ySize, double* xPoints, double* yPoints) const;

    void printPathToFile(unsigned long generation, double routeLength, unsigned long* order);

    void printBestPath(unsigned long generation, double bestRouteLength, unsigned long* bestOrder);

    void finalize();
};


#endif //GATSP_MPICONTROLLER_H
