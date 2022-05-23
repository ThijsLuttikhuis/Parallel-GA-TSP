//
// Created by thijs on 22-04-22.
//

#include "src/TravellingSalesman.h"
#include "src/MPIController.h"
#include "src/Random.h"

#define TSP_N_RUNS 5                                // number of runs to average the time between
#define USE_DEFAULT_SEED 0                          // use a default seed to make every run with random points the same

#define USE_FILE_INPUT_POINTS 0                     // use an input file with starting points
#define FILE_NAME "../src/inputdata/uscapitals.dat" // file name containing input starting points

#define TSP_N_MIGRATE 10                            // number of parents migrating left and right per migration round
#define TSP_GENS_BETWEEN_MIGRATE 10                 // number of generations between migration
#define TSP_N_KEEP_BEST_PARENTS 5                   // number of best parents not reproducing, to keep optimal solution

int main(int argc, char** argv) {
    /// check for the correct number of input parameters
    if (argc < 5 || argc > 7) {
        fprintf(stderr, "usage: %s pop_size n_route x_size [y_size] [cout]\n", argv[0]);
        fprintf(stderr, "    pop_size = number of trial populations for the genetic algorithm\n");
        fprintf(stderr, "    gens     = number of generations (\'time steps\')\n");
        fprintf(stderr, "    n_route  = number of points for the salesman to travel past\n");
        fprintf(stderr, "    x_size   = box width where the route points could be in\n");
        fprintf(stderr, "    y_size   = (optional) box height -- default: x_size\n");
        fprintf(stderr, "    cout     = interval for printing best route length to stdout\n");
        exit(-1);
    }

    /// ----- initialize program variables -----
    auto mpiController = MPIController(argc, argv, TSP_N_MIGRATE);
    auto travellingSalesman = TravellingSalesman(argc, argv, &mpiController, TSP_N_KEEP_BEST_PARENTS);

    /// ----- initialize the timer and random engine
    MPITimer timer;
    Random::initialize(USE_DEFAULT_SEED ? mpiController.getID() + 12345
                                        : mpiController.getID() * 10 + time(nullptr));

    /// ----- run TSP_N_RUNS times to measure mean and std of time taken -----
    for (int n = 0; n < TSP_N_RUNS; n++) {
        timer.start();

        /// ----- create a population of paths -----
        if (USE_FILE_INPUT_POINTS) {
            travellingSalesman.loadRoutePoints(FILE_NAME);
        } else {
            travellingSalesman.randomizeRoutePoints();
        }

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

