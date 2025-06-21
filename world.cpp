#include "world.h"
#include "ispecies.h"
#include <iostream>

thread_local World* World::currentContext = nullptr;

World::~World() {
    clear();  // ✅ Ensure proper cleanup
}

void World::registerSpecies(void* listPtr) {
    speciesList.push_back(listPtr);
}

void World::addRule(Rule* rule) {
    rules.push_back(rule);
}

void World::executeRules() {
    //std::cout << "Executing rules..." << std::endl;
    for (auto* rule : rules) {
        rule->execute();
    }
}

void World::clearRules() {
    for (size_t i = 0; i < rules.size(); ++i) {
        delete rules[i];
    }
    rules.clear();
}

void World::listAllAgents() {
    int i = 1;
    for (void* listPtr : speciesList) {
        // Here we assume all agent lists are of std::vector<ISpecies*> type
        auto agents = static_cast<std::vector<ISpecies*>*>(listPtr);

        for (ISpecies* agent : *agents) {
            std::cout << "Agent " << i++ << "position: ("
                      << agent->getVector3D()->x << ", "
                      << agent->getVector3D()->y << ", "
                      << agent->getVector3D()->z << ")" << std::endl;
        }
    }
}

std::vector<AgentData> World::collectAllAgentData() {
    std::vector<AgentData> agent_snapshot;
    for (void* listPtr : speciesList) {
        // All agent lists are std::vector<ISpecies*>*

        auto agents = static_cast<std::vector<ISpecies*>*>(listPtr);
        if (agents == nullptr) {
            fprintf(stderr, "  ⚠️ agents is nullptr\n");
            continue;
        }

        for (ISpecies* agent : *agents) {
            auto pos = agent->getVector3D();
            if (pos) {
                agent_snapshot.push_back({
                    pos->x,
                    pos->y,
                    pos->z,
                    agent->getSpeciesID()
                });
            }
        }
    }
    return agent_snapshot;
}


void World::clear() {
    if (alreadyCleared) return;
    alreadyCleared = true;
    // Clear rules
    clearRules();
    // Clear all agents
    for (void* listPtr : speciesList) {
        auto* agents = static_cast<std::vector<ISpecies*>*>(listPtr);
        std::vector<ISpecies*> temp = *agents;

        for (auto* agent : temp) {
            delete agent;
        }

        agents->clear();
    }
    //clear grid
    if (grid) {
        delete grid;
        grid = nullptr;
    }
}

void World::reset() {
    alreadyCleared = false;
    clear();
    initialize();
}
