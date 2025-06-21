#ifndef SPECIES_H
#define SPECIES_H

#include <functional>
#include <vector>
#include "ispecies.h"
#include "world.h"

template<typename Derived>
class Species : public ISpecies {
private:
    static bool registered;

public:
    Vec3* position;
    static std::vector<Derived*> agents;

    static void addAgents(int numAgents, const std::vector<std::function<float()>>& distributions) {
        for (int i = 0; i < numAgents; ++i) {
            float x = distributions[0]();
            float y = distributions[1]();
            float z = distributions[2]();
            new Derived(x, y, z);
        }
    }

    Species(){
        if (!registered) {
            World::context()->registerSpecies(&agents);
            registered = true;
        }
        agents.push_back(static_cast<Derived*>(this));
    }
    Species(float x, float y, float z) : Species() {
        position = new Vec3(x, y, z);
    }
    ~Species() {
        delete position;
    }
    Vec3* getVector3D() override  {
        return position;
    }
    int getSpeciesID() const override {
        return Derived::SpeciesID;
    }
};

template<typename Derived>
std::vector<Derived*> Species<Derived>::agents;

template<typename Derived>
bool Species<Derived>::registered = false;
#endif // SPECIES_H
