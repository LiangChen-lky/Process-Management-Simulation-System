#pragma once
#include "process_manager.h"
#include <string>

enum ScheduleAlgorithm {
    FCFS,
    RR
};

class Scheduler {
public:
    explicit Scheduler(ProcessManager& pm);

    void set_algorithm(ScheduleAlgorithm algo);
    void set_time_slice(int slice);

    PCB* schedule();
    bool run_one_tick();
    bool run_all();

    std::string algorithm_name() const;

private:
    ProcessManager& pm_;
    ScheduleAlgorithm algo_;
    int time_slice_;
    int rr_counter_;
    int last_rr_pid_;

    PCB* fcfs_schedule();
    PCB* rr_schedule();
};
