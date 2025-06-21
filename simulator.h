#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "world.h"
#include <QObject>
#include <atomic>

class Simulator : public QObject {
    Q_OBJECT
private:
    std::atomic<bool> running;
    long long stepCount;
    World& world;

public:
    Simulator(World& w);
    ~Simulator();

    int runSimulation();

    void start();
    void stop();
    void step(int n);
    void step();
    void performStepLogic();

};

#endif // SIMULATOR_H
