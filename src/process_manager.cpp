#include "process_manager.h"
#include <algorithm>

ProcessManager::ProcessManager() : pid_counter_(0), current_time_(0) {}

PCB* ProcessManager::create_process(const std::string& name, int total_time, int priority) {
    PCB proc;
    proc.pid = next_pid();
    proc.name = name;
    proc.state = READY;
    proc.priority = priority;
    proc.total_time = total_time;
    proc.remaining_time = total_time;
    proc.create_time = current_time_;
    processes_.push_back(proc);
    return &processes_.back();
}

bool ProcessManager::terminate_process(int pid) {
    for (auto& p : processes_) {
        if (p.pid == pid && p.state != TERMINATED) {
            p.state = TERMINATED;
            p.finish_time = current_time_;
            p.remaining_time = 0;
            return true;
        }
    }
    return false;
}

bool ProcessManager::block_process(int pid, const std::string& reason) {
    for (auto& p : processes_) {
        if (p.pid == pid && p.state == RUNNING) {
            p.state = BLOCKED;
            p.blocked_reason = reason;
            return true;
        }
    }
    return false;
}

bool ProcessManager::wake_process(int pid) {
    for (auto& p : processes_) {
        if (p.pid == pid && p.state == BLOCKED) {
            p.state = READY;
            p.blocked_reason.clear();
            return true;
        }
    }
    return false;
}

PCB* ProcessManager::get_process(int pid) {
    for (auto& p : processes_) {
        if (p.pid == pid) return &p;
    }
    return nullptr;
}

std::vector<PCB*> ProcessManager::get_all_processes() const {
    std::vector<PCB*> result;
    for (auto& p : const_cast<std::vector<PCB>&>(processes_)) {
        result.push_back(&p);
    }
    return result;
}

std::vector<PCB*> ProcessManager::get_ready_queue() const {
    std::vector<PCB*> result;
    for (auto& p : const_cast<std::vector<PCB>&>(processes_)) {
        if (p.state == READY) result.push_back(&p);
    }
    return result;
}

std::vector<PCB*> ProcessManager::get_blocked_queue() const {
    std::vector<PCB*> result;
    for (auto& p : const_cast<std::vector<PCB>&>(processes_)) {
        if (p.state == BLOCKED) result.push_back(&p);
    }
    return result;
}
