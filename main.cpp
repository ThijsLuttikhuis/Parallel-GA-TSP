//
// Created by thijs on 22-04-22.
//

#include "src/TravellingSalesman.h"
#include "src/MPIController.h"
#include "src/Random.h"

#define TSP_N_RUNS 10

#define TSP_N_MIGRATE 10
#define TSP_GENS_BETWEEN_MIGRATE 1
#define TSP_N_KEEP_BEST_PARENTS 5

int main(int argc, char** argv) {
    /// ----- initialize program variables -----
    MPIController mpiController;
    TravellingSalesman travellingSalesman;

    bool success;
    success = travellingSalesman.initializeParameters(argc, argv, &mpiController, TSP_N_KEEP_BEST_PARENTS);
    if (!success) return -1;

    success = mpiController.initialize(argc, argv, TSP_N_MIGRATE);
    if (!success) return -1;

    Random::initialize(mpiController.getID()+123);

    /// ----- create a population of random paths -----
    travellingSalesman.randomizeRoutePoints();
    travellingSalesman.createPopulation();

    /// ----- create new generations of paths in a loop -----
    for (unsigned long generation = 0; generation < travellingSalesman.getNumberOfGenerations(); generation++) {
        travellingSalesman.runGeneration(generation);

        if (generation % TSP_GENS_BETWEEN_MIGRATE == 0) {
            travellingSalesman.migrate();
        }
    }

    /// ----- finalize mpi, close file and exit -----
    mpiController.finalize();

    return 0;
}

