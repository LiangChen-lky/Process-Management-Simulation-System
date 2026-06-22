#pragma once
#include "pcb.h"
#include <queue>
#include <string>
#include <vector>

class Semaphore {
public:
    explicit Semaphore(int value, const std::string& name = "");

    bool P(PCB* process);
    bool V();

    int get_value() const { return value_; }
    std::string get_name() const { return name_; }
    int waiting_count() const { return static_cast<int>(wait_queue_.size()); }

    std::vector<PCB*> get_waiting_processes() const;

private:
    int value_;
    std::string name_;
    std::queue<PCB*> wait_queue_;
};
