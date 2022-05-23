//
// Created by thijs on 23-04-22.
//

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "TravellingSalesman.h"
#include "Random.h"

TravellingSalesman::TravellingSalesman(int argc, char** argv, MPIController* mpiController_,
                                       unsigned long nKeepBestParents_) {

    /// get input parameters
    char* pEnd;
    populationSize = strtol(*++argv, &pEnd, 10);
    generations = strtol(*++argv, &pEnd, 10);
    length = strtol(*++argv, &pEnd, 10);
    xSize = strtod(*++argv, &pEnd);
    ySize = argc > 5 ? strtod(*++argv, &pEnd) : xSize;

    /// check if the input parameters are valid
    if (populationSize < 1 || populationSize >= 100000) {
        std::cerr << "pop_size should be between 1 and 100000" << std::endl;
        exit(-1);
    }
    if (generations < 1 || generations >= 10000) {
        std::cerr << "gens should be between 1 and 10000" << std::endl;
        exit(-1);
    }
    if (length < 4 || length >= 10000) {
        std::cerr << "n_route should be between 4 and 10000" << std::endl;
        exit(-1);
    }
    if (xSize < 0.1 || xSize >= 10000.0) {
        std::cerr << "x_size should be between 0.1 and 10000" << std::endl;
        exit(-1);
    }
    if (ySize < 0.1 || ySize >= 10000.0) {
        std::cerr << "y_size should be between 0.1 and 10000" << std::endl;
        exit(-1);
    }

    /// allocate space for the x- and y-points
    xPoints = new double[length];
    yPoints = new double[length];

    mpiController = mpiController_;
    nKeepBestParents = nKeepBestParents_;

    // divide population size between processes (assuming it is divisible by nTasks)
    int nTasks = mpiController->getNTasks();
    populationSize /= nTasks;
}

unsigned long TravellingSalesman::getNumberOfGenerations() const {
    return generations;
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

void TravellingSalesman::loadRoutePoints(const std::string &fileName) {
    if (mpiController->getID() == 0) {
        std::ifstream file;
        file.open(fileName);

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::string inputStr = buffer.str();
        char** ptr = nullptr;
        size_t pos = 0;
        for (unsigned long i = 0; i < length; i++) {
            size_t commaPos = inputStr.find(',');
            size_t endLinePos = inputStr.find('\n');

            auto xstr = inputStr.substr(pos, commaPos);
            auto ystr = inputStr.substr(commaPos+1, endLinePos);
            inputStr = inputStr.substr(endLinePos+1, inputStr.size()-1);

            xPoints[i] = strtod(xstr.c_str(), ptr);
            yPoints[i] = strtod(ystr.c_str(), ptr);
        }

        mpiController->printPointsToFile(populationSize, generations, xSize, ySize, xPoints, yPoints);
    }
    mpiController->pointsBroadcast(xPoints, yPoints);
}

void TravellingSalesman::createPopulation() {
    /// initialize a number of parent routes equal to the pop size and set a random route
    tspChildren = std::vector<TSPRoute*>(populationSize);
    tspParents = std::vector<TSPRoute*>(populationSize);

    for (unsigned long i = 0; i < populationSize; i++) {
        tspChildren[i] = new TSPRoute(length, xPoints, yPoints);
        tspParents[i] = new TSPRoute(length, xPoints, yPoints);
        tspParents[i]->setRandomOrder();
    }
}

int TravellingSalesman::getRandomWeightedIndex(double powerFactor) {
    double power = std::pow(populationSize, powerFactor);
    double num = Random::random(0.0, power - 1.0);
    return (int) std::pow((num), 1.0 / powerFactor);
}

void TravellingSalesman::runGeneration(unsigned long generation) {

    /// sort the parents by route length (greatest length first, putting the 'fittest' member last)
    std::sort(tspParents.begin(), tspParents.end(), [](TSPRoute* parent1, TSPRoute* parent2) {
        return parent1->getRouteLength() > parent2->getRouteLength();
    });

    /// print best path to file, which is at the last inde-> of tspParents
    double bestRouteLength = tspParents[populationSize - 1]->getRouteLength();
    auto bestOrder = tspParents[populationSize - 1]->getOrder();
    auto* bestOrderArr = new unsigned long[length];
    std::copy(bestOrder.begin(), bestOrder.end(), &bestOrderArr[0]);
    mpiController->printBestPath(generation, bestRouteLength, bestOrderArr);

    /// create new children equal to the population size, keep the 5 best parents intact
    for (unsigned long i = 0; i < populationSize - nKeepBestParents; i++) {
        // select two unique parents randomly
        // the likelihood of a parent selected is proportional to the power (powerFactor) of its index in the array
        double powerFactor = 2;
        int r1 = getRandomWeightedIndex(powerFactor);
        int r2 = getRandomWeightedIndex(powerFactor);
        while (r2 == r1) r2 = getRandomWeightedIndex(powerFactor);

        tspChildren[i]->setOrderFromParents(tspParents[r1], tspParents[r2]);
    }

    /// set the children as the new parents
    for (unsigned long i = 0; i < populationSize - nKeepBestParents; i++) {
        tspParents[i]->setOrder(tspChildren[i]->getOrder());
    }
}

void TravellingSalesman::migrate() {

    unsigned long nMigrate = mpiController->getNMigrate();

    auto* receiveMigrationData = new unsigned long[nMigrate * length * 2];
    auto* sendMigrationData = new unsigned long[nMigrate * length * 2];

    /// put all outgoing parents' orders into one array
    for (unsigned long i = 0; i < nMigrate * 2; i++) {
        auto order = tspParents[populationSize - 1 - i]->getOrder();
        std::copy(order.begin(), order.end(), &sendMigrationData[i * length]);
    }

    /// send and receive migrating populations to other processes
    mpiController->orderBufferSend(&sendMigrationData[0], true);
    mpiController->orderBufferSend(&sendMigrationData[length * nMigrate], false);

    mpiController->orderBufferReceive(&receiveMigrationData[length * nMigrate], false);
    mpiController->orderBufferReceive(&receiveMigrationData[0], true);

    mpiController->sendBufferedMessages();

    /// separate the array of incoming route orders and put them into the place of parents that migrated
    for (unsigned long i = 0; i < nMigrate * 2; i++) {
        std::vector<unsigned long> order;
        std::copy(&receiveMigrationData[i * length], &receiveMigrationData[(i + 1) * length], back_inserter(order));
        tspParents[populationSize - 1 - i]->setOrder(order);
    }

    delete[] receiveMigrationData;
    delete[] sendMigrationData;
}
