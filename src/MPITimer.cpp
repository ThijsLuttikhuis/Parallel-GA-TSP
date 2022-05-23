//
// Created by thijs on 23-05-22.
//

#include <mpi.h>
#include <vector>
#include <cmath>

#include "MPITimer.h"

void MPITimer::start() {
    t1 = MPI_Wtime();
}

void MPITimer::stop() {
    t2 = MPI_Wtime();

    times.push_back(t2 - t1);
}

void MPITimer::printTimeStats() const {
    unsigned long n = times.size();
    double mean = 0.0;
    double std = 0.0;

    /// calculate mean of times
    for (auto &t : times) {
        mean += t;
    }
    mean /= (double) n;

    /// calculate standard deviation of times
    for (auto &t : times) {
        std += (t - mean) * (t - mean);
    }
    std = std::sqrt(std) / (double) n;

    /// print mean and std to terminal
    std::cout << n << " runs, time taken: " << mean << " +- " << std << std::endl;
}
