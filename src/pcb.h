#pragma once
#include <string>
#include <queue>
#include <ctime>

enum ProcessState {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct PCB {
    int pid;
    std::string name;
    ProcessState state;
    int priority;
    int total_time;
    int remaining_time;
    int create_time;
    int start_time;
    int finish_time;
    std::string blocked_reason;

    PCB()
        : pid(0), state(READY), priority(0),
          total_time(0), remaining_time(0),
          create_time(0), start_time(-1), finish_time(-1) {}

    std::string state_str() const {
        switch (state) {
            case READY:      return "就绪";
            case RUNNING:    return "运行";
            case BLOCKED:    return "阻塞";
            case TERMINATED: return "终止";
        }
        return "未知";
    }
};

struct Message {
    int from_pid;
    int to_pid;
    std::string content;
    int timestamp;

    Message() : from_pid(0), to_pid(0), timestamp(0) {}
    Message(int from, int to, const std::string& msg, int ts)
        : from_pid(from), to_pid(to), content(msg), timestamp(ts) {}
};
