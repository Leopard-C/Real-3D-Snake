#pragma once
#include "../utility/coordinate.h"
#include <deque>
#include <list>
#include <vector>
#include <cstring>
#include <array>

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
    using uint = unsigned int;
    using Path = std::list<Coordinate>;
    using Walls = std::array<std::array<std::array<char,8>,8>,8>;

    std::list<Coordinate> findPath(const std::deque<Coordinate>& coords_, const Coordinate& dest);

private:
    std::list<Coordinate> findPath_(const std::deque<Coordinate>& coords_,
                                    const Coordinate& dest,
                                    bool destIsFood);

    bool detectCollision(const Walls& walls, const Coordinate& coord) const;

    Node* findNodeOnList(const std::list<Node*>& list, const Coordinate& coord);

    uint manhattan(const Coordinate& src, const Coordinate& dest);

    void destroyList(std::list<Node*>& list);

private:
    static const Coordinate directions[6];
};
