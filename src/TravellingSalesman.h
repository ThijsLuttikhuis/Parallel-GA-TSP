//
// Created by thijs on 23-04-22.
//

#ifndef GATSP_TRAVELLINGSALESMAN_H
#define GATSP_TRAVELLINGSALESMAN_H


#include <iostream>
#include <vector>

class MPIController;

class TSPRoute;

class TravellingSalesman {
private:
    std::vector<TSPRoute*> tspChildren;
    std::vector<TSPRoute*> tspParents;

    MPIController* mpiController;

    unsigned long populationSize;
    unsigned long generations;
    unsigned long nKeepBestParents;
    unsigned long generationsBetweenMigrate;
    double xSize;
    double ySize;

    unsigned long nPoints;
    double* xPoints;
    double* yPoints;

    /**
     * @brief return a random parent index weighted by powerFactor according to the position of the parent in the array
     */
    int getRandomWeightedIndex(double powerFactor);

    /**
     * @brief migrate some of the best parents between processes using the stepping-stone model
     */
    void migrate();

public:
    TravellingSalesman(int argc, char** argv, MPIController* mpiController_,
                       unsigned long nKeepBestParents_, unsigned long generationsBetweenMigrate_);

    [[nodiscard]] unsigned long getNumberOfGenerations() const;

    /**
     * @brief load the route points from a file
     */
    void loadRoutePoints(const std::string &fileName);

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
     * 1. Create new children from parents, where parents with a shorter path length are more likely to 'breed'.
     * A heuristic algorithm is used based on the order of both parents, more info in the function setOrderFromParents.
     *
     * 2. Set the children as the parents for the next generation and repeat.
     *
     * 3. Sort parents by route length and print the best parent to file.
     */
    void runGeneration(unsigned long generation);
};


#endif //GATSP_TRAVELLINGSALESMAN_H
