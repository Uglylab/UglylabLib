#ifndef GRID3D_H
#define GRID3D_H
#include <cstring>
#include <vector>
#include <cassert>
#include "vec3.h"
#include "grid.h"

template<typename T>
struct SharedGrid {
    int xSize, ySize, zSize;
    float cellSize;
    T data[];  // flexible array member
};

template<typename T>
class Grid3D :public Grid {
public:
    Grid3D(int size, float cellSize = 1.0f)
        : xSize(size), ySize(size), zSize(size), cellSize(cellSize),
        data(size * size * size) {}
    Grid3D(int xSize, int ySize, int zSize, float cellSize = 1.0f)
        : xSize(xSize), ySize(ySize), zSize(zSize), cellSize(cellSize),
        data(xSize * ySize * zSize) {}

    inline T& at(int x, int y, int z) {
        assert(inBounds(x, y, z));
        return data[index(x, y, z)];
    }

    inline const T& at(int x, int y, int z) const {
        assert(inBounds(x, y, z));
        return data[index(x, y, z)];
    }

    inline bool inBounds(int x, int y, int z) const {
        return x >= 0 && x < xSize &&
               y >= 0 && y < ySize &&
               z >= 0 && z < zSize;
    }

    inline int getXSize() const override { return xSize; }
    inline int getYSize() const override { return ySize; }
    inline int getZSize() const override { return zSize; }
    inline float getCellSize() const override { return cellSize; }
    void* rawVoidData() override { return data.data(); }

    void writeToMemoryRegion(void* ptr) const override{
        auto* out = reinterpret_cast<SharedGrid<T>*>(ptr);
        out->xSize = xSize;
        out->ySize = ySize;
        out->zSize = zSize;
        out->cellSize = cellSize;
        std::memcpy(out->data, data.data(), getTotalSize() * sizeof(T));
    }

    size_t getRequiredSharedMemorySize() const override{
        return sizeof(SharedGrid<T>) + getTotalSize() * sizeof(T);
    }


    void clear(const T& value = T()) {
        std::fill(data.begin(), data.end(), value);
    }

    // Convert from grid indices (i,j,k) to world coordinates
    Vec3 toWorldCoordinates(int i, int j, int k) const {
        return Vec3{
            (i + 0.5f) * cellSize,
            (j + 0.5f) * cellSize,
            (k + 0.5f) * cellSize
        };
    }

    // Convert from world coordinates (x,y,z) to grid indices (i,j,k)
    Vec3 fromWorldPosition(float x, float y, float z) const {
        int i = static_cast<int>(std::floor(x / cellSize));
        int j = static_cast<int>(std::floor(y / cellSize));
        int k = static_cast<int>(std::floor(z / cellSize));
        return Vec3(i, j, k);
    }

    template<typename Func>
    void forEachNeighbor(int x, int y, int z, Func&& func, bool includeCenter = false) const {
        for (int dz = -1; dz <= 1; ++dz)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    if (!includeCenter && dx == 0 && dy == 0 && dz == 0)
                        continue;

                    int nx = x + dx;
                    int ny = y + dy;
                    int nz = z + dz;

                    if (inBounds(nx, ny, nz))
                        func(nx, ny, nz, at(nx, ny, nz));
                }
    }

    std::size_t getTotalSize() const {
        return static_cast<std::size_t>(xSize) *
               static_cast<std::size_t>(ySize) *
               static_cast<std::size_t>(zSize);
    }

    GridDataType getType() const override {
        if constexpr (std::is_same_v<T, int>) return GRID_TYPE_INT;
        if constexpr (std::is_same_v<T, float>) return GRID_TYPE_FLOAT;
        if constexpr (std::is_same_v<T, bool>) return GRID_TYPE_BOOL;
        return static_cast<GridDataType>(-1);  // unsupported
    }

private:
    int xSize, ySize, zSize;
    float cellSize;
    std::vector<T> data;

    inline int index(int x, int y, int z) const {
        return x + xSize * (y + ySize * z);
    }
};
#endif // GRID3D_H
