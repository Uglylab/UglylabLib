#include "simulator.h"
#include "uglylab_sharedmemory.h"
#include <QThread>

// Global shared memory instance
static CommandBuffer* cmd = attachCommandBuffer();
static SharedBuffer* shm = attachSharedBuffer();

static int grid_shm_fd = -1;
static void* grid_shm_ptr = nullptr;

Simulator::Simulator(World& w)
    : running(false), stepCount(0), world(w) {
}

Simulator::~Simulator() {
    stop();
    if (grid_shm_ptr && grid_shm_fd != -1) {
        munmap(grid_shm_ptr, world.getGrid()->getRequiredSharedMemorySize());
        close(grid_shm_fd);
    }

}

void Simulator::start() {
    if (running) return;

    running = true;

}

void Simulator::stop() {
    if (!running) return;
    running = false;
}

void Simulator::step(int n) {
    for (int i = 0; i < n; ++i) {
        step();
    }
}

int Simulator::runSimulation() {
    if (!cmd || !shm) {
        fprintf(stderr, "Shared memory not available.\n");
        return 1;
    }

    if (world.hasGrid() && !cmd->gridRequested.load()) {

        cmd->gridRequested.store(true);
        cmd->gridType.store(world.getGrid()->getType());
        cmd->gridX.store(world.getGrid()->getXSize());
        cmd->gridY.store(world.getGrid()->getYSize());
        cmd->gridZ.store(world.getGrid()->getZSize());
        cmd->gridCellSize.store(world.getGrid()->getCellSize());
    }


    while (true) {
        // 🌟 Late binding: attach grid only when viewer says "I'm ready"
        if (world.hasGrid() && cmd->gridReady.load() && !grid_shm_ptr) {
            int type = cmd->gridType.load();
            if (type == GRID_TYPE_INT) {
                grid_shm_ptr = attachSharedGrid<int>();
            } else if (type == GRID_TYPE_FLOAT) {
                grid_shm_ptr = attachSharedGrid<float>();
            } else if (type == GRID_TYPE_BOOL) {
                grid_shm_ptr = attachSharedGrid<bool>();
            }

            if (!grid_shm_ptr) {
                fprintf(stderr, "❌ Failed to attach to grid shared memory\n");
            } else {
                fprintf(stderr, "✅ Simulator attached to grid shared memory at %p\n", grid_shm_ptr);
            }
            continue;  // Wait for next command
        }

        if (cmd->command.load() == CMD_STEP) {
            step();
            cmd->command.store(CMD_NONE);
        }
        else if (cmd->command.load() == CMD_START) {
            running = true;
            cmd->command.store(CMD_NONE);
        }
        else if (cmd->command.load() == CMD_STOP) {
            running = false;
            cmd->command.store(CMD_NONE);
        }
        else if (cmd->command.load() == CMD_RESET) {
            fprintf(stderr, "SIM: CMD_RESET received.\n");
            stepCount = 0;
            world.reset();
            shm->currentStep.store(0);
            performStepLogic();
            cmd->command.store(CMD_NONE);
        }
        else if (cmd->command.load() == CMD_TERMINATE) {
            cmd->command.store(CMD_NONE);
            return 0;  // ✅ clean exit
        }
        else if (cmd->command.load() == CMD_INITIALIZE) {
            performStepLogic();  // Just one step, not setting `running = true`
            cmd->command.store(CMD_NONE);
        }

        if (running) {
            step();
        }

        QThread::msleep(16);  // keep loop responsive
    }
    return 0;
}

void Simulator::performStepLogic() {
    world.executeRules();
    if (shm) {
        auto agent_snapshot = world.collectAllAgentData();
        writeAgentsPaged(shm, agent_snapshot, stepCount);
        if (world.hasGrid() && grid_shm_ptr) {
            world.getGrid()->writeToMemoryRegion(grid_shm_ptr);
        }
    }
}

/*void Simulator::step()
{
    if (cmd->command.load() == CMD_TERMINATE) {
        return;  // ✋ don’t do anything, skip heavy work
    }
    world.executeRules();

    stepCount++;
    shm->currentStep.store(stepCount);

    // Gather agent data   
    std::vector<AgentData> agent_snapshot = world.collectAllAgentData();

    // Write data to double-buffered shared memory
    writeAgentsPaged(shm, agent_snapshot, stepCount);

    if (world.hasGrid() && grid_shm_ptr) {
        world.getGrid()->writeToMemoryRegion(grid_shm_ptr);
    }
}*/

void Simulator::step() {
    ++stepCount;
    performStepLogic();
    if (shm)
        shm->currentStep.store(stepCount);  // ✅ Update only after real step
}




