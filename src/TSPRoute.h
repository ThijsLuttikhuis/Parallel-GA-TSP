//
// Created by thijs on 22-04-22.
//

#ifndef GATSP_TSPROUTE_H
#define GATSP_TSPROUTE_H

#include <iostream>
#include <vector>

class TSPRoute {
private:
    std::vector<unsigned long> order;
    double routeLength = 0.0;

    unsigned long length;
    double* xPoints;
    double* yPoints;

    void setUniqueEncoding();
public:

    TSPRoute(unsigned long length, double* xPoints, double* yPoints)
          : length(length), xPoints(xPoints), yPoints(yPoints) {}

    [[nodiscard]] const std::vector<unsigned long> &getOrder() const;

    static void createDistanceTable(unsigned long length_, const double* xPoints_, const double* yPoints_);

    /**
     * @brief set a route randomly by shuffling the points around
     */
    void setRandomOrder();

    /**
     * @brief set a route from a vector
     */
    void setOrder(std::vector<unsigned long> route);

    /**
     * @brief set the route of the child using two parents and some heuristics
     *
     * 1. Take as the first city of the child the first one from either of the parents.
     *
     * 2. Choose as the second city of the child the one of the corresponding cities in
     * the parents that is closer to the first city in the child.
     *
     * 3. If the new city is already included in the child choose the second city of the
     * other parent. If this is also included in the child then choose the next city from
     * either of the parents randomly (and in such a way that it does not introduce a
     * cycle).
     *
     * 4. Go in a similar fashion through all the cities until the child has all cities
     */
    void setOrderFromParents(TSPRoute* parent1, TSPRoute* parent2);

    /**
     * @brief return the total length of the route
     */
    [[nodiscard]] double getRouteLength();

    /**
     * @brief return the distance squared between two points (indices) in the vector 'order'
     */
    [[nodiscard]] double getDistSquared(unsigned long indexA, unsigned long indexB) const;

    /**
     * @brief return the distance between two points (indices) in the vector 'order'
     */
    [[nodiscard]] double getDist(unsigned long indexA, unsigned long indexB) const;
};


#endif //GATSP_TSPROUTE_H
