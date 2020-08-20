#pragma once
#include "./audio.h"
#include <time.h>
#include <vector>
#include <chrono>

class SnakeAudio {
public:
    void playBg();
    void playEatFood();
    void playFail();
    void stop();

private:
    Audio audioBg;        // 背景音效
    Audio audioEatFood;   // 短暂音效(吃食物 / 死亡)
    Audio audioFail;      // 游戏结束 

    using tp = std::chrono::system_clock::time_point;
    tp start, end;
    std::vector<tp> eatFoodsTime;
};
