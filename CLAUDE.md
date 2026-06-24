# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an **Operating System Course Design Project** (操作系统课程设计) — a C++ process management simulation system. The entire UI is in **Chinese**. It simulates core OS concepts: process lifecycle, scheduling, synchronization, and inter-process communication.

## Build & Run

**Compiler:** MinGW-w64 GCC (located at `C:\msys64\mingw64\`)

**Build command:**
```bash
C:\msys64\mingw64\bin\g++.exe -std=c++17 -static -o process_sim.exe src\main.cpp src\process_manager.cpp src\scheduler.cpp src\semaphore.cpp src\message_queue.cpp src\cli_renderer.cpp
```

**Note:** If compilation fails silently, ensure MSYS2's MinGW bin is first in PATH: `$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH`

**Run:**
```powershell
.\process_sim.exe
```

The executable `process_sim.exe` is committed to the repo as a statically-linked binary.

## Architecture

### Core Modules

| Module | Files | Responsibility |
|--------|-------|----------------|
| **PCB** | `pcb.h` | Defines `PCB` struct (process control block) and `Message` struct. All process state enums (`READY`, `RUNNING`, `BLOCKED`, `TERMINATED`) live here. |
| **ProcessManager** | `process_manager.h/.cpp` | Owns all PCBs in a `vector<PCB>`. Manages process lifecycle: create, terminate, block, wake. Maintains a global simulation clock (`current_time_`). |
| **Scheduler** | `scheduler.h/.cpp` | Takes a `ProcessManager&` reference. Implements FCFS and Round Robin (RR) scheduling. `run_one_tick()` advances one time unit; `run_all()` runs until completion. |
| **Semaphore** | `semaphore.h/.cpp` | Classic counting semaphore with P (wait) and V (signal) operations. Maintains a wait queue of `PCB*`. Blocks processes on P when value < 0, wakes them on V. |
| **MessageQueue** | `message_queue.h/.cpp` | Simple message passing between processes by PID. Supports send, recv (removes first match), peek, and clear. |
| **CLIRenderer** | `cli_renderer.h/.cpp` | Console UI rendering: ANSI colors, Unicode box-drawing tables, progress bars, spinner animations, screen clearing. Uses VT100 escape codes with Windows compatibility. |

### Data Flow

`main.cpp` declares three **global singletons**: `ProcessManager pm`, `Scheduler scheduler(pm)`, and `MessageQueue mq`. The menu-driven UI calls methods on these directly. There is no dependency injection or abstraction layer — the globals are the architecture.

### Key Design Patterns

- **PCB pointers are stable**: `ProcessManager` stores PCBs in a `vector<PCB>` and returns raw pointers (`PCB*`). These pointers remain valid because the vector only grows (processes are marked `TERMINATED`, never removed).
- **Scheduler owns the clock**: `run_one_tick()` calls `pm_.advance_time(1)` — the scheduler drives time progression, not the UI.
- **Semaphore blocks via state mutation**: `Semaphore::P()` directly sets `process->state = BLOCKED` on the PCB pointer, coupling it tightly to `ProcessManager`'s state.
- **RR scheduling uses PID ordering**: The round-robin scheduler selects the next ready process by PID > `last_rr_pid_`, not by queue position.

### Important Implementation Details

- `ProcessManager::get_all_processes()`, `get_ready_queue()`, and `get_blocked_queue()` use `const_cast` to return `vector<PCB*>` from a const method — this is a known workaround for the const-incorrect design.
- The producer-consumer demo in `main.cpp` (`demo_producer_consumer()`) creates processes and runs a fixed simulation loop — it does not use the scheduler.
- `MessageQueue::recv()` removes the **first** message for the target PID (FIFO per receiver).
- The scheduler has a safety limit of 1000 ticks to prevent infinite loops.
