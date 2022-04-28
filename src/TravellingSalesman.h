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

    /**
     * @brief return a random parent index weighted by powerFactor according to the position of the parent in the array
     */
    int getRandomWeightedIndex(double powerFactor);

public:
    TravellingSalesman(int argc, char** argv, MPIController* mpiController_, unsigned long nKeepBestParents);

    [[nodiscard]] unsigned long getNumberOfGenerations() const;

    /**
     * @brief create a set of x and y locations to visit randomly selected within the box x(0,xSize), y(0,ySize)
     */
    void randomizeRoutePoints();

    /**
     * @brief initialize the parents and children TSPRoutes and set a random order for each parent
     */
    void createPopulation();

    /**
     * @brief create a new set of parents by genetics of the parents
     *
     * 1. Sort parents by route length and print the best parent to file.
     *
     * 2. Create new children from these parents, where parents with a shorter path length are more likely to 'breed'.
     * A heuristic algorithm is used based on the order of both parents.
     *
     * 3. Set the children as the parents for the next generation and repeat.
     */
    void runGeneration(unsigned long generation);

    /**
     * @brief migrate some of the best parents between processes using the stepping-stone model
     */
    void migrate();

};


#endif //GATSP_TRAVELLINGSALESMAN_H
