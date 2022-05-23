//
// Created by thijs on 25-04-22.
//

#include "MPIController.h"


MPIController::MPIController(int argc, char** argv, unsigned long nMigrate_) {
    char* pEnd;
    unsigned long populationSize = strtol(*++argv, &pEnd, 10);
    ++argv;
    length = strtol(*++argv, &pEnd, 10);
    ++argv;
    ++argv;
    cout = argc > 6 ? strtol(*++argv, &pEnd, 10) : 0;

    /// ----- initialize MPI -----
    rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        printf("MPI initialization failed\n");
        exit(-1);
    }

    nTasks = id = pnLength = {};
    rc = MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
    rc = MPI_Comm_rank(MPI_COMM_WORLD, &id);
    rc = MPI_Get_processor_name(pName, &pnLength);

    if (id == 0) {
        printf("MPI initialized\n");
    }

    if (populationSize < 2 * nMigrate_ * nTasks) {
        std::cerr << "pop_size should be larger than twice the migrating population times number of processes"
                  << std::endl;
        exit(-1);
    }

    leftID = (id == 0) ? nTasks - 1 : id - 1;
    rightID = (id == nTasks - 1) ? 0 : id + 1;

    nMigrate = nMigrate_;
    mpiBufferSize = MPI_BSEND_OVERHEAD + sizeof(unsigned long) * nMigrate * length * 2;
    mpiBuffer = new char[mpiBufferSize];
    MPI_Buffer_attach(mpiBuffer, mpiBufferSize);

    if (cout > 0 && id == 0) file = fopen("tsp.dat", "w");
}

int MPIController::getNTasks() const {
    return nTasks;
}

int MPIController::getID() const {
    return id;
}

unsigned long MPIController::getNMigrate() const {
    return nMigrate;
}

void MPIController::printPointsToFile(unsigned long populationSize, unsigned long generations,
                                      double xSize, double ySize, double* xPoints, double* yPoints) const {

    if (id != 0 || cout == 0) return;

    fprintf(file, "%20s %20s %20s %20s %20s\n",
            "population size", "generations", "number of points", "box size (x)", "box size (y)");
    fprintf(file, "%20lu %20lu %20lu %20.10g %20.10g\n\n%20s %20s\n",
            populationSize, generations, length, xSize, ySize, "xPoints", "yPoints");

    for (unsigned long i = 0; i < length; i++) {
        fprintf(file, "%20.10g %20.10g\n", xPoints[i], yPoints[i]);
    }
    fprintf(file, "\n\ngeneration, path-length, path-order[number of points in path]\n");
}

void MPIController::finalize() {
    MPI_Buffer_detach(&mpiBuffer, &mpiBufferSize);
    delete[] mpiBuffer;

    if (cout > 0 && id == 0) fclose(file);

    printf("\nhost %s (%d)\n", pName, id);
    rc = MPI_Finalize();
}

void MPIController::printPathToFile(unsigned long generation, double routeLength, unsigned long* order) {
    if (!(cout > 0 && id == 0)) return;

    fprintf(file, "%lu, %f, ", generation, routeLength);
    for (unsigned long i = 0; i < length; i++) {
        fprintf(file, "%lu,", order[i]);
    }
    fprintf(file, "%lu\n", order[0]);

    if (cout > 0 && generation % cout == 0) {
        std::cout << "generation: " << generation << "\nroute length: " << routeLength << ", path: ";
        for (unsigned long i = 0; i < length; i++) {
            std::cout << order[i] << " ";
        }
        std::cout << std::endl;
    }
}

void MPIController::orderBufferSend(unsigned long* data, bool left) {
    rc = MPI_Bsend(data, (int) (length * nMigrate), MPI_UNSIGNED_LONG,
                   left ? leftID : rightID, tag, MPI_COMM_WORLD);
}

void MPIController::orderBufferReceive(unsigned long* data, bool left) {
    rc = MPI_Recv(data, (int) (length * nMigrate), MPI_UNSIGNED_LONG,
                  left ? leftID : rightID, tag, MPI_COMM_WORLD, &status);
}

void MPIController::sendBufferedMessages() {
    MPI_Buffer_detach(&mpiBuffer, &mpiBufferSize);
    MPI_Buffer_attach(mpiBuffer, mpiBufferSize);
}

void MPIController::pointsBroadcast(double* xPoints, double* yPoints) {
    rc = MPI_Bcast(xPoints, (int) length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    rc = MPI_Bcast(yPoints, (int) length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void MPIController::printBestPath(unsigned long generation, double bestRouteLength, unsigned long* bestOrder) {

    double* allRouteLengths = nullptr;
    unsigned long* allBestOrders = nullptr;

    if (id == 0) {
        allRouteLengths = new double[nTasks];
        allBestOrders = new unsigned long[nTasks * length];
    }

    rc = MPI_Gather(&bestRouteLength, 1, MPI_DOUBLE,
                    allRouteLengths, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    rc = MPI_Gather(&bestOrder[0], (int) length, MPI_UNSIGNED_LONG,
                    allBestOrders, (int) length, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    if (id != 0) {
        return;
    }

    int bestI = 0;
    for (int i = 0; i < nTasks; i++) {
        if (allRouteLengths[i] < allRouteLengths[bestI]) {
            bestI = i;
        }
    }

    printPathToFile(generation, allRouteLengths[bestI], &allBestOrders[bestI * length]);
}
