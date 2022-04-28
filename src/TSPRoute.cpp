//
// Created by thijs on 22-04-22.
//

#include <random>
#include <algorithm>

#include "TSPRoute.h"
#include "Random.h"

const std::vector<unsigned long> &TSPRoute::getOrder() const {
    return order;
}

void TSPRoute::setRandomOrder() {
    /// set the route in order of index
    order = std::vector<unsigned long>(length);
    for (unsigned long i = 0; i < length; i++) {
        order[i] = i;
    }

    /// shuffle the points in the route
    std::shuffle(order.begin(), order.end(), Random::getRNG());
}


void TSPRoute::setOrderFromParents(TSPRoute* parent1, TSPRoute* parent2) {
    /// create new order vector and get the order of each parent
    order = std::vector<unsigned long>(length, -1);
    auto parent1order = parent1->getOrder();
    auto parent2order = parent2->getOrder();

    /// set first city as the first city from one of the parents randomly
    int r = Random::randInt(0, 1);
    order[0] = r ? parent1order[0] : parent2order[0];

    /// add new points to the 'order' vector iteratively
    for (unsigned long i = 1; i < length; i++) {
        auto orderContains = [this, i](unsigned long orderi) {
            for (unsigned long j = 0; j < i; j++) {
                if (this->order[j] == orderi) {
                    return true;
                }
            }
            return false;
        };

        /// iterate until we find the next city that is not yet included for each parent
        unsigned long v1 = i, v2 = i;
        while (orderContains(parent1order[v1]) && ++v1 < length) {}
        while (orderContains(parent2order[v2]) && ++v2 < length) {}

        if (v1 >= length && v2 >= length) {
            /// no parent cities left, fill the rest of the order-array randomly from the remaining cities

            // find the number of remaining cities and their location
            std::vector<int> remainingCities(length, -1);
            int nRemaining = 0;
            for (unsigned long j = 0; j < length; j++) {
                if (!orderContains(j)) {
                    remainingCities[j] = j;
                    nRemaining++;
                }
            }

            // append the remaining cities randomly to the order-array
            while (nRemaining > 0) {
                int element = Random::randInt(0, nRemaining - 1);
                int ei = 0;
                for (unsigned long j = 0; j < length; j++) {
                    if (remainingCities[j] >= 0) {
                        if (element == ei++) {
                            remainingCities[j] = -1;
                            nRemaining--;
                            order[i++] = j;
                            break;
                        }
                    }
                }
            }
            break;
        } else if (v1 >= length) {
            /// parent 1 does not have remaining cities, therefore use the first value from parent 2
            order[i] = parent2order[v2];
        } else if (v2 >= length) {
            /// parent 2 does not have remaining cities, therefore use the first value from parent 1
            order[i] = parent1order[v1];
        } else {
            /// both parents have a city left, therefore choose the closest city
            double dist1 = getDistSquared(parent1order[v1], order[i - 1]);
            double dist2 = getDistSquared(parent2order[v2], order[i - 1]);

            order[i] = (dist1 < dist2) ? parent1order[v1] : parent2order[v2];
        }
    }

    /// add a random mutation by swapping two cities

    // get two unique indices
    int r1 = Random::randInt(0, length-1);
    int r2 = Random::randInt(0, length-1);
    while (r2 == r1) r2 = Random::randInt(0, length-1);

    // swap the values at those two indices
    unsigned long temp = order[r1];
    order[r1] = order[r2];
    order[r2] = temp;
}

double TSPRoute::getRouteLength() const {
    double routeLength = 0.0;

    for (unsigned long i = 0; i < length; i++) {
        unsigned long j = (i == 0) ? length-1 : i - 1;
        routeLength += getDist(order[i], order[j]);
    }
    
    return routeLength;
}

double TSPRoute::getDistSquared(unsigned long indexA, unsigned long indexB) const {
    double xA = xPoints[indexA];
    double xB = xPoints[indexB];
    double yA = yPoints[indexA];
    double yB = yPoints[indexB];

    return ((xA - xB) * (xA - xB) + (yA - yB) * (yA - yB));
}

double TSPRoute::getDist(unsigned long indexA, unsigned long indexB) const {
    return sqrt(getDistSquared(indexA, indexB));
}

void TSPRoute::setOrder(std::vector<unsigned long> route) {
    order = {};
    std::copy(&route[0], &route[length], back_inserter(order));
}

