#include "./astar.h"


void AStar::setSnake(const std::deque<Coordinate>& coords_) {
    memset(walls, 0, 512);
    coords = coordsBuff = coords_;
    for (auto& coord : coords_) {
        walls[coord.z][coord.x][coord.y] = 1;
    }
}

bool AStar::detectCollision(const Coordinate& coord) const {
    if (coord.x < 0 || coord.x >= 8 ||
        coord.y < 0 || coord.y >= 8 ||
        coord.z < 0 || coord.z >= 8 ||
        walls[coord.z][coord.x][coord.y] == 1)
    {
        return true;
    }
    return false;
}

uint AStar::manhattan(const Coordinate& src, const Coordinate& dest) {
    uint deltaX = abs(src.x - dest.x);
    uint delatY = abs(src.y - dest.y);
    uint delatZ = abs(src.z - dest.z);
    return 10 * (deltaX + delatY + delatZ);
}

Node* AStar::findNodeOnList(const std::list<Node*>& list, const Coordinate& coord) {
    for (auto node : list) {
        if (node->coord == coord) {
            return node;
        }
    }
    return nullptr;
}

std::list<Coordinate> AStar::findPath(const Coordinate& dest) {
    const Coordinate& src = coords.front();

    // 无路可走
    int count = 0;
    for (int i = 0; i < 6; ++i) {
        auto coord = src + directions[i];
        if (detectCollision(coord)) {
            count++;
        }
    }
    if (count == 6) {
        return std::list<Coordinate>();
    }

    Node* current = nullptr;
    std::list<Node*> openList;
    std::list<Node*> closedList;
    openList.push_back(new Node(src));

    while (!openList.empty()) {
        auto current_iter = openList.begin();
        current = *current_iter;

        for (auto iter = openList.begin(); iter != openList.end(); ++iter) {
            auto node = *iter;
            if (node->getScore() <= current->getScore()) {
                current = node;
                current_iter = iter;
            }
        }

        if (current->coord == dest) {
            break;
        }

        closedList.push_back(current);
        openList.erase(current_iter);

        for (uint i = 0; i < 6; ++i) {
            Coordinate newCoord(current->coord + directions[i]);
            if (detectCollision(newCoord) || findNodeOnList(closedList, newCoord)) {
                continue;
            }

            uint totalCost = current->G + 10;

            Node* successor = findNodeOnList(openList, newCoord);
            if (!successor) {
                successor = new Node(newCoord, current);
                successor->G = totalCost;
                successor->H = manhattan(successor->coord, dest);
                openList.push_back(successor);
            }
            else if (totalCost < successor->G) {
                successor->parent = current;
                successor->G = totalCost;
            }
        }
    }

    std::list<Coordinate> path;
    while (current) {
        path.push_front(current->coord);
        current = current->parent;
    }
    path.pop_front();

    for (auto iter = openList.begin(); iter != openList.end(); ) {
        delete *iter;
        iter = openList.erase(iter);
    }

    for (auto iter = closedList.begin(); iter != closedList.end(); ) {
        delete *iter;
        iter = closedList.erase(iter);
    }

    if (path.empty()) {
        return std::list<Coordinate>();
    }

    if (path.back() != dest) {
        coordsBuff.pop_back();
        coordsBuff.push_front(path.front());
        this->setSnake(coordsBuff);
        return findPath(dest);
    }
    else {
        return path;
    }
}

