#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include "process_manager.h"
#include "scheduler.h"
#include "semaphore.h"
#include "message_queue.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

ProcessManager pm;
Scheduler scheduler(pm);
MessageQueue mq;

void clear_input() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int read_int(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) { clear_input(); return val; }
        cout << "  输入无效，请重试。" << endl;
        clear_input();
    }
}

string read_str(const string& prompt) {
    string val;
    cout << prompt;
    getline(cin, val);
    return val;
}

void display_processes() {
    auto procs = pm.get_all_processes();
    if (procs.empty()) {
        cout << "\n  当前没有进程。\n" << endl;
        return;
    }
    cout << "\n+------+----------+------+----------+--------+--------+--------+" << endl;
    cout << "| PID  | 名称     | 状态 | 总时间   | 剩余   | 创建   | 完成   |" << endl;
    cout << "+------+----------+------+----------+--------+--------+--------+" << endl;
    for (auto* p : procs) {
        cout << "| " << setw(4) << p->pid << " "
             << "| " << setw(8) << left << p->name << right
             << "| " << setw(4) << p->state_str()
             << "| " << setw(8) << p->total_time
             << "| " << setw(6) << p->remaining_time
             << "| " << setw(6) << p->create_time
             << "| " << setw(6) << (p->finish_time >= 0 ? to_string(p->finish_time) : "-")
             << " |" << endl;
    }
    cout << "+------+----------+------+----------+--------+--------+--------+\n" << endl;
}

void display_ready_queue() {
    auto ready = pm.get_ready_queue();
    cout << "\n  就绪队列: ";
    if (ready.empty()) { cout << "(空)"; }
    else {
        for (auto* p : ready) cout << p->name << "(P" << p->pid << ") ";
    }
    cout << endl;
}

void display_blocked_queue() {
    auto blocked = pm.get_blocked_queue();
    cout << "  阻塞队列: ";
    if (blocked.empty()) { cout << "(空)"; }
    else {
        for (auto* p : blocked)
            cout << p->name << "(P" << p->pid << ":" << p->blocked_reason << ") ";
    }
    cout << endl;
}

void do_create_process() {
    string name = read_str("  进程名称: ");
    int time = read_int("  所需运行时间: ");
    PCB* p = pm.create_process(name, time);
    cout << "  -> 进程 " << p->name << "(PID=" << p->pid << ") 创建成功，状态: 就绪" << endl;
}

void do_terminate_process() {
    int pid = read_int("  输入要终止的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { cout << "  进程不存在。" << endl; return; }
    if (pm.terminate_process(pid)) {
        cout << "  -> 进程 " << p->name << "(PID=" << pid << ") 已终止。" << endl;
    } else {
        cout << "  终止失败。" << endl;
    }
}

void do_block_process() {
    int pid = read_int("  输入要阻塞的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { cout << "  进程不存在。" << endl; return; }
    if (p->state != RUNNING) {
        cout << "  只能阻塞运行中的进程。当前状态: " << p->state_str() << endl;
        return;
    }
    if (pm.block_process(pid)) {
        cout << "  -> 进程 " << p->name << "(PID=" << pid << ") 已阻塞。" << endl;
    }
}

void do_wake_process() {
    int pid = read_int("  输入要唤醒的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { cout << "  进程不存在。" << endl; return; }
    if (p->state != BLOCKED) {
        cout << "  只能唤醒阻塞的进程。当前状态: " << p->state_str() << endl;
        return;
    }
    if (pm.wake_process(pid)) {
        cout << "  -> 进程 " << p->name << "(PID=" << pid << ") 已唤醒到就绪队列。" << endl;
    }
}

void do_run_schedule() {
    cout << "\n  选择调度算法:" << endl;
    cout << "  1. 先来先服务(FCFS)" << endl;
    cout << "  2. 时间片轮转(RR)" << endl;
    int choice = read_int("  选择: ");
    if (choice == 1) {
        scheduler.set_algorithm(FCFS);
    } else if (choice == 2) {
        int slice = read_int("  输入时间片大小(默认2): ");
        if (slice <= 0) slice = 2;
        scheduler.set_time_slice(slice);
        scheduler.set_algorithm(RR);
    } else {
        cout << "  无效选择。" << endl;
        return;
    }

    display_ready_queue();
    cout << endl;
    scheduler.run_all();
}

void demo_producer_consumer() {
    cout << "\n===== 生产者-消费者同步演示 =====" << endl;
    cout << "  说明: 缓冲区大小为3，生产者生产5个产品，消费者消费5个产品\n" << endl;

    Semaphore mutex(1, "mutex");
    Semaphore empty_s(3, "empty");
    Semaphore full_s(0, "full");

    PCB* producer = pm.create_process("生产者", 5, 5);
    PCB* consumer = pm.create_process("消费者", 5, 5);

    int buffer = 0;
    int produced = 0, consumed = 0;

    for (int i = 0; i < 10; i++) {
        cout << "\n--- 时间 " << pm.get_current_time() << " ---" << endl;

        if (producer->state == READY && produced < 5) {
            cout << "  生产者尝试生产..." << endl;
            if (empty_s.P(producer)) {
                if (mutex.P(producer)) {
                    buffer++;
                    produced++;
                    cout << "  生产者生产了产品 " << produced
                         << "，缓冲区: " << buffer << "/3" << endl;
                    mutex.V();
                    full_s.V();
                }
            }
        }

        if (consumer->state == READY && consumed < 5) {
            cout << "  消费者尝试消费..." << endl;
            if (full_s.P(consumer)) {
                if (mutex.P(consumer)) {
                    buffer--;
                    consumed++;
                    cout << "  消费者消费了产品 " << consumed
                         << "，缓冲区: " << buffer << "/3" << endl;
                    mutex.V();
                    empty_s.V();
                }
            }
        }

        pm.advance_time(1);

        if (produced >= 5 && consumed >= 5) break;
    }

    pm.terminate_process(producer->pid);
    pm.terminate_process(consumer->pid);
    cout << "\n===== 演示结束 =====\n" << endl;
}

void demo_message_comm() {
    cout << "\n===== 进程通信演示(消息队列) =====" << endl;
    cout << "  说明: 两个进程通过消息队列交换信息\n" << endl;

    PCB* sender = pm.create_process("发送者", 4, 5);
    PCB* receiver = pm.create_process("接收者", 4, 5);

    vector<string> messages = {"Hello", "你好", "操作系统", "消息传递"};

    cout << "\n--- 发送阶段 ---" << endl;
    for (size_t i = 0; i < messages.size(); i++) {
        mq.send(sender->pid, receiver->pid, messages[i], pm.get_current_time());
        cout << "  [时间 " << pm.get_current_time() << "] "
             << sender->name << " -> " << receiver->name
             << ": \"" << messages[i] << "\"" << endl;
        pm.advance_time(1);
    }

    cout << "\n--- 接收阶段 ---" << endl;
    while (mq.has_message(receiver->pid)) {
        Message msg;
        if (mq.recv(receiver->pid, msg)) {
            cout << "  [时间 " << pm.get_current_time() << "] "
                 << receiver->name << " 收到来自 " << sender->name
                 << ": \"" << msg.content << "\" (发送于时间" << msg.timestamp << ")" << endl;
        }
        pm.advance_time(1);
    }

    pm.terminate_process(sender->pid);
    pm.terminate_process(receiver->pid);
    cout << "\n===== 演示结束 =====\n" << endl;
}

void do_communication() {
    cout << "\n  进程通信:" << endl;
    cout << "  1. 手动发送消息" << endl;
    cout << "  2. 手动接收消息" << endl;
    cout << "  3. 查看进程消息" << endl;
    cout << "  4. 运行通信演示" << endl;
    int choice = read_int("  选择: ");

    if (choice == 1) {
        int from = read_int("  发送方PID: ");
        int to = read_int("  接收方PID: ");
        string msg = read_str("  消息内容: ");
        if (pm.get_process(from) && pm.get_process(to)) {
            mq.send(from, to, msg, pm.get_current_time());
            cout << "  -> 消息已发送。" << endl;
        } else {
            cout << "  进程不存在。" << endl;
        }
    } else if (choice == 2) {
        int pid = read_int("  接收方PID: ");
        Message msg;
        if (mq.recv(pid, msg)) {
            cout << "  收到来自PID=" << msg.from_pid << " 的消息: \"" << msg.content
                 << "\" (时间" << msg.timestamp << ")" << endl;
        } else {
            cout << "  没有待接收的消息。" << endl;
        }
    } else if (choice == 3) {
        int pid = read_int("  查看进程PID: ");
        auto msgs = mq.peek_all(pid);
        if (msgs.empty()) {
            cout << "  该进程没有待接收的消息。" << endl;
        } else {
            cout << "  待接收消息:" << endl;
            for (const auto& m : msgs) {
                cout << "    来自PID=" << m.from_pid << ": \"" << m.content
                     << "\" (时间" << m.timestamp << ")" << endl;
            }
        }
    } else if (choice == 4) {
        demo_message_comm();
    }
}

void do_sync_demo() {
    cout << "\n  进程同步:" << endl;
    cout << "  1. 运行生产者-消费者演示" << endl;
    cout << "  2. 手动P操作" << endl;
    cout << "  3. 手动V操作" << endl;
    int choice = read_int("  选择: ");

    if (choice == 1) {
        demo_producer_consumer();
    } else if (choice == 2) {
        int pid = read_int("  进程PID: ");
        PCB* p = pm.get_process(pid);
        if (!p) { cout << "  进程不存在。" << endl; return; }
        static Semaphore sem(1, "manual");
        sem.P(p);
    } else if (choice == 3) {
        static Semaphore sem(1, "manual");
        sem.V();
    }
}

void display_menu() {
    cout << "========================================" << endl;
    cout << "        进程管理模拟系统" << endl;
    cout << "========================================" << endl;
    cout << "  1. 创建进程" << endl;
    cout << "  2. 终止进程" << endl;
    cout << "  3. 阻塞进程" << endl;
    cout << "  4. 唤醒进程" << endl;
    cout << "  5. 运行调度" << endl;
    cout << "  6. 进程同步(PV操作)" << endl;
    cout << "  7. 进程通信(消息队列)" << endl;
    cout << "  8. 显示所有进程" << endl;
    cout << "  0. 退出" << endl;
    cout << "========================================" << endl;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    cout << "欢迎使用进程管理模拟系统!" << endl;
    cout << "当前系统时间: " << pm.get_current_time() << "\n" << endl;

    while (true) {
        display_menu();
        int choice = read_int("请选择操作: ");

        switch (choice) {
            case 1: do_create_process(); break;
            case 2: do_terminate_process(); break;
            case 3: do_block_process(); break;
            case 4: do_wake_process(); break;
            case 5: do_run_schedule(); break;
            case 6: do_sync_demo(); break;
            case 7: do_communication(); break;
            case 8:
                display_processes();
                display_ready_queue();
                display_blocked_queue();
                break;
            case 0:
                cout << "感谢使用，再见!" << endl;
                return 0;
            default:
                cout << "  无效选择，请重试。" << endl;
        }
    }
    return 0;
}
