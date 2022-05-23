//
// Created by thijs on 25-04-22.
//

#include "MPIController.h"


MPIController::MPIController(int argc, char** argv, unsigned long nMigrate_) {
    /// set relevant input variables
    char* pEnd;
    unsigned long populationSize = strtol(*++argv, &pEnd, 10);
    ++argv;
    nPoints = strtol(*++argv, &pEnd, 10);
    ++argv;
    ++argv;
    cout = argc > 6 ? strtol(*++argv, &pEnd, 10) : 0;

    /// initialize MPI
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

    /// set left and right neighbouring ids
    leftID = (id == 0) ? nTasks - 1 : id - 1;
    rightID = (id == nTasks - 1) ? 0 : id + 1;

    /// initialize the mpi buffer
    nMigrate = nMigrate_;
    mpiBufferSize = MPI_BSEND_OVERHEAD + sizeof(unsigned long) * nMigrate * nPoints * 2;
    mpiBuffer = new char[mpiBufferSize];
    MPI_Buffer_attach(mpiBuffer, mpiBufferSize);

    /// open the file tsp.dat
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

void MPIController::orderBufferSend(unsigned long* data, Neighbour neighbour) {
    rc = MPI_Bsend(data, (int) (nPoints * nMigrate), MPI_UNSIGNED_LONG,
                   neighbour == Neighbour::left ? leftID : rightID, tag, MPI_COMM_WORLD);
}

void MPIController::orderBufferReceive(unsigned long* data, Neighbour neighbour) {
    rc = MPI_Recv(data, (int) (nPoints * nMigrate), MPI_UNSIGNED_LONG,
                  neighbour == Neighbour::left ? leftID : rightID, tag, MPI_COMM_WORLD, &status);
}

void MPIController::sendBufferedMessages() {
    MPI_Buffer_detach(&mpiBuffer, &mpiBufferSize);
    MPI_Buffer_attach(mpiBuffer, mpiBufferSize);
}

void MPIController::pointsBroadcast(double* xPoints, double* yPoints) {
    rc = MPI_Bcast(xPoints, (int) nPoints, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    rc = MPI_Bcast(yPoints, (int) nPoints, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void MPIController::printPointsToFile(unsigned long populationSize, unsigned long generations,
                                      double xSize, double ySize, double* xPoints, double* yPoints) const {

    if (id != 0 || cout == 0) return;

    fprintf(file, "%20s %20s %20s %20s %20s\n",
            "population size", "generations", "number of points", "box size (x)", "box size (y)");
    fprintf(file, "%20lu %20lu %20lu %20.10g %20.10g\n\n%20s %20s\n",
            populationSize, generations, nPoints, xSize, ySize, "xPoints", "yPoints");

    for (unsigned long i = 0; i < nPoints; i++) {
        fprintf(file, "%20.10g %20.10g\n", xPoints[i], yPoints[i]);
    }
    fprintf(file, "\n\ngeneration, path-length, path-order[number of points in path]\n");
}

void MPIController::printPathToFile(unsigned long generation, double routeLength, unsigned long* order) {
    if (cout <= 0 || id != 0) return;

    /// print the generation, route length and path order to file
    fprintf(file, "%lu, %f, ", generation, routeLength);
    for (unsigned long i = 0; i < nPoints; i++) {
        fprintf(file, "%lu,", order[i]);
    }
    fprintf(file, "%lu\n", order[0]);

    /// print to terminal every 'cout' generations if cout is non-zero
    if (cout > 0 && generation % cout == 0) {
        std::cout << "generation: " << generation << "\nroute length: " << routeLength << ", path: ";
        for (unsigned long i = 0; i < nPoints; i++) {
            std::cout << order[i] << " ";
        }
        std::cout << std::endl;
    }
}

void MPIController::printBestPathToFile(unsigned long generation, double bestRouteLength, unsigned long* bestOrder) {

    /// initialize and gather the best route order and route length from each process to process 0
    double* allRouteLengths = nullptr;
    unsigned long* allBestOrders = nullptr;

    if (id == 0) {
        allRouteLengths = new double[nTasks];
        allBestOrders = new unsigned long[nTasks * nPoints];
    }

    rc = MPI_Gather(&bestRouteLength, 1, MPI_DOUBLE,
                    allRouteLengths, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    rc = MPI_Gather(&bestOrder[0], (int) nPoints, MPI_UNSIGNED_LONG,
                    allBestOrders, (int) nPoints, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    if (id != 0) return;

    /// find global best route length
    int bestI = 0;
    for (int i = 0; i < nTasks; i++) {
        if (allRouteLengths[i] < allRouteLengths[bestI]) {
            bestI = i;
        }
    }

    /// print global best route length to file
    printPathToFile(generation, allRouteLengths[bestI], &allBestOrders[bestI * nPoints]);
}

void MPIController::finalize() {
    MPI_Buffer_detach(&mpiBuffer, &mpiBufferSize);
    delete[] mpiBuffer;

    if (cout > 0 && id == 0) fclose(file);

    printf("\nhost %s (%d)\n", pName, id);
    rc = MPI_Finalize();
}
