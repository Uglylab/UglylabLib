#ifndef ISPECIES_H
#define ISPECIES_H

#include "vec3.h"

class ISpecies {
public:
    virtual ~ISpecies() = default;
    virtual Vec3* getVector3D() = 0;
    virtual int getSpeciesID() const = 0;
};

#endif // ISPECIES_H
