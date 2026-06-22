# 进程管理模拟系统

操作系统课程设计项目 —— 基于 C++ 实现的进程管理模拟系统。

## 功能特性

| 功能 | 说明 |
|------|------|
| 进程创建/终止 | 动态创建进程，设置运行时间，支持终止 |
| 进程阻塞/唤醒 | 阻塞运行中的进程，唤醒阻塞队列中的进程 |
| 调度算法 | 先来先服务(FCFS)、时间片轮转(RR) |
| 进程同步 | 信号量 + PV 操作，内置生产者-消费者演示 |
| 进程通信 | 消息队列，支持 send/recv 操作 |
| 状态显示 | 表格化展示所有进程状态、就绪队列、阻塞队列 |

## 项目结构

```
操作系统课设/
├── src/
│   ├── main.cpp              # 主程序 + 控制台菜单界面
│   ├── pcb.h                 # PCB 结构体定义（进程控制块）
│   ├── process_manager.h     # 进程管理器头文件
│   ├── process_manager.cpp   # 进程管理器实现
│   ├── scheduler.h           # 调度器头文件
│   ├── scheduler.cpp         # 调度器实现（FCFS + RR）
│   ├── semaphore.h           # 信号量头文件
│   ├── semaphore.cpp         # 信号量实现（PV 操作）
│   ├── message_queue.h       # 消息队列头文件
│   └── message_queue.cpp     # 消息队列实现
├── data/
│   ├── 进程管理模拟系统.docx   # 课程设计任务书
│   └── 课程设计安排.pptx       # 课程安排
├── process_sim.exe            # 编译产物（静态链接）
└── README.md
```

## 编译运行

### 环境要求

- MinGW-w64 GCC（已安装于 `C:\msys64\mingw64\`）
- 支持 C++17

### 编译

```bash
C:\msys64\mingw64\bin\g++.exe -std=c++17 -static -o process_sim.exe src\main.cpp src\process_manager.cpp src\scheduler.cpp src\semaphore.cpp src\message_queue.cpp
```

### 运行

```powershell
.\process_sim.exe
```

## 菜单说明

```
========================================
        进程管理模拟系统
========================================
  1. 创建进程          — 输入名称、运行时间
  2. 终止进程          — 按 PID 终止
  3. 阻塞进程          — 阻塞运行中的进程
  4. 唤醒进程          — 唤醒阻塞队列中的进程
  5. 运行调度          — 选择 FCFS 或 RR 算法执行调度
  6. 进程同步(PV操作)   — 生产者-消费者演示 / 手动 PV
  7. 进程通信(消息队列)  — 手动收发消息 / 通信演示
  8. 显示所有进程       — 查看进程状态表
  0. 退出
========================================
```

## 模块设计

### PCB（进程控制块）

```cpp
struct PCB {
    int pid;              // 进程 ID
    std::string name;     // 进程名称
    ProcessState state;   // 状态：就绪/运行/阻塞/终止
    int total_time;       // 总运行时间
    int remaining_time;   // 剩余时间
    int create_time;      // 创建时间
    int start_time;       // 首次运行时间
    int finish_time;      // 完成时间
};
```

### 调度算法

- **FCFS**：按创建时间顺序执行，先来先服务，运行中的进程不会被打断
- **RR**：按时间片轮流执行，时间片用完后进程回到就绪队列末尾

### 信号量

```cpp
Semaphore mutex(1, "互斥锁");
Semaphore empty(3, "空位");   // 缓冲区大小 3
Semaphore full(0, "产品");

mutex.P(process);  // 申请资源，不足时阻塞
mutex.V();         // 释放资源，唤醒等待进程
```

### 消息队列

```cpp
mq.send(from_pid, to_pid, "消息内容", current_time);  // 发送
mq.recv(pid, out_msg);                                 // 接收
```
