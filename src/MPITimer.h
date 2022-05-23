//
// Created by thijs on 23-05-22.
//

#ifndef GATSP_MPITIMER_H
#define GATSP_MPITIMER_H


class MPITimer {
private:
    double t1 = 0.0;
    double t2 = 0.0;
    std::vector<double> times = {};

public:
    MPITimer() = default;

    /**
    * @brief start the timer
    */
    void start();

    /**
    * @brief stop the timer and add the time to the vector times
    */
    void stop();

    /**
    * @brief print the mean and standard deviation of measured times
    */
    void printTimeStats() const;
};


#endif //GATSP_MPITIMER_H
