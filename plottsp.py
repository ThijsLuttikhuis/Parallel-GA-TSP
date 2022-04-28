import numpy as np
import matplotlib.pyplot as plt

def ReadHeader(filename):
    data = np.loadtxt(filename, skiprows=1, max_rows=1)

    populationSize = int(data[0])
    generations = int(data[1])
    length = int(data[2])
    xSize = data[3]
    ySize = data[4]

    return populationSize, generations, length, xSize, ySize

def ReadPoints(filename, length):
    data = np.loadtxt(filename, skiprows=4, max_rows=length + 1)
    return data

def ReadPath(filename, length):
    data = np.loadtxt(filename, skiprows=length + 7, delimiter=',')
    return data

populationSize, generations, length, xSize, ySize = ReadHeader("./build/tsp.dat")

points = ReadPoints("./build/tsp.dat", length)
xPoints = points[:, 0]
yPoints = points[:, 1]
data = ReadPath("./build/tsp.dat", length)

generation = data[:, 0]
pathLength = data[:, 1]
path = np.array(data[:, 2:-1], dtype=int)

bestPathLength = 9e99
for i in range(len(generation)):

    # only make a plot if a shorter path is found
    if not pathLength[i] < bestPathLength:
        continue

    bestPathLength = pathLength[i]

    order = np.argsort(path[i, :])

    newXPoints = xPoints[path[i,:]]
    newYPoints = yPoints[path[i,:]]

    plt.figure()
    plt.plot(xPoints, yPoints, '.', color='black')

    plt.plot(newXPoints, newYPoints, color='red')
    plt.plot([newXPoints[0], newXPoints[-1]], [newYPoints[0], newYPoints[-1]], color='red')

    plt.xlabel("x")
    plt.ylabel("y")

    plt.title("generation: " + str(i) + "  -  path length: " + str(pathLength[i]))

    if (i < 10):
        generationstr = "000" + str(i)
    elif (i < 100):
        generationstr = "00" + str(i)
    elif (i < 1000):
        generationstr = "0" + str(i)
    else:
        generationstr = str(i)

    plt.savefig("figures/tsp" + generationstr + ".png")
    plt.close()
