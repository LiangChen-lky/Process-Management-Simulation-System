#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include "process_manager.h"
#include "scheduler.h"
#include "semaphore.h"
#include "message_queue.h"
#include "cli_renderer.h"

using namespace std;
using namespace cli;

ProcessManager pm;
Scheduler scheduler(pm);
MessageQueue mq;

void show_main_menu() {
    draw_main_menu(pm.get_current_time());
}

void do_create_process() {
    draw_page_header("创建进程");
    print_info("创建新进程");
    string name = read_str("进程名称: ");
    int time = read_int("所需运行时间: ");
    PCB* p = pm.create_process(name, time);
    print_success("进程 " + p->name + "(PID=" + to_string(p->pid) + ") 创建成功");
    println();
    print_queue("就绪队列", pm.get_ready_queue(), color::GREEN);
    wait_enter();
}

void do_terminate_process() {
    draw_page_header("终止进程");
    int pid = read_int("输入要终止的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { print_error("进程不存在。"); wait_enter(); return; }
    if (pm.terminate_process(pid)) {
        print_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已终止。");
    } else {
        print_error("终止失败。");
    }
    wait_enter();
}

void do_block_process() {
    draw_page_header("阻塞进程");
    int pid = read_int("输入要阻塞的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { print_error("进程不存在。"); wait_enter(); return; }
    if (p->state != RUNNING) {
        print_warning("只能阻塞运行中的进程。当前状态: " + p->state_str());
        wait_enter();
        return;
    }
    if (pm.block_process(pid)) {
        print_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已阻塞。");
    }
    wait_enter();
}

void do_wake_process() {
    draw_page_header("唤醒进程");
    int pid = read_int("输入要唤醒的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { print_error("进程不存在。"); wait_enter(); return; }
    if (p->state != BLOCKED) {
        print_warning("只能唤醒阻塞的进程。当前状态: " + p->state_str());
        wait_enter();
        return;
    }
    if (pm.wake_process(pid)) {
        print_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已唤醒到就绪队列。");
    }
    wait_enter();
}

void do_run_schedule() {
    draw_page_header("运行调度");
    print_info("选择调度算法");
    println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "先来先服务(FCFS)" + color::RESET);
    println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "时间片轮转(RR)" + color::RESET);
    println();

    int choice = read_int("选择: ");
    if (choice == 1) {
        scheduler.set_algorithm(FCFS);
        print_info("使用 FCFS 调度算法");
    } else if (choice == 2) {
        int slice = read_int("输入时间片大小(默认2): ");
        if (slice <= 0) slice = 2;
        scheduler.set_time_slice(slice);
        scheduler.set_algorithm(RR);
        print_info("使用 RR 调度算法，时间片=" + to_string(slice));
    } else {
        print_error("无效选择。");
        wait_enter();
        return;
    }

    println();
    print_queue("就绪队列", pm.get_ready_queue(), color::GREEN);
    println();
    spinner("调度执行中...", 300);
    scheduler.run_all();
    wait_enter();
}

void demo_producer_consumer() {
    draw_page_header("生产者-消费者同步演示");
    print_info("缓冲区大小: 3，生产者生产5个产品，消费者消费5个产品");
    println();

    Semaphore mutex(1, "mutex");
    Semaphore empty_s(3, "empty");
    Semaphore full_s(0, "full");

    PCB* producer = pm.create_process("生产者", 5, 5);
    PCB* consumer = pm.create_process("消费者", 5, 5);

    int buffer = 0;
    int produced = 0, consumed = 0;

    for (int i = 0; i < 10; i++) {
        println(color::CYAN + "── 时间 " + to_string(pm.get_current_time()) + " ──" + color::RESET);

        if (producer->state == READY && produced < 5) {
            print_info("生产者尝试生产...");
            if (empty_s.P(producer)) {
                if (mutex.P(producer)) {
                    buffer++;
                    produced++;
                    println("    " + color::GREEN + icon::SPARKLE + " 生产者生产了产品 " + to_string(produced)
                           + "，缓冲区: " + color::YELLOW + to_string(buffer) + "/3" + color::RESET);
                    mutex.V();
                    full_s.V();
                }
            }
        }

        if (consumer->state == READY && consumed < 5) {
            print_info("消费者尝试消费...");
            if (full_s.P(consumer)) {
                if (mutex.P(consumer)) {
                    buffer--;
                    consumed++;
                    println("    " + color::MAGENTA + icon::SPARKLE + " 消费者消费了产品 " + to_string(consumed)
                           + "，缓冲区: " + color::YELLOW + to_string(buffer) + "/3" + color::RESET);
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
    println();
    print_success("演示完成！");
    wait_enter();
}

void demo_message_comm() {
    draw_page_header("进程通信演示 - 消息队列");
    print_info("两个进程通过消息队列交换信息");
    println();

    PCB* sender = pm.create_process("发送者", 4, 5);
    PCB* receiver = pm.create_process("接收者", 4, 5);

    vector<string> messages = {"Hello", "你好", "操作系统", "消息传递"};

    println(color::CYAN + "── 发送阶段 ──" + color::RESET);
    for (size_t i = 0; i < messages.size(); i++) {
        mq.send(sender->pid, receiver->pid, messages[i], pm.get_current_time());
        println("  " + color::GRAY + "[时间 " + to_string(pm.get_current_time()) + "]"
               + color::RESET + " " + color::GREEN + sender->name + color::RESET
               + " " + icon::ARROW + " " + color::YELLOW + receiver->name + color::RESET
               + ": " + color::CYAN + "\"" + messages[i] + "\"" + color::RESET);
        pm.advance_time(1);
    }

    println();
    println(color::CYAN + "── 接收阶段 ──" + color::RESET);
    while (mq.has_message(receiver->pid)) {
        Message msg;
        if (mq.recv(receiver->pid, msg)) {
            println("  " + color::GRAY + "[时间 " + to_string(pm.get_current_time()) + "]"
                   + color::RESET + " " + color::YELLOW + receiver->name + color::RESET
                   + " 收到来自 " + color::GREEN + sender->name + color::RESET
                   + ": " + color::CYAN + "\"" + msg.content + "\""
                   + color::RESET + color::GRAY + " (发送于时间" + to_string(msg.timestamp) + ")" + color::RESET);
        }
        pm.advance_time(1);
    }

    pm.terminate_process(sender->pid);
    pm.terminate_process(receiver->pid);
    println();
    print_success("演示完成！");
    wait_enter();
}

void do_communication() {
    draw_page_header("进程通信 - 消息队列");
    println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "手动发送消息" + color::RESET);
    println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "手动接收消息" + color::RESET);
    println("  " + color::CYAN + "3" + color::RESET + ". " + color::WHITE + "查看进程消息" + color::RESET);
    println("  " + color::MAGENTA + "4" + color::RESET + ". " + color::WHITE + "运行通信演示" + color::RESET);
    println();

    int choice = read_int("选择: ");

    if (choice == 1) {
        draw_page_header("发送消息");
        int from = read_int("发送方PID: ");
        int to = read_int("接收方PID: ");
        string msg = read_str("消息内容: ");
        if (pm.get_process(from) && pm.get_process(to)) {
            mq.send(from, to, msg, pm.get_current_time());
            print_success("消息已发送。");
        } else {
            print_error("进程不存在。");
        }
    } else if (choice == 2) {
        draw_page_header("接收消息");
        int pid = read_int("接收方PID: ");
        Message msg;
        if (mq.recv(pid, msg)) {
            print_success("收到来自PID=" + to_string(msg.from_pid) + " 的消息: \"" + msg.content
                         + "\" (时间" + to_string(msg.timestamp) + ")");
        } else {
            print_warning("没有待接收的消息。");
        }
    } else if (choice == 3) {
        draw_page_header("查看消息");
        int pid = read_int("查看进程PID: ");
        auto msgs = mq.peek_all(pid);
        if (msgs.empty()) {
            print_warning("该进程没有待接收的消息。");
        } else {
            print_info("待接收消息:");
            for (const auto& m : msgs) {
                println("    " + color::GRAY + icon::BULLET + color::RESET
                       + " 来自PID=" + color::GREEN + to_string(m.from_pid) + color::RESET
                       + ": " + color::CYAN + "\"" + m.content + "\""
                       + color::RESET + color::GRAY + " (时间" + to_string(m.timestamp) + ")" + color::RESET);
            }
        }
    } else if (choice == 4) {
        demo_message_comm();
        return;
    }
    wait_enter();
}

void do_sync_demo() {
    draw_page_header("进程同步 - PV操作");
    println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "运行生产者-消费者演示" + color::RESET);
    println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "手动P操作" + color::RESET);
    println("  " + color::CYAN + "3" + color::RESET + ". " + color::WHITE + "手动V操作" + color::RESET);
    println();

    int choice = read_int("选择: ");

    if (choice == 1) {
        demo_producer_consumer();
        return;
    } else if (choice == 2) {
        draw_page_header("手动P操作");
        int pid = read_int("进程PID: ");
        PCB* p = pm.get_process(pid);
        if (!p) { print_error("进程不存在。"); wait_enter(); return; }
        static Semaphore sem(1, "manual");
        sem.P(p);
    } else if (choice == 3) {
        draw_page_header("手动V操作");
        static Semaphore sem(1, "manual");
        sem.V();
    }
    wait_enter();
}

void do_show_processes() {
    draw_page_header("所有进程");
    print_process_table(pm.get_all_processes());
    println();
    print_queue("就绪队列", pm.get_ready_queue(), color::GREEN);
    print_queue("阻塞队列", pm.get_blocked_queue(), color::RED);
    wait_enter();
}

int main() {
    init_console();

    // 欢迎页面
    clear_screen();
    cout << endl;
    cout << color::CYAN << color::BOLD;
    cout << "  ┌────────────────────────────────────────────────────────────────────────┐" << endl;
    cout << "  │                                                                        │" << endl;
    cout << "  │                    ✦  欢迎使用进程管理模拟系统  ✦                      │" << endl;
    cout << "  │                                                                        │" << endl;
    cout << "  └────────────────────────────────────────────────────────────────────────┘" << endl;
    cout << color::RESET << endl;
    cout << "    按回车进入系统..." << flush;
    cin.get();

    while (true) {
        show_main_menu();
        int choice = read_int("请选择操作: ");

        switch (choice) {
            case 1: do_create_process(); break;
            case 2: do_terminate_process(); break;
            case 3: do_block_process(); break;
            case 4: do_wake_process(); break;
            case 5: do_run_schedule(); break;
            case 6: do_sync_demo(); break;
            case 7: do_communication(); break;
            case 8: do_show_processes(); break;
            case 0:
                clear_screen();
                cout << endl;
                cout << color::CYAN << color::BOLD;
                cout << "  ┌────────────────────────────────────────────────────────────────────────┐" << endl;
                cout << "  │                                                                        │" << endl;
                cout << "  │                      ✦  感谢使用，再见！  ✦                           │" << endl;
                cout << "  │                                                                        │" << endl;
                cout << "  └────────────────────────────────────────────────────────────────────────┘" << endl;
                cout << color::RESET << endl;
                return 0;
            default:
                print_error("无效选择，请重试。");
                wait_enter();
        }
    }
    return 0;
}
