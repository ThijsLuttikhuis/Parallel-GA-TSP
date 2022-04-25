//
// Created by thijs on 23-04-22.
//

#ifndef GATSP_TRAVELLINGSALESMAN_H
#define GATSP_TRAVELLINGSALESMAN_H

#include <mpi.h>
#include <vector>
#include "TSPRoute.h"
#include "MPIController.h"

class TravellingSalesman {
private:
    MPIController* mpiController;

    unsigned long populationSize;
    unsigned long generations;
    unsigned long length;
    unsigned long nKeepBestParents;
    double xSize;
    double ySize;

    double* xPoints;
    double* yPoints;

    std::vector<TSPRoute> tspChildren;
    std::vector<TSPRoute> tspParents;

public:
    TravellingSalesman() = default;

    unsigned long getNumberOfGenerations() const;

    /**
     * @brief
     */
    bool initializeParameters(int argc, char** argv, MPIController* mpiController_, unsigned long nKeepBestParents);

    void randomizeRoutePoints();

    void createPopulation();

    int getWeightedIndex(double powerFactor);

    void runGeneration(unsigned long generation);

    void migrate();

};


#endif //GATSP_TRAVELLINGSALESMAN_H
