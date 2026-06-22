#include "scheduler.h"
#include <iostream>

Scheduler::Scheduler(ProcessManager& pm)
    : pm_(pm), algo_(FCFS), time_slice_(2), rr_counter_(0), last_rr_pid_(-1) {}

void Scheduler::set_algorithm(ScheduleAlgorithm algo) {
    algo_ = algo;
    rr_counter_ = 0;
    last_rr_pid_ = -1;
}

void Scheduler::set_time_slice(int slice) {
    time_slice_ = slice;
}

std::string Scheduler::algorithm_name() const {
    return algo_ == FCFS ? "先来先服务(FCFS)" : "时间片轮转(RR)";
}

PCB* Scheduler::schedule() {
    if (algo_ == FCFS) return fcfs_schedule();
    return rr_schedule();
}

PCB* Scheduler::fcfs_schedule() {
    auto all = pm_.get_all_processes();
    for (auto* p : all) {
        if (p->state == RUNNING) return p;
    }
    auto ready = pm_.get_ready_queue();
    if (ready.empty()) return nullptr;
    PCB* earliest = ready[0];
    for (auto* p : ready) {
        if (p->create_time < earliest->create_time)
            earliest = p;
    }
    return earliest;
}

PCB* Scheduler::rr_schedule() {
    auto all = pm_.get_all_processes();
    for (auto* p : all) {
        if (p->state == RUNNING) return p;
    }
    auto ready = pm_.get_ready_queue();
    if (ready.empty()) return nullptr;

    PCB* next = nullptr;
    for (auto* p : ready) {
        if (p->pid > last_rr_pid_) {
            next = p;
            break;
        }
    }
    if (!next) next = ready[0];

    last_rr_pid_ = next->pid;
    return next;
}

bool Scheduler::run_one_tick() {
    PCB* proc = schedule();
    if (!proc) return false;

    if (proc->state == READY) {
        proc->state = RUNNING;
        if (proc->start_time < 0)
            proc->start_time = pm_.get_current_time();
    }

    int before = proc->remaining_time;
    proc->remaining_time--;
    pm_.advance_time(1);

    if (proc->remaining_time <= 0) {
        proc->state = TERMINATED;
        proc->finish_time = pm_.get_current_time();
        std::cout << "  [时间 " << (pm_.get_current_time() - 1) << "] "
                  << proc->name << " 运行  剩余: " << before << "->0"
                  << "  ** 完成 **" << std::endl;
        return true;
    }

    std::cout << "  [时间 " << (pm_.get_current_time() - 1) << "] "
              << proc->name << " 运行  剩余: " << before << "->" << proc->remaining_time;

    if (algo_ == RR) {
        rr_counter_++;
        if (rr_counter_ >= time_slice_) {
            rr_counter_ = 0;
            proc->state = READY;
            std::cout << "  (时间片用完，切换)";
        }
    }

    std::cout << std::endl;
    return true;
}

bool Scheduler::run_all() {
    std::cout << "\n===== 使用 " << algorithm_name() << " 调度 =====" << std::endl;
    int ticks = 0;
    while (run_one_tick()) {
        ticks++;
        if (ticks > 1000) {
            std::cout << "  超过最大时间片限制，强制停止" << std::endl;
            break;
        }
    }
    std::cout << "===== 调度完成，总用时: " << pm_.get_current_time() << " =====\n" << std::endl;
    return true;
}
