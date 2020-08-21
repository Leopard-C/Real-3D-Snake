#include "./snake.h"
#include "../utility/utils.h"
#include "../driver/cube.h"
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>

extern LedCube cube;

Coordinate Snake::foodPos = { -1, -1, -1 };
Direction Snake::currDirection = Y_ASCEND;
std::deque<Coordinate> Snake::coords;
bool Snake::shouldQuit = false;
int Snake::state = GAME_OVER;
int Snake::mode = MODE_UNKNOWN;
AStar Snake::astar;
std::mutex Snake::foodFlashMutex;
std::mutex Snake::coordsMutex;
int Snake::flashInterval = 150;
bool Snake::isFlash = false;
SnakeAudio Snake::audio;


/*
*  退出当前游戏
**/
void Snake::quit() {
    foodPos = { -1, -1, -1 };
    currDirection = DIR_ERROR;
    shouldQuit = true;
    state = GAME_OVER;
    mode = MODE_UNKNOWN;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    audio.stop();
    coordsMutex.lock();
    std::deque<Coordinate>().swap(coords);
    coordsMutex.unlock();
    Call(cube.clear());
}

/*
*  生成新的食物
**/
bool Snake::generateNewFoodDirection() {
    int length = coords.size();
    if (length == 512) {
        return false;
    }
    int randNum = rand() % (512 - length) + 1;
    for (int z = 0; z < 8; ++z) {
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                if (cube(x, y, z) == LED_OFF) {
                    if (--randNum == 0) {
                        foodFlashMutex.lock();
                        cube(foodPos) = LED_ON;
                        foodPos.x = x;
                        foodPos.y = y;
                        foodPos.z = z;
                        cube(foodPos) = LED_ON;
                        foodFlashMutex.unlock();
                        return true;
                    }
                }
            }
        }
    }
}


/*
*  初始化
**/
bool Snake::init(const Coordinate& coord, Direction direction) {
    if (!coord.isValid()) {
        return false;
    }
    currDirection = direction;

    coordsMutex.lock();
    std::deque<Coordinate>().swap(coords);
    coords.push_back(coord);
    coordsMutex.unlock();

    cube.clear();
    cube(coord) = LED_ON;
    if (!generateNewFoodDirection()) {
        return false;
    }

    cube.update();

    return true;
}


/* 终止当前游戏
 **/
void Snake::stop() {
    quit();
}


/*************************************************
 *
 *  人工 控制
 *
*************************************************/

/* 改变蛇的移动方向
 **/
bool Snake::changeDirection(Direction newDirection) {
    if (state != Snake::RUNNING) {
        return false;
    }
    if (newDirection == util::reverseDirection(currDirection)) {
        return false;
    }
    else {
        currDirection = newDirection;
        return true;
    }
}

/* 沿着当前方向移动一步
 * return：
 *   GAME_OVER: 游戏结束
 *   CONTINUE:  游戏继续
 **/
int Snake::move() {
    auto newCoord = coords.front().getOffset(currDirection, 1);
    if (newCoord.isValid()) {
        if (std::find(coords.begin(), coords.end(), newCoord) != coords.end()) {
            return Snake::GAME_OVER;
        }
        printf("(%d, %d, %d)\n", newCoord.x, newCoord.y, newCoord.z);
        if (newCoord == foodPos) {
            audio.playEatFood();
            coordsMutex.lock();
            coords.push_front(newCoord);
            cube(coords.front()) = LED_ON;
            coordsMutex.unlock();
            printf("          <%ld>\n", coords.size());
            if (!generateNewFoodDirection()) {
                return Snake::GAME_OVER;
            }
        }
        else {
            cube(coords.back()) = LED_OFF;
            coordsMutex.lock();
            coords.pop_back();
            coords.push_front(newCoord);
            coordsMutex.unlock();
            cube(coords.front()) = LED_ON;
        }
        return Snake::CONTINUE;
    }
    else {
        return Snake::GAME_OVER;
    }
}

/*
*  开始人工控制
**/
bool Snake::startMoveGoAhead(const Coordinate& coord, Direction direction) {
    if (state == Snake::RUNNING || state == Snake::PAUSED) {
        std::cout << "Game is running\n";
        return false;
    }
    if (!init(coord, direction)) {
        return false;
    }

    state = RUNNING;
    mode = Snake::MODE_MANUAL;
    shouldQuit = false;

    audio.playBg();

    std::thread t(backgroundThread_MoveGoAhead); 
    t.detach();
    return true;
}

/* 
*  新线程，蛇自动沿着当前方向前进
**/
void Snake::backgroundThread_MoveGoAhead() {
    while (!shouldQuit) {
        if (state == Snake::GAME_OVER) {
            break;
        }
        if (state == Snake::PAUSED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (move() == Snake::GAME_OVER) {
            audio.playFail();
            state = GAME_OVER;
            printf("Game over!\n");
            quit();
            break;
        }
        cube.update();
        int interval = getSleepInterval();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
    std::cout << "backgroundThread_MoveGoAhead quit!\n";
}



/*************************************************
 *
 *  自动寻路  
 *
*************************************************/

/* 沿着指定的路径（自动寻路获得的路径）移动
 **/
int Snake::autoMove(const std::list<Coordinate>& path, int intervalMs) {
    for (const auto& coord : path) {
        if (state == Snake::GAME_OVER) {
            return Snake::GAME_OVER;
        }
        while (state == PAUSED) {
            if (state == Snake::GAME_OVER) {
                return Snake::GAME_OVER;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (std::find(coords.begin(), coords.end(), coord) != coords.end()) {
            return Snake::GAME_OVER;
        }
        if (coord == foodPos) {
            audio.playEatFood();
            coordsMutex.lock();
            coords.push_front(coord);
            coordsMutex.unlock();
            cube(coords.front()) = LED_ON;
            if (!generateNewFoodDirection()) {
                audio.playFail();
                return Snake::GAME_OVER;
            }
        }
        else {
            cube(coords.back()) = LED_OFF;
            coordsMutex.lock();
            coords.pop_back();
            coords.push_front(coord);
            coordsMutex.unlock();
            cube(coords.front()) = LED_ON;
        }
        cube.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
    if (coords.size() == 512) {
        audio.playFail();
        return Snake::GAME_OVER;
    }
    return Snake::CONTINUE;
}

/*
*  开始自动寻路
**/
bool Snake::autoRun(int intervalMs, const Coordinate& coord, Direction direction) {
    if (state == Snake::RUNNING || state == Snake::PAUSED) {
        std::cout << "Game is running\n";
        return false;
    }
    if (!init(coord, direction)) {
        return false;
    }

    state = RUNNING;
    mode = Snake::MODE_AUTO;

    unsigned int count = 0;
    std::cout << 1 << std::endl;
    audio.playBg();
    //audio.playEatFood();

    while (true) {
        std::list<Coordinate> path = astar.findPath(coords, foodPos);
        if (path.empty()) {
            printf("no path\n");
            break;
        }
        count += path.size();
        printf("          #%ld\n", path.size());
        for (auto coord : path) {
            printf("(%d, %d, %d)\n", coord.x, coord.y, coord.z);
        }
        if (autoMove(path, intervalMs) == Snake::GAME_OVER) {
            break;
        }
        printf("              <%ld>\n", coords.size());
    }
    audio.playFail();
    std::cout << "Moved " << count << " steps" << std::endl;
    std::cout << "Game OVER!" << std::endl;
    quit();
    return true;
}

/**************************************************
 *
 *   让食物闪烁
 *
**************************************************/

bool Snake::startFlashFoodPos() {
    if (isFlash) {
        return true;
    }
    isFlash = true;
    std::thread t(backgroundThread_FlashFoodPos); 
    t.detach();
    return true;
}

bool Snake::stopFlashFoodPos() {
    isFlash = false;
}

void Snake::backgroundThread_FlashFoodPos() {
    bool isLight = true;
    while (isFlash) {
        if ((state == RUNNING || state == PAUSED) && foodPos.x >= 0) {
            foodFlashMutex.lock();
            Call(cube(foodPos) = isLight ? LED_ON : LED_OFF);
            isLight = !isLight;
            foodFlashMutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(flashInterval));
    }

    foodFlashMutex.lock();
    Call(cube(foodPos) = LED_ON);
    foodFlashMutex.unlock();
    //std::cout << "backgroundThread_FlashFoodPos quit!\n";
}



/* 根据当前蛇身长度，获取暂停时间（即蛇移动的速度）
 **/
int Snake::getSleepInterval() {
    unsigned int size = coords.size();

    // size     : [1,   10]
    // interval : [600, 400]
    if (size < 11) {
        return 600 - 20 * size;
    }

    // size     : [11,  30] 
    // interval : [390, 200]
    else if (size < 31) {
        return 400 - (size - 10) * 10;
    }

    // size     : [31,  50] 
    // interval : [195, 100]
    else if (size < 51) {
        return 200 - (size - 30) * 5;
    }

    // size     : [51, +)
    // interval : 100
    else {
        return 100;
    }
}

