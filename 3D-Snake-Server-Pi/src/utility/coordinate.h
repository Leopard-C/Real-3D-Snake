#pragma once
#include <ostream>
#include "./enum.h"

/*******************  Coordinate ********************/
struct Coordinate {
    Coordinate(int x = 0, int y = 0, int z = 0) :
        x(x), y(y), z(z) {}

    Coordinate getOffset(Direction direction, int val) const {
        switch (direction) {
            case X_ASCEND:  return { x + val, y, z };
            case X_DESCEND: return { x - val, y, z };
            case Y_ASCEND:  return { x, y + val, z };
            case Y_DESCEND: return { x, y - val, z };
            case Z_ASCEND:  return { x, y, z + val };
            case Z_DESCEND: return { x, y, z - val };
            default: return { x, y, z };
        }
    }

    void offset(Direction direction, int val) {
        switch (direction) {
            case X_ASCEND:  x += val; break;
            case X_DESCEND: x -= val; break;
            case Y_ASCEND:  y += val; break;
            case Y_DESCEND: y -= val; break;
            case Z_ASCEND:  z += val; break;
            case Z_DESCEND: z -= val; break;
            default: break;
        }
    }

    bool operator==(const Coordinate& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(const Coordinate& rhs) const {
        return !(*this == rhs);
    }

    bool isValid() const { 
        return x > -1 && x < 8 && y > -1 && y < 8 && z > -1 && z < 8;
    }

    bool isAdjacent(const Coordinate& rhs) const {
        int deltaX = abs(rhs.x - x);
        int deltaY = abs(rhs.y - y);
        int deltaZ = abs(rhs.z - z);
        if (deltaX > 1 || deltaY > 1 || deltaZ > 1) {
            return false;
        }
        return (deltaX + deltaY + deltaZ < 2);
    }

    bool isNear(const Coordinate& rhs) const {
        int deltaX = abs(rhs.x - x);
        int deltaY = abs(rhs.y - y);
        int deltaZ = abs(rhs.z - z);
        return (deltaX < 2 && deltaY < 2 && deltaZ < 2);
    }

    bool isSame(const Coordinate& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    int x;
    int y;
    int z;
};


inline std::ostream& operator<<(std::ostream& os, const Coordinate& coord) {
    os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ")";
    return os;
}

inline Coordinate operator+(const Coordinate& left, const Coordinate& right) {
    return { left.x + right.x, left.y + right.y, left.z + right.z };
}

