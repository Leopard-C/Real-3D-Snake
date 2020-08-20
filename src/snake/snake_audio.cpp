#include "./snake_audio.h"
#include <cstdio>


void SnakeAudio::playBg() {
    audioBg.playLoop("wav/bg.wav");
    start = std::chrono::system_clock::now();
    std::vector<tp>().swap(eatFoodsTime);
    eatFoodsTime.reserve(256);
}

void SnakeAudio::playEatFood() {
    audioEatFood.playOnce("wav/eat-food.wav");
    eatFoodsTime.push_back(std::chrono::system_clock::now());
}

void SnakeAudio::playFail() {
    audioFail.playOnce("wav/fail.wav");
    end = std::chrono::system_clock::now();

    time_t nowtime = time(NULL);
    struct tm* p = localtime(&nowtime);
    char filename[128] = { 0 };
    snprintf(filename, 128, "/home/pi/dev/cpp/3D-Snake/data/%4d-%02d-%02d_%02d:%02d:%02d.dat", 
             p->tm_year + 1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    printf("%s\n", filename);
    FILE* fp = fopen(filename, "w+b");
    if (fp) {
        fwrite(&start, sizeof(tp), 1, fp);
        fwrite(&(eatFoodsTime.front()), sizeof(tp), eatFoodsTime.size(), fp);
        fwrite(&end, sizeof(tp), 1, fp);
        fclose(fp);
        //fprintf(fp, "%ld\n\n", start);
        //for (auto t : eatFoodsTime) {
        //    fprintf(fp, "%ld\n", t);
        //}
        //fprintf(fp, "\n%ld\n", end);
    }
}

void SnakeAudio::stop() {
    audioBg.stop();
    audioEatFood.stop();
    audioFail.stop();
}

