#include "semaphore.h"
#include <iostream>

Semaphore::Semaphore(int value, const std::string& name)
    : value_(value), name_(name) {}

bool Semaphore::P(PCB* process) {
    value_--;
    if (value_ < 0) {
        process->state = BLOCKED;
        process->blocked_reason = "等待信号量 " + name_;
        wait_queue_.push(process);
        std::cout << "  [P操作] 进程 " << process->name
                  << "(PID=" << process->pid << ") 被阻塞，等待信号量 " << name_
                  << " (value=" << value_ << ")" << std::endl;
        return false;
    }
    std::cout << "  [P操作] 进程 " << process->name
              << "(PID=" << process->pid << ") 获得信号量 " << name_
              << " (value=" << value_ << ")" << std::endl;
    return true;
}

bool Semaphore::V() {
    value_++;
    if (value_ <= 0 && !wait_queue_.empty()) {
        PCB* proc = wait_queue_.front();
        wait_queue_.pop();
        proc->state = READY;
        proc->blocked_reason.clear();
        std::cout << "  [V操作] 唤醒进程 " << proc->name
                  << "(PID=" << proc->pid << ")，信号量 " << name_
                  << " (value=" << value_ << ")" << std::endl;
        return true;
    }
    std::cout << "  [V操作] 信号量 " << name_
              << " (value=" << value_ << ")，无等待进程" << std::endl;
    return false;
}

std::vector<PCB*> Semaphore::get_waiting_processes() const {
    std::vector<PCB*> result;
    std::queue<PCB*> tmp = wait_queue_;
    while (!tmp.empty()) {
        result.push_back(tmp.front());
        tmp.pop();
    }
    return result;
}
