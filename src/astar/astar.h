#pragma once
#include "../utility/coordinate.h"
#include <deque>
#include <list>
#include <vector>
#include <cstring>

using uint = unsigned int;

struct Node {
    Node(const Coordinate& coord_, Node* parent_ = nullptr)
        : coord(coord_), parent(parent_) {}
    uint getScore() const { return G + H; }
    uint G = 0;
    uint H = 0;
    Coordinate coord;
    Node* parent;
};

class AStar {
public:
    void setSnake(const std::deque<Coordinate>& coords_);

    bool detectCollision(const Coordinate& coord) const;

    Node* findNodeOnList(const std::list<Node*>& list, const Coordinate& coord);

    uint manhattan(const Coordinate& src, const Coordinate& dest);

    std::list<Coordinate> findPath(const Coordinate& dest);

private:
    char walls[8][8][8];
    std::deque<Coordinate> coords;
    std::deque<Coordinate> coordsBuff;

    Coordinate directions[6] = {
        { 0, 0, 1 }, {  0, 1, 0 }, { 0, 0, -1 }, { 0, -1, 0 },
        { 1, 0, 0 }, { -1, 0, 0 }
    };
};
