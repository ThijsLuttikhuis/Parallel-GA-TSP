//
// Created by thijs on 22-04-22.
//

#include "src/TravellingSalesman.h"
#include "src/MPIController.h"
#include "src/MPITimer.h"
#include "src/Random.h"

#define TSP_N_RUNS 10                                   // number of runs to average the time between
#define TSP_USE_DEFAULT_SEED 0                          // use default seed to make points in every random run the same

#define TSP_USE_FILE_INPUT_POINTS 1                     // use an input file with starting points
#define TSP_FILE_NAME "../src/inputdata/uscapitals.dat" // file name containing input starting points

#define TSP_N_MIGRATE 20                                // number of parents migrating left/right per migration round
#define TSP_GENS_BETWEEN_MIGRATE 5                      // number of generations between migration
#define TSP_N_KEEP_BEST_PARENTS 2                       // number of parents not reproducing, to keep optimal solution

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
    auto travellingSalesman = TravellingSalesman(argc, argv, &mpiController,
                                                 TSP_N_KEEP_BEST_PARENTS, TSP_GENS_BETWEEN_MIGRATE);

    /// ----- initialize the timer and random engine
    MPITimer timer;
#if TSP_USE_DEFAULT_SEED == 0
    Random::initialize(10 * mpiController.getID() + (int) time(nullptr));
#else
    Random::initialize(mpiController.getID() + 12345);
#endif

    /// ----- run TSP_N_RUNS times to measure mean and std of time taken -----
    for (int n = 0; n < TSP_N_RUNS; n++) {
        timer.start();

        /// ----- create a population of paths -----
#if TSP_USE_FILE_INPUT_POINTS == 0
        travellingSalesman.randomizeRoutePoints();
#else
        travellingSalesman.loadRoutePoints(TSP_FILE_NAME);
#endif

        travellingSalesman.createPopulation();

        /// ----- create new generations of paths in a loop -----
        for (unsigned long generation = 0; generation < travellingSalesman.getNumberOfGenerations(); generation++) {
            travellingSalesman.runGeneration(generation);
        }
        timer.stop();
    }
    timer.printTimeStats();

    /// ----- finalize mpi, close file and exit -----
    mpiController.finalize();

    return 0;
}
