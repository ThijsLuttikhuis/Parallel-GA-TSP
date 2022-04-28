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
    auto mpiController = MPIController(argc, argv, TSP_N_MIGRATE);
    auto travellingSalesman = TravellingSalesman(argc, argv, &mpiController, TSP_N_KEEP_BEST_PARENTS);
    Random::initialize(mpiController.getID()+42);
    MPITimer timer;

    /// ----- run TSP_N_RUNS times to measure mean and std of time taken -----
    for (int n = 0; n < TSP_N_RUNS; n++) {
        timer.start();

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

        timer.stop();
    }

    timer.printTimeStats();

    /// ----- finalize mpi, close file and exit -----
    mpiController.finalize();

    return 0;
}

