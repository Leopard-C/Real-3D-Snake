#include "driver/cube.h"
#include "utility/utils.h"
#include "snake/snake.h"
#include <wiringPi.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include "server/http_server.h"
#include "server/simple_log.h"

#define RETURN_OK()                                root["code"]=200;root["msg"]="OK";return
#define RETURN_OK_MSG(_msg)                        root["code"]=200;root["msg"]=_msg;return;
#define RETURN_ERROR(_code,_msg)                   root["code"]=_code;root["msg"]=_msg;return;

#define RETURN_INTERNAL_SERVER_ERROR() \
    RETURN_ERROR(INTERNAL_SERVER_ERROR,"Internal server error")
#define RETURN_MISSING_PARAM(_param) \
    root["code"]=300;root["msg"]="Missing param: "#_param;return
#define RETURN_INVALID_PARAM(_param) \
    root["code"]=300;root["msg"]="Invalid param: "#_param;return
#define RETURN_INVALID_PARAM_MSG(_param,_more_msg) \
    root["code"]=300;\
    root["msg"]=std::string("Invalid param: "#_param" [") + _more_msg + "]";\
    return


#define CHECK_PARAM(_param_) \
    std::string _param_=req.get_param(#_param_);\
    if(_param_.empty()){RETURN_MISSING_PARAM(_param_);}

#define CHECK_PARAM_TYPE(_param_, type) \
    std::string _param_##_=req.get_param(#_param_);\
    if(_param_##_.empty()) {RETURN_MISSING_PARAM(_param_);}\
    type _param_;\
    if (!util::convert(_param_##_, _param_)){RETURN_INVALID_PARAM(_param_);}

#define CHECK_PARAM_STR(_param_) \
    std::string _param_=req.get_param(#_param_);\
    if(_param_.empty()){RETURN_MISSING_PARAM(_param_);}
#define CHECK_PARAM_INT(_param_)     CHECK_PARAM_TYPE(_param_, int)
#define CHECK_PARAM_INT64(_param_)   CHECK_PARAM_TYPE(_param_, int64_t)
#define CHECK_PARAM_DOUBLE(_param_)  CHECK_PARAM_TYPE(_param_, double)


LedCube cube;
HttpServer svr;

void catchCtrlC(int) {
    printf("\nCatch Ctrl+C!\n");
    LedCube::quit();
    exit(1);
}

#include "snake/audio.h"

int main() {
    set_log_level("ERROR");

    /* 捕获 Ctrl + C */
    signal(SIGINT, catchCtrlC);

    /* 设置GPIO编号方式（BCM）*/
    if (wiringPiSetupGpio() == -1) {
        printf("Wiringpi setup failed\n");
        return 1;
    }

    /* 初始化光立方驱动 */
    cube.setup();

    /* 随机函数种子 */
    srand(time(NULL));

    Snake  snake;

    ThreadPool tp;
    tp.set_pool_size(2);
    svr.set_thread_pool(&tp);

    svr.add_mapping("/hello", [](Request& req, Json::Value& root){
        RETURN_OK_MSG("hello");
    }, GET_METHOD);

    svr.add_mapping("/flash", [&](Request& req, Json::Value& root){
        CHECK_PARAM_STR(state);
        if (state == "on" || state == "start") {
            snake.startFlashFoodPos();
            RETURN_OK();
        }
        else if (state == "off" || state == "stop") {
            snake.stopFlashFoodPos();
            RETURN_OK();
        }
        else {
            RETURN_INVALID_PARAM(state);
        }
    }, GET_METHOD);

    svr.add_mapping("/set-flash-interval", [&](Request& req, Json::Value& root){
        CHECK_PARAM_INT(interval);
        if (interval < 50 || interval > 1000) {
            RETURN_INVALID_PARAM(interval);
        }
        else {
            snake.setFlashInterval(interval);
            RETURN_OK();
        }
    }, GET_METHOD);

    svr.add_mapping("/new-manual", [&](Request& req, Json::Value& root){
        std::string x_ = req.get_param("x");
        std::string y_ = req.get_param("y");
        std::string z_ = req.get_param("z");
        if (x_.empty() && y_.empty() && z_.empty()) {
            if (snake.startMoveGoAhead({3, 2, 3}, Y_ASCEND)) {
                RETURN_OK_MSG("Start");
            }
            else {
                RETURN_ERROR(300, "Start failed");
            }
        }
        else if (!x_.empty() && !y_.empty() && !z_.empty()) {
            int x = stoi(x_);
            int y = stoi(y_);
            int z = stoi(z_);
            std::string dir_ = req.get_param("dir");
            Direction dir;
            if (dir_.empty()) {
                dir = Y_ASCEND;
            }
            else {
                dir = util::getDirection(dir_); 
                if (dir == DIR_ERROR) {
                    RETURN_INVALID_PARAM(dir);
                }
            }
            if (snake.startMoveGoAhead({x, y, z}, dir)) {
                RETURN_OK_MSG("Start");
            }
            else {
                RETURN_ERROR(300, "Start failed");
            }
        }
    }, GET_METHOD);

    svr.add_mapping("/new-auto", [&snake](Request& req, Json::Value& root){
        std::string flash = req.get_param("flash");
        if (flash == "true" || flash == "1") {
            snake.startFlashFoodPos();
        }
        else if (flash == "false" || flash == "0") {
            snake.stopFlashFoodPos();
        }
        std::thread t([&snake]{
            snake.autoRun(30);
        });
        t.detach();
        RETURN_OK();
    }, GET_METHOD);

    svr.add_mapping("/move", [&snake](Request& req, Json::Value& root){
        CHECK_PARAM_STR(dir);
        Direction direction = util::getDirection(dir);
        if (direction != DIR_ERROR) {
            snake.changeDirection(direction);
            RETURN_OK();
        }
        else {
            RETURN_INVALID_PARAM(dir);
        }
    }, GET_METHOD);

    svr.add_mapping("/pause", [&](Request& req, Json::Value& root){
        CHECK_PARAM_STR(state);
        if (state == "on") {
            snake.pause(true);
            RETURN_OK();
        }
        else if (state == "off") {
            snake.pause(false);
            RETURN_OK();
        }
        else {
            RETURN_INVALID_PARAM(state);
        }
    }, GET_METHOD);

    svr.add_mapping("/stop", [&](Request& req, Json::Value& root){
        snake.quit();
        RETURN_OK();
    }, GET_METHOD);

    svr.add_mapping("/stop-server", [&](Request& req, Json::Value& root){
        snake.quit();
        cube.quit();
        svr.stop();
    }, GET_METHOD);

    svr.set_port(1234);
    svr.add_bind_ip("192.168.1.102");
    svr.start_sync();

    return 0;
}
