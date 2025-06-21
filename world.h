#ifndef WORLD_H
#define WORLD_H
#include <vector>
#include "rule.h"
#include "uglylab_sharedmemory.h"

class World {
    friend class Rule;  // ✅ Give access to Rule
private:
    void addRule(Rule* rule);
protected:
    std::vector<Rule*> rules;
    Grid* grid = nullptr;  // Pointer to polymorphic grid base    
    static thread_local World* currentContext;
    bool alreadyCleared = false;
public:
    World() {
        currentContext = this;
    }
    std::vector<void*> speciesList;
    static World* context() { return currentContext; }
    virtual ~World();
    void registerSpecies(void* listPtr);    
    void executeRules();
    void clearRules();
    void listAllAgents(); // List all agents in the world (debugging purpose)
    std::vector<AgentData> collectAllAgentData();
    virtual void initialize() = 0;  // ← Pure virtual!
    void clear();
    void reset();
    Grid* getGrid() const { return grid; }
    template<typename T>
    Grid3D<T>* asGrid() const {
        return static_cast<Grid3D<T>*>(grid);
    }
    bool hasGrid() const { return grid != nullptr; }

};

#endif // WORLD_H
