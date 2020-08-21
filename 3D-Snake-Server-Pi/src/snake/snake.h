#pragma once
#include <deque>
#include <vector>
#include <list>
#include <mutex>
#include "../utility/coordinate.h"
#include "../utility/enum.h"
#include "../astar/astar.h"
#include "./snake_audio.h"


class Snake {
public:
    /* 游戏状态 */
    enum {
        CONTINUE = 0,
        INVALID_DIRECTION = 1,
        GAME_OVER = 2,
        PAUSED = 4,
        RUNNING = 5
    };

    /* 游戏模式 */
    enum {
        MODE_UNKNOWN = 0,
        MODE_MANUAL = 1,
        MODE_AUTO = 2,
    };

public:

    /*********************************************************
     *
     *                      游戏状态
     *
     ********************************************************/

    /* 退出函数，应该手动调用，让其它线程正常退出
     **/
    static void quit();

    /* 重置
     **/
    static void reset();

    /* 初始化函数（必须首先调用）
     * @coord：蛇的起始位置
     * @direction：蛇的起始运动方向
     **/
    static bool init(const Coordinate& coord = { 3, 2, 3 }, Direction direction = Y_ASCEND);

    /* 终止当前游戏
     **/
    static void stop();


    /*********************************************************
     *
     *                      食物闪烁
     *
     ********************************************************/

    /* 开启/关闭食物闪烁
     **/
    static bool startFlashFoodPos();
    static bool stopFlashFoodPos();
    /* 对应的线程函数 */
    static void backgroundThread_FlashFoodPos();

    /* 设置闪烁间歇
     * @delayMs: 闪烁间歇（毫秒）
     **/
    static void setFlashInterval(int interval) {
        flashInterval = interval;
    }


    /*********************************************************
     *
     *                      人工控制
     *
     ********************************************************/

    /* 开始人工控制蛇的移动
     **/
    static bool startMoveGoAhead(const Coordinate& coord = { 3, 2, 3 }, Direction direction = Y_ASCEND);
    /* 对应的线程函数 */
    static void backgroundThread_MoveGoAhead();

    /* 改变蛇的移动方向
     **/
    static bool changeDirection(Direction newDirection);

    /* 沿着当前方向移动一步
     * return：
     *   GAME_OVER: 游戏结束
     *   CONTINUE:  游戏继续
     **/
    static int move();

    /* 改变游戏状态： 暂停 / 继续 
     **/
    static void pause(bool bPause) {
        state = bPause ? PAUSED : RUNNING;
    }

    /* 获取游戏状态
     **/
    static int getState() {
        return state;
    }


    /*********************************************************
     *
     *                       自动寻路 
     *
    *********************************************************/

    /* 开始自动寻路
     **/
    static bool autoRun(int intervalMs = 100, const Coordinate& coord = { 3, 2, 3 },
                        Direction direction = Y_ASCEND);

    /* 沿着指定的路径（自动寻路获得的路径）移动
     **/
    static int autoMove(const std::list<Coordinate>& path, int intervalMs);


    /*********************************************************
     *
     *                       其他函数
     *
    *********************************************************/

    /* 获得贪吃蛇头部的坐标
     **/
    const Coordinate& head() const { return coords.front(); }
    int headX() const { return coords.front().x; }
    int headY() const { return coords.front().y; }
    int headZ() const { return coords.front().z; }


    /* 生成食物位置
     * return: 
     *   true:  成功，游戏继续
     *   false: 失败，贪吃蛇已经占据所有空间，游戏结束
     **/
    static bool generateNewFoodDirection();


    /* 根据当前蛇身长度，获取暂停时间（即蛇移动的速度）
     **/
    static int getSleepInterval();

protected:
    static bool shouldQuit;

    static Coordinate foodPos;
    static Direction currDirection;
    static std::deque<Coordinate> coords;

    static int state;
    static int mode;

    static AStar astar;         // A*寻路算法

    static std::mutex foodFlashMutex;    // 让食物闪烁的线程使用
    static std::mutex coordsMutex;
    static bool isFlash;
    static int  flashInterval;

    /* 播放音频
    **/
    static SnakeAudio audio;
};

