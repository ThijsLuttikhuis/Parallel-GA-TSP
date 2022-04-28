//
// Created by thijs on 22-04-22.
//

#ifndef GATSP_RANDOM_H
#define GATSP_RANDOM_H


#include <random>

class Random {
private:
    static std::mt19937 rng;
    static std::uniform_real_distribution<double> unif;

public:

    static void initialize(int seed) {
        if (seed == 0) {
            std::random_device rd;
            rng = std::mt19937(rd());
        }
        else {
            rng = std::mt19937(seed);
        }
        unif = std::uniform_real_distribution<double>(0.0, 1.0);
    }

    static std::mt19937 &getRNG() {
        return rng;
    }

    static double random() {
        return unif(rng);
    }

    static int randInt(int min, int max) {
        return (int)(min + (1 + max - min) * unif(rng));
    }

    static double random(double min, double max) {
        return min + (max - min) * unif(rng);
    }

};


#endif //GATSP_RANDOM_H
