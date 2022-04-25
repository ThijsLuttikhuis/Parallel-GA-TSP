//
// Created by thijs on 23-04-22.
//

#include <iostream>
#include <algorithm>

#include "TravellingSalesman.h"
#include "Random.h"

unsigned long TravellingSalesman::getNumberOfGenerations() const {
    return generations;
}

bool TravellingSalesman::initializeParameters(int argc, char** argv,
                                              MPIController* mpiController_, unsigned long nKeepBestParents_) {

    if (argc < 5 || argc > 7) {
        fprintf(stderr, "usage: %s pop_size n_route x_size [y_size] [cout]\n", argv[0]);
        fprintf(stderr, "    pop_size = number of trial populations for the genetic algorithm\n");
        fprintf(stderr, "    gens     = number of generations (\'time steps\')\n");
        fprintf(stderr, "    n_route  = number of points for the salesman to travel past\n");
        fprintf(stderr, "    x_size   = box width where the route points could be in\n");
        fprintf(stderr, "    y_size   = (optional) box height -- default: x_size\n");
        fprintf(stderr, "    cout     = interval for printing best route length to stdout\n");
        return false;
    }

    // get input parameters
    char* pEnd;
    populationSize = strtol(*++argv, &pEnd, 10);
    generations = strtol(*++argv, &pEnd, 10);
    length = strtol(*++argv, &pEnd, 10);
    xSize = strtod(*++argv, &pEnd);
    ySize = argc > 5 ? strtod(*++argv, &pEnd) : xSize;

    // check if inputs are valid
    if (populationSize < 1 || populationSize > 10000) {
        std::cerr << "pop_size should be between 1 and 10000" << std::endl;
        return false;
    }
    if (generations < 1 || generations > 10000) {
        std::cerr << "gens should be between 1 and 10000" << std::endl;
        return false;
    }
    if (length < 3 || length > 10000) {
        std::cerr << "n_route should be between 3 and 10000" << std::endl;
        return false;
    }
    if (xSize < 0.1 || xSize > 10000.0) {
        std::cerr << "x_size should be between 0.1 and 10000" << std::endl;
        return false;
    }
    if (ySize < 0.1 || ySize > 10000.0) {
        std::cerr << "y_size should be between 0.1 and 10000" << std::endl;
        return false;
    }

    xPoints = new double[length];
    yPoints = new double[length];

    mpiController = mpiController_;
    nKeepBestParents = nKeepBestParents_;

    return true;
}

void TravellingSalesman::randomizeRoutePoints() {
    if (mpiController->getID() == 0) {
        for (unsigned long i = 0; i < length; i++) {
            xPoints[i] = Random::random(0, xSize);
            yPoints[i] = Random::random(0, ySize);
        }

        mpiController->printPointsToFile(populationSize, generations, xSize, ySize, xPoints, yPoints);
    }
    mpiController->pointsBroadcast(xPoints, yPoints);

}

void TravellingSalesman::createPopulation() {
    // initialize a number of parent routes equal to the pop size and set a random route
    tspChildren = std::vector<TSPRoute>(populationSize);
    tspParents = std::vector<TSPRoute>(populationSize);

    for (unsigned long i = 0; i < populationSize; i++) {
        tspParents[i] = TSPRoute(length, xPoints, yPoints);
        tspParents[i].setRandomRoute();
    }
}

int TravellingSalesman::getWeightedIndex(double powerFactor) {
    return (int) std::pow((Random::random(0, std::pow(populationSize, powerFactor) - 1)), 1.0 / powerFactor);
}

void TravellingSalesman::runGeneration(unsigned long generation) {

    // sort the parents by route length (greatest length first, putting the 'fittest' member last)
    std::sort(tspParents.begin(), tspParents.end(), [](const TSPRoute &parent1, const TSPRoute &parent2) {
        return parent1.getRouteLength() > parent2.getRouteLength();
    });

    // print best path to file, which is at the last index of tspParents
    double bestRouteLength = tspParents[populationSize - 1].getRouteLength();
    auto bestOrder = tspParents[populationSize-1].getOrder();
    auto* bestOrderArr = new unsigned long[length];
    std::copy(bestOrder.begin(), bestOrder.end(), &bestOrderArr[0]);
    mpiController->printBestPath(generation, bestRouteLength, bestOrderArr);

    // create new children equal to the population size, keep the 5 best parents intact
    for (unsigned long i = 0; i < populationSize - nKeepBestParents; i++) {
        // select two unique parents randomly
        // the likelihood of a parent selected is proportional to the power (powerFactor) of its index in the array
        double powerFactor = 2;
        int r1 = getWeightedIndex(powerFactor);
        int r2 = getWeightedIndex(powerFactor);
        while (r2 == r1) r2 = getWeightedIndex(powerFactor);

        tspChildren[i] = TSPRoute(length, xPoints, yPoints);
        tspChildren[i].setRouteFromParents(&tspParents[r1], &tspParents[r2]);
    }

    // set the children as the new parents
    for (unsigned long i = 0; i < populationSize - nKeepBestParents; i++) {
        tspParents[i] = tspChildren[i];
    }
}

void TravellingSalesman::migrate() {

    unsigned long nMigrate = mpiController->getNMigrate();

    unsigned long* receiveMigrationData = new unsigned long[nMigrate * length * 2];
    unsigned long* sendMigrationData = new unsigned long[nMigrate * length * 2];

    for (unsigned long i = 0; i < nMigrate * 2; i++) {
        auto order = tspParents[populationSize - 1 - i].getOrder();
        std::copy(order.begin(), order.end(), &sendMigrationData[i * length]);
    }

    mpiController->orderBufferSend(&sendMigrationData[0], true);
    mpiController->orderBufferSend(&sendMigrationData[length * nMigrate], false);

    mpiController->orderBufferReceive(&receiveMigrationData[length * nMigrate], false);
    mpiController->orderBufferReceive(&receiveMigrationData[0], true);

    mpiController->sendBufferedMessages();

    for (unsigned long i = 0; i < nMigrate * 2; i++) {
        std::vector<unsigned long> order;
        std::copy(&receiveMigrationData[i * length], &receiveMigrationData[(i + 1) * length], back_inserter(order));
        tspParents[populationSize - 1 - i].setRoute(order);
    }

    delete[] receiveMigrationData;
    delete[] sendMigrationData;
}






