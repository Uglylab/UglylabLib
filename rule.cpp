#include "rule.h"
#include "world.h"

Rule::Rule()
{
    World::context()->addRule(this);
}

Rule::~Rule() {
    // Destructor implementation (if any)
}
