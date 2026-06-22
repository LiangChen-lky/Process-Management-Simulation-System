#pragma once
#include "pcb.h"
#include <vector>
#include <string>

class ProcessManager {
public:
    ProcessManager();

    PCB* create_process(const std::string& name, int total_time, int priority = 1);
    bool terminate_process(int pid);
    bool block_process(int pid, const std::string& reason = "手动阻塞");
    bool wake_process(int pid);

    PCB* get_process(int pid);
    std::vector<PCB*> get_all_processes() const;
    std::vector<PCB*> get_ready_queue() const;
    std::vector<PCB*> get_blocked_queue() const;

    int get_current_time() const { return current_time_; }
    void advance_time(int ticks = 1) { current_time_ += ticks; }
    void set_time(int t) { current_time_ = t; }

    int next_pid() { return ++pid_counter_; }

private:
    std::vector<PCB> processes_;
    int pid_counter_;
    int current_time_;
};
