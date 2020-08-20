#include "./astar.h"
#include <iostream>

const Coordinate AStar::directions[6] = {
    { 0, 0, 1 }, {  0, 1, 0 }, { 0, 0, -1 }, { 0, -1, 0 },
    { 1, 0, 0 }, { -1, 0, 0 }
};


bool AStar::detectCollision(const Walls& walls, const Coordinate& coord) const {
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

void AStar::destroyList(std::list<Node*>& list) {
    for (auto iter = list.begin(); iter != list.end(); ) {
        delete *iter;
        iter = list.erase(iter);
    }
}

std::list<Coordinate> AStar::findPath(const std::deque<Coordinate>& coords_,
                                      const Coordinate& dest)
{
    return findPath_(coords_, dest, true);
}

std::list<Coordinate> AStar::findPath_(const std::deque<Coordinate>& coords_,
                                      const Coordinate& dest,
                                      bool destIsFood)
{
    // 备份原始障碍物坐标
    // 转化为三维数组,提高检测障碍物效率
    std::deque<Coordinate> coords(coords_);
    std::deque<Coordinate> coordsBuff(coords_);
    Walls walls {};
    for (auto& coord : coords_) {
        walls[coord.z][coord.x][coord.y] = 1;
    }

    const Coordinate& src = coords.front();

    // 无路可走
    int count = 0;
    for (int i = 0; i < 6; ++i) {
        auto coord = src + directions[i];
        if (detectCollision(walls, coord)) {
            count++;
        }
    }
    if (count == 6) {
        return Path();
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
            if (detectCollision(walls, newCoord) || findNodeOnList(closedList, newCoord)) {
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

    Path path;
    while (current) {
        path.push_front(current->coord);
        current = current->parent;
    }
    path.pop_front();

    destroyList(openList);
    destroyList(closedList);

    if (path.empty()) {
        std::cout << "empty xx" << std::endl;
        return Path();
    }

    // 如果找到路径
    if (path.back() == dest) {
        // 如果是找尾巴,说明找到,直接返回路径
        if (!destIsFood) {
            return path;
        }

        // 如果是找食物
        // 需要判断 蛇沿着路径吃到食物后, 是否仍然可以跟着尾巴走

        // 先让蛇吃到食物
        for (auto& coord : path) {
            coords.pop_back();
            coords.push_front(coord);
        }

        // 是否能跟着尾巴走
        // 不能
        if (coords.front().isAdjacent(coords.back())) {
            // 先让蛇沿着路径,走着瞧
            // 先走一步, 直到吃到食物
            //  这里要用coordsBuff, 因为coords已经不是原来蛇身坐标
            coordsBuff.pop_back();
            coordsBuff.push_front(path.front());
            Path nextNextPath = findPath_(coordsBuff, dest, true);
            nextNextPath.push_front(path.front());
            return nextNextPath;
        }


        Coordinate back = coords.back();
        coords.pop_back();
        Path nextPath = findPath_(coords, back, false);

        // 能
        if (nextPath.empty() || nextPath.back() == back) {
            std::cout << "Double" << std::endl;
            return path;
        }

        // 不能
        else {
            // 先让蛇沿着路径,走着瞧
            // 先走一步, 直到吃到食物
            //  这里要用coordsBuff, 因为coords已经不是原来蛇身坐标
            coordsBuff.pop_back();
            coordsBuff.push_front(path.front());
            Path nextNextPath = findPath_(coordsBuff, dest, true);
            nextNextPath.push_front(path.front());
            return nextNextPath;
        }
    }


    // 如果没找到路径 
    else {
        // 如果是找尾巴,直接返回路径(无效)
        if (!destIsFood) {
            return path;
        }

        // 下面是找食物没找到路径

        // 是否可以跟着尾巴走
        Path nextPath = findPath_(coords, coords.back(), false);
        if (nextPath.empty()) {
            return Path();
        }

        // 如果能沿着尾巴走
        if (nextPath.back() == coords.back()) {
            // 一步一步走,直到走到某一步,可以找到吃到食物的路径
            coords.pop_back();
            coords.push_front(nextPath.front());
            Path nextNextPath = findPath_(coords, dest, true);
            nextNextPath.push_front(nextPath.front());
            return nextNextPath;
        }

        // 如果不能能沿着尾巴走
        else {
            // 一步一步走,直到走到某一步,可以找到吃到食物的路径
            coords.pop_back();
            coords.push_front(path.front());
            Path nextNextPath = findPath_(coords, dest, true);
            nextNextPath.push_front(path.front());
            return nextNextPath;
        }
    }
}

