#ifndef GRID_H
#define GRID_H
#include <cstddef>
enum GridDataType {
    GRID_TYPE_INT = 0,
    GRID_TYPE_FLOAT = 1,
    GRID_TYPE_BOOL = 2
};
class Grid {
public:
    virtual ~Grid() = default;

    virtual int getXSize() const = 0;
    virtual int getYSize() const = 0;
    virtual int getZSize() const = 0;
    virtual float getCellSize() const = 0;

    virtual void writeToMemoryRegion(void* ptr) const = 0;
    virtual size_t getRequiredSharedMemorySize() const = 0;

    virtual void* rawVoidData() = 0;  // allow raw access if needed
    virtual GridDataType getType() const = 0;
};
#endif // GRID_H
