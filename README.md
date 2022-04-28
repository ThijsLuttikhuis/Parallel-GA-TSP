# Parallel-GA-TSP
Solving the Travelling Salesman Problem (TSP) using a Genetic Algorithm (GA) parallelized with OpenMPI using migration and the stepping-stone model.


### Project compilation for Ubuntu 18.04

#### Required packages:
```
MPI
CMake
```

#### Compile instructions:
```
mkdir build
cd build
cmake ..
make
mpirun -np <#-of-processes> GATSP <pop-size> <generations> <#-of-points> <box-size-x> <box-size-x> <print-output>
```
for example:
```
mpirun -np 4 GATSP 2000 500 80 10.0 10.0 10
```
The output is stored in tsp.dat - The output can be plotted by running plottsp.py.
