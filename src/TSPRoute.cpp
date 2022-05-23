//
// Created by thijs on 22-04-22.
//

#include <algorithm>

#include "TSPRoute.h"
#include "Random.h"

const std::vector<unsigned long> &TSPRoute::getOrder() const {
    return order;
}

void TSPRoute::setOrder(std::vector<unsigned long> route) {
    order = {};
    routeLength = 0.0;
    std::copy(&route[0], &route[nPoints], back_inserter(order));
    setUniqueEncoding();
}

void TSPRoute::setRandomOrder() {
    /// set the route in order of index
    order = std::vector<unsigned long>(nPoints);
    routeLength = 0.0;
    for (unsigned long i = 0; i < nPoints; i++) {
        order[i] = i;
    }

    /// shuffle the points in the route
    std::shuffle(order.begin(), order.end(), Random::getRNG());
    setUniqueEncoding();
}

void TSPRoute::setOrderFromParents(TSPRoute* parent1, TSPRoute* parent2) {
    /// create new order vector and get the order of each parent
    std::vector<bool> orderContains(nPoints, false);
    order = std::vector<unsigned long>(nPoints, -1);
    routeLength = 0.0;
    auto parent1order = parent1->getOrder();
    auto parent2order = parent2->getOrder();

    /// set first city as the first city from one of the parents randomly
    int r = Random::randInt(0, 1);
    order[0] = r ? parent1order[0] : parent2order[0];
    orderContains[order[0]] = true;

    /// add new points to the 'order' vector iteratively
    for (unsigned long i = 1; i < nPoints; i++) {

        /// iterate until we find the next city that is not yet included for each parent
        unsigned long v1 = i, v2 = i;
        while (orderContains[parent1order[v1]] && ++v1 < nPoints) {}
        while (orderContains[parent2order[v2]] && ++v2 < nPoints) {}

        if (v1 >= nPoints && v2 >= nPoints) {
            /// no parent cities left, fill the rest of the order-array randomly from the remaining cities

            /// find the number of remaining cities and their location
            std::vector<int> remainingCities(nPoints, -1);
            int nRemaining = 0;
            for (unsigned long j = 0; j < nPoints; j++) {
                if (!orderContains[j]) {
                    remainingCities[j] = j;
                    nRemaining++;
                }
            }

            /// append the remaining cities randomly to the order-array
            while (nRemaining > 0) {
                int element = Random::randInt(0, nRemaining - 1);
                int ei = 0;
                for (unsigned long j = 0; j < nPoints; j++) {
                    if (remainingCities[j] >= 0) {
                        if (element == ei++) {
                            remainingCities[j] = -1;
                            nRemaining--;
                            order[i] = j;
                            orderContains[order[i++]] = true;
                            break;
                        }
                    }
                }
            }
            break;
        } else if (v1 >= nPoints) {
            /// parent 1 does not have remaining cities, therefore use the first value from parent 2
            order[i] = parent2order[v2];
            orderContains[order[i]] = true;
        } else if (v2 >= nPoints) {
            /// parent 2 does not have remaining cities, therefore use the first value from parent 1
            order[i] = parent1order[v1];
            orderContains[order[i]] = true;
        } else {
            /// both parents have a city left, therefore choose the closest city
            double dist1 = getDistSquared(parent1order[v1], order[i - 1]);
            double dist2 = getDistSquared(parent2order[v2], order[i - 1]);

            order[i] = (dist1 < dist2) ? parent1order[v1] : parent2order[v2];
            orderContains[order[i]] = true;
        }
    }

    /// add a random mutation by swapping two cities

    /// get two unique indices and swap their values
    int r1 = Random::randInt(0, (int) nPoints - 1);
    int r2 = Random::randInt(0, (int) nPoints - 1);
    while (r2 == r1) r2 = Random::randInt(0, (int) nPoints - 1);

    std::swap(order[r1], order[r2]);

    setUniqueEncoding();
}

double TSPRoute::getRouteLength() {
    /// make sure the route length is only calculated once
    if (routeLength != 0.0) {
        return routeLength;
    }

    /// calculate sum of distances between consecutive points in the path order, returning back to the starting point
    for (unsigned long i = 0; i < nPoints; i++) {
        unsigned long j = (i == 0) ? nPoints - 1 : i - 1;
        routeLength += getDist(order[i], order[j]);
    }

    return routeLength;
}

double TSPRoute::getDistSquared(unsigned long indexA, unsigned long indexB) const {
    double &xA = xPoints[indexA];
    double &xB = xPoints[indexB];
    double &yA = yPoints[indexA];
    double &yB = yPoints[indexB];

    return ((xA - xB) * (xA - xB) + (yA - yB) * (yA - yB));
}

double TSPRoute::getDist(unsigned long indexA, unsigned long indexB) const {
    return std::sqrt(getDistSquared(indexA, indexB));
}

void TSPRoute::setUniqueEncoding() {
    auto itOrder = std::find(order.begin(), order.end(), 0);
    unsigned long itIndex = itOrder - order.begin();

    if (itIndex != 0) {
        /// set the order such that 0 is the first index
        std::vector<unsigned long> tempswap(nPoints);
        std::copy(itOrder, order.end(), tempswap.begin());
        std::copy(order.begin(), itOrder, tempswap.begin() + nPoints - itIndex);
        tempswap.swap(order);
    }

    if (order[nPoints - 1] > order[1]) {
        /// set the value at index 1 is larger than the value at the last index by reversing the order
        std::reverse(order.begin() + 1, order.end());
    }
}
