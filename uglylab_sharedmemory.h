#ifndef UGLYLAB_SHAREDMEMORY_H
#define UGLYLAB_SHAREDMEMORY_H


#include "grid3d.h"
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include <vector>

// ---------- Agent data ----------
struct AgentData {
    float x, y, z;
    int species_id;
};

// ---------- Command buffer ----------
constexpr const char* CMD_SHM_NAME = "/uglylab_cmd";

enum CommandType {
    CMD_NONE = 0,
    CMD_START,
    CMD_STOP,
    CMD_STEP,
    CMD_RESET,
    CMD_TERMINATE,
    CMD_INITIALIZE //run one step at start (just to fill buffers), without incrementing step count
};

struct CommandBuffer {
    std::atomic<int> command;

    std::atomic<bool> gridRequested;
    std::atomic<int> gridType;     // 0 = int, 1 = float, etc.
    std::atomic<int> gridX, gridY, gridZ;
    std::atomic<float> gridCellSize;
    std::atomic<bool> gridReady;   // viewer will set this to true
};

inline CommandBuffer* attachCommandBuffer() {
    int fd = shm_open(CMD_SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (sim command)");
        return nullptr;
    }

    void* ptr = mmap(nullptr, sizeof(CommandBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (sim command)");
        return nullptr;
    }

    return static_cast<CommandBuffer*>(ptr);
}

inline CommandBuffer* openOrCreateCommandBuffer() {
    int fd = shm_open(CMD_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (viewer command)");
        return nullptr;
    }

    if (ftruncate(fd, sizeof(CommandBuffer)) == -1) {
        perror("ftruncate (command)");
        close(fd);
        return nullptr;
    }

    void* ptr = mmap(nullptr, sizeof(CommandBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (viewer command)");
        close(fd);
        return nullptr;
    }

    // Zero-initialize command
    auto* cmd = static_cast<CommandBuffer*>(ptr);
    cmd->command.store(CMD_NONE);
    return cmd;
}

// ---------- Visualization buffer ----------
constexpr const char* SHM_NAME = "/uglylab_shm";
constexpr int MAX_CHUNK_SIZE = 4096;
constexpr int MAX_CHUNKS_PER_FRAME = 10000;
constexpr int NUM_BUFFERS = 2;

struct AgentChunk {
    std::atomic<int> ready;
    int frame_index;
    int chunk_index;
    int total_chunks;
    int agents_in_chunk;
    AgentData agents[MAX_CHUNK_SIZE];
};

struct SharedBuffer {
    std::atomic<int> currentStep;
    std::atomic<int> visible_buffer_index;
    AgentChunk buffers[NUM_BUFFERS][MAX_CHUNKS_PER_FRAME];
};

inline SharedBuffer* attachSharedBuffer() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (sim shared)");
        return nullptr;
    }

    void* ptr = mmap(nullptr, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (sim shared)");
        return nullptr;
    }

    return static_cast<SharedBuffer*>(ptr);
}

inline void writeAgentsPaged(SharedBuffer* buffer, const std::vector<AgentData>& allAgents, int frame_index) {
    if (!buffer) return;

    const int write_index = 1 - buffer->visible_buffer_index.load();
    AgentChunk* chunks = buffer->buffers[write_index];

    const size_t total = allAgents.size();
    const int total_chunks = static_cast<int>((total + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE);

    if (total_chunks > MAX_CHUNKS_PER_FRAME) {
        fprintf(stderr, "Too many agents (%zu), max allowed is %d\n", total, MAX_CHUNK_SIZE * MAX_CHUNKS_PER_FRAME);
        return;
    }

    size_t offset = 0;
    for (int i = 0; i < total_chunks; ++i) {
        size_t count = std::min(static_cast<size_t>(MAX_CHUNK_SIZE), total - offset);
        AgentChunk& chunk = chunks[i];

        chunk.ready.store(0);
        chunk.frame_index = frame_index;
        chunk.chunk_index = i;
        chunk.total_chunks = total_chunks;
        chunk.agents_in_chunk = static_cast<int>(count);

        std::memcpy(chunk.agents, allAgents.data() + offset, count * sizeof(AgentData));
        chunk.ready.store(1);

        offset += count;
    }

    buffer->visible_buffer_index.store(write_index);
}

inline SharedBuffer* openOrCreateSharedBuffer() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (viewer shared)");
        return nullptr;
    }

    const size_t size = sizeof(SharedBuffer);
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate (shared)");
        close(fd);
        return nullptr;
    }

    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (shared)");
        close(fd);
        return nullptr;
    }

    auto* buffer = static_cast<SharedBuffer*>(ptr);
    buffer->visible_buffer_index.store(0);

    for (int b = 0; b < NUM_BUFFERS; ++b)
        for (int c = 0; c < MAX_CHUNKS_PER_FRAME; ++c)
            buffer->buffers[b][c].ready.store(0);

    return buffer;
}

// ---------- Grid buffer ----------
constexpr const char* GRID_SHM_NAME = "/uglylab_grid";

template<typename T>
SharedGrid<T>* openOrCreateSharedGrid(int x, int y, int z, float cellSize = 1.0f) {
    const size_t gridSize = sizeof(SharedGrid<T>) + sizeof(T) * x * y * z;

    int fd = shm_open(GRID_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (viewer grid)");
        return nullptr;
    }

    if (ftruncate(fd, gridSize) == -1) {
        perror("ftruncate (grid)");
        close(fd);
        return nullptr;
    }

    void* ptr = mmap(nullptr, gridSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (grid)");
        close(fd);
        return nullptr;
    }

    auto* grid = static_cast<SharedGrid<T>*>(ptr);
    grid->xSize = x;
    grid->ySize = y;
    grid->zSize = z;
    grid->cellSize = cellSize;

    // Optional: zero-initialize grid data
    std::memset(grid->data, 0, sizeof(T) * x * y * z);

    return grid;
}

inline void* createGridBufferFromCommand(const CommandBuffer* cmd) {
    int type = cmd->gridType.load();
    int x = cmd->gridX.load();
    int y = cmd->gridY.load();
    int z = cmd->gridZ.load();
    float cellSize = cmd->gridCellSize.load();

    switch (type) {
    case GRID_TYPE_INT:
        return openOrCreateSharedGrid<int>(x, y, z, cellSize);
    case GRID_TYPE_FLOAT:
        return openOrCreateSharedGrid<float>(x, y, z, cellSize);
    case GRID_TYPE_BOOL:
        return openOrCreateSharedGrid<bool>(x, y, z, cellSize);
    default:
        fprintf(stderr, "Unsupported grid type: %d\n", type);
        return nullptr;
    }
}


template<typename T>
SharedGrid<T>* attachSharedGrid() {
    int fd = shm_open(GRID_SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (sim grid)");
        return nullptr;
    }

    // Read only the header to get sizes
    void* headerPtr = mmap(nullptr, sizeof(SharedGrid<T>), PROT_READ, MAP_SHARED, fd, 0);
    if (headerPtr == MAP_FAILED) {
        perror("mmap header (grid)");
        close(fd);
        return nullptr;
    }

    auto* header = static_cast<SharedGrid<T>*>(headerPtr);
    size_t gridSize = sizeof(SharedGrid<T>) + sizeof(T) * header->xSize * header->ySize * header->zSize;
    munmap(headerPtr, sizeof(SharedGrid<T>));  // unmap header view

    // Remap full grid
    void* ptr = mmap(nullptr, gridSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap full (grid)");
        close(fd);
        return nullptr;
    }

    return static_cast<SharedGrid<T>*>(ptr);
}

// Write a Grid3D<T> into shared memory
template<typename T>
bool writeGridToSharedMemory(const char* shm_name, const Grid3D<T>& grid) {
    const size_t headerSize = sizeof(SharedGrid<T>);
    const size_t totalSize = headerSize + grid.getTotalSize() * sizeof(T);

    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (grid)");
        return false;
    }

    if (ftruncate(fd, totalSize) == -1) {
        perror("ftruncate (grid)");
        close(fd);
        return false;
    }

    void* ptr = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (grid)");
        close(fd);
        return false;
    }

    auto* shared = static_cast<SharedGrid<T>*>(ptr);
    shared->xSize = grid.getXSize();
    shared->ySize = grid.getYSize();
    shared->zSize = grid.getZSize();
    shared->cellSize = grid.getCellSize();

    std::memcpy(shared->data, grid.rawData(), grid.getTotalSize() * sizeof(T));

    munmap(ptr, totalSize);
    close(fd);
    return true;
}

#endif // UGLYLAB_SHAREDMEMORY_H
