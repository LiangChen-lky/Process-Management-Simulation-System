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

void clear_input() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// TUI 风格的输入函数
int read_int_tui(const string& prompt) {
    int val;
    while (true) {
        set_cursor(INPUT_ROW, 1);
        cout << color::CYAN << "  " << prompt << color::RESET << flush;
        if (cin >> val) {
            clear_input();
            // 清空输入行
            set_cursor(INPUT_ROW, 1);
            cout << string(60, ' ') << flush;
            return val;
        }
        out_error("输入无效，请重试。");
        clear_input();
    }
}

string read_str_tui(const string& prompt) {
    string val;
    set_cursor(INPUT_ROW, 1);
    cout << color::CYAN << "  " << prompt << color::RESET << flush;
    getline(cin, val);
    // 清空输入行
    set_cursor(INPUT_ROW, 1);
    cout << string(60, ' ') << flush;
    return val;
}

// 等待用户按回车
void wait_for_enter() {
    set_cursor(INPUT_ROW, 1);
    cout << color::GRAY << "  [按回车继续...]" << color::RESET << flush;
    cin.get();
    // 清空输入行
    set_cursor(INPUT_ROW, 1);
    cout << string(60, ' ') << flush;
}

void display_processes() {
    auto procs = pm.get_all_processes();
    out_process_table(procs);
}

void display_ready_queue() {
    auto ready = pm.get_ready_queue();
    out_queue("就绪队列", ready, color::GREEN);
}

void display_blocked_queue() {
    auto blocked = pm.get_blocked_queue();
    out_queue("阻塞队列", blocked, color::RED);
}

void update_time_display() {
    // 行6列15是时间值的起始位置，先清除旧值再写入新值
    set_cursor(6, 15);
    cout << "      " << flush;  // 清除旧值
    set_cursor(6, 15);
    cout << color::CYAN << pm.get_current_time() << color::RESET << flush;
}

void do_create_process() {
    clear_output_area();
    out_info("创建新进程");
    string name = read_str_tui("进程名称: ");
    int time = read_int_tui("所需运行时间: ");
    PCB* p = pm.create_process(name, time);
    out_success("进程 " + p->name + "(PID=" + to_string(p->pid) + ") 创建成功");
    display_ready_queue();
    update_time_display();
    wait_for_enter();
}

void do_terminate_process() {
    clear_output_area();
    int pid = read_int_tui("输入要终止的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { out_error("进程不存在。"); wait_for_enter(); return; }
    if (pm.terminate_process(pid)) {
        out_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已终止。");
    } else {
        out_error("终止失败。");
    }
    update_time_display();
    wait_for_enter();
}

void do_block_process() {
    clear_output_area();
    int pid = read_int_tui("输入要阻塞的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { out_error("进程不存在。"); wait_for_enter(); return; }
    if (p->state != RUNNING) {
        out_warning("只能阻塞运行中的进程。当前状态: " + p->state_str());
        wait_for_enter();
        return;
    }
    if (pm.block_process(pid)) {
        out_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已阻塞。");
    }
    update_time_display();
    wait_for_enter();
}

void do_wake_process() {
    clear_output_area();
    int pid = read_int_tui("输入要唤醒的进程PID: ");
    PCB* p = pm.get_process(pid);
    if (!p) { out_error("进程不存在。"); wait_for_enter(); return; }
    if (p->state != BLOCKED) {
        out_warning("只能唤醒阻塞的进程。当前状态: " + p->state_str());
        wait_for_enter();
        return;
    }
    if (pm.wake_process(pid)) {
        out_success("进程 " + p->name + "(PID=" + to_string(pid) + ") 已唤醒到就绪队列。");
    }
    update_time_display();
    wait_for_enter();
}

void do_run_schedule() {
    clear_output_area();
    out_info("选择调度算法");
    out_println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "先来先服务(FCFS)" + color::RESET);
    out_println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "时间片轮转(RR)" + color::RESET);

    int choice = read_int_tui("选择: ");
    if (choice == 1) {
        scheduler.set_algorithm(FCFS);
        out_info("使用 FCFS 调度算法");
    } else if (choice == 2) {
        int slice = read_int_tui("输入时间片大小(默认2): ");
        if (slice <= 0) slice = 2;
        scheduler.set_time_slice(slice);
        scheduler.set_algorithm(RR);
        out_info("使用 RR 调度算法，时间片=" + to_string(slice));
    } else {
        out_error("无效选择。");
        wait_for_enter();
        return;
    }

    display_ready_queue();
    spinner("调度执行中...", 300);
    scheduler.run_all();
    update_time_display();
    wait_for_enter();
}

void demo_producer_consumer() {
    clear_output_area();
    out_info("生产者-消费者同步演示");
    out_info("缓冲区大小: 3，生产者生产5个产品，消费者消费5个产品");

    Semaphore mutex(1, "mutex");
    Semaphore empty_s(3, "empty");
    Semaphore full_s(0, "full");

    PCB* producer = pm.create_process("生产者", 5, 5);
    PCB* consumer = pm.create_process("消费者", 5, 5);

    int buffer = 0;
    int produced = 0, consumed = 0;

    for (int i = 0; i < 10; i++) {
        out_println(color::CYAN + "── 时间 " + to_string(pm.get_current_time()) + " ──" + color::RESET);

        if (producer->state == READY && produced < 5) {
            out_info("生产者尝试生产...");
            if (empty_s.P(producer)) {
                if (mutex.P(producer)) {
                    buffer++;
                    produced++;
                    out_println("    " + color::GREEN + icon::SPARKLE + " 生产者生产了产品 " + to_string(produced)
                               + "，缓冲区: " + color::YELLOW + to_string(buffer) + "/3" + color::RESET);
                    mutex.V();
                    full_s.V();
                }
            }
        }

        if (consumer->state == READY && consumed < 5) {
            out_info("消费者尝试消费...");
            if (full_s.P(consumer)) {
                if (mutex.P(consumer)) {
                    buffer--;
                    consumed++;
                    out_println("    " + color::MAGENTA + icon::SPARKLE + " 消费者消费了产品 " + to_string(consumed)
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
    out_success("演示完成！");
    update_time_display();
    wait_for_enter();
}

void demo_message_comm() {
    clear_output_area();
    out_info("进程通信演示 - 消息队列");
    out_info("两个进程通过消息队列交换信息");

    PCB* sender = pm.create_process("发送者", 4, 5);
    PCB* receiver = pm.create_process("接收者", 4, 5);

    vector<string> messages = {"Hello", "你好", "操作系统", "消息传递"};

    out_println(color::CYAN + "── 发送阶段 ──" + color::RESET);
    for (size_t i = 0; i < messages.size(); i++) {
        mq.send(sender->pid, receiver->pid, messages[i], pm.get_current_time());
        out_println("  " + color::GRAY + "[时间 " + to_string(pm.get_current_time()) + "]"
                   + color::RESET + " " + color::GREEN + sender->name + color::RESET
                   + " " + icon::ARROW + " " + color::YELLOW + receiver->name + color::RESET
                   + ": " + color::CYAN + "\"" + messages[i] + "\"" + color::RESET);
        pm.advance_time(1);
    }

    out_println(color::CYAN + "── 接收阶段 ──" + color::RESET);
    while (mq.has_message(receiver->pid)) {
        Message msg;
        if (mq.recv(receiver->pid, msg)) {
            out_println("  " + color::GRAY + "[时间 " + to_string(pm.get_current_time()) + "]"
                       + color::RESET + " " + color::YELLOW + receiver->name + color::RESET
                       + " 收到来自 " + color::GREEN + sender->name + color::RESET
                       + ": " + color::CYAN + "\"" + msg.content + "\""
                       + color::RESET + color::GRAY + " (发送于时间" + to_string(msg.timestamp) + ")" + color::RESET);
        }
        pm.advance_time(1);
    }

    pm.terminate_process(sender->pid);
    pm.terminate_process(receiver->pid);
    out_success("演示完成！");
    update_time_display();
    wait_for_enter();
}

void do_communication() {
    clear_output_area();
    out_info("进程通信 - 消息队列");
    out_println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "手动发送消息" + color::RESET);
    out_println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "手动接收消息" + color::RESET);
    out_println("  " + color::CYAN + "3" + color::RESET + ". " + color::WHITE + "查看进程消息" + color::RESET);
    out_println("  " + color::MAGENTA + "4" + color::RESET + ". " + color::WHITE + "运行通信演示" + color::RESET);

    int choice = read_int_tui("选择: ");

    if (choice == 1) {
        int from = read_int_tui("发送方PID: ");
        int to = read_int_tui("接收方PID: ");
        string msg = read_str_tui("消息内容: ");
        if (pm.get_process(from) && pm.get_process(to)) {
            mq.send(from, to, msg, pm.get_current_time());
            out_success("消息已发送。");
        } else {
            out_error("进程不存在。");
        }
    } else if (choice == 2) {
        int pid = read_int_tui("接收方PID: ");
        Message msg;
        if (mq.recv(pid, msg)) {
            out_success("收到来自PID=" + to_string(msg.from_pid) + " 的消息: \"" + msg.content
                       + "\" (时间" + to_string(msg.timestamp) + ")");
        } else {
            out_warning("没有待接收的消息。");
        }
    } else if (choice == 3) {
        int pid = read_int_tui("查看进程PID: ");
        auto msgs = mq.peek_all(pid);
        if (msgs.empty()) {
            out_warning("该进程没有待接收的消息。");
        } else {
            out_info("待接收消息:");
            for (const auto& m : msgs) {
                out_println("    " + color::GRAY + icon::BULLET + color::RESET
                           + " 来自PID=" + color::GREEN + to_string(m.from_pid) + color::RESET
                           + ": " + color::CYAN + "\"" + m.content + "\""
                           + color::RESET + color::GRAY + " (时间" + to_string(m.timestamp) + ")" + color::RESET);
            }
        }
    } else if (choice == 4) {
        demo_message_comm();
        return;
    }
    update_time_display();
    wait_for_enter();
}

void do_sync_demo() {
    clear_output_area();
    out_info("进程同步 - PV操作");
    out_println("  " + color::GREEN + "1" + color::RESET + ". " + color::WHITE + "运行生产者-消费者演示" + color::RESET);
    out_println("  " + color::YELLOW + "2" + color::RESET + ". " + color::WHITE + "手动P操作" + color::RESET);
    out_println("  " + color::CYAN + "3" + color::RESET + ". " + color::WHITE + "手动V操作" + color::RESET);

    int choice = read_int_tui("选择: ");

    if (choice == 1) {
        demo_producer_consumer();
        return;
    } else if (choice == 2) {
        int pid = read_int_tui("进程PID: ");
        PCB* p = pm.get_process(pid);
        if (!p) { out_error("进程不存在。"); wait_for_enter(); return; }
        static Semaphore sem(1, "manual");
        sem.P(p);
    } else if (choice == 3) {
        static Semaphore sem(1, "manual");
        sem.V();
    }
    update_time_display();
    wait_for_enter();
}

int main() {
    init_console();
    hide_cursor();

    // 绘制完整界面
    draw_full_screen();

    while (true) {
        int choice = read_int_tui("请选择操作: ");

        switch (choice) {
            case 1: do_create_process(); break;
            case 2: do_terminate_process(); break;
            case 3: do_block_process(); break;
            case 4: do_wake_process(); break;
            case 5: do_run_schedule(); break;
            case 6: do_sync_demo(); break;
            case 7: do_communication(); break;
            case 8:
                clear_output_area();
                display_processes();
                display_ready_queue();
                display_blocked_queue();
                wait_for_enter();
                break;
            case 0:
                clear_output_area();
                out_println("");
                out_println(color::CYAN + color::BOLD + "  感谢使用，再见!" + color::RESET);
                show_cursor();
                set_cursor(INPUT_ROW, 1);
                return 0;
            default:
                clear_output_area();
                out_error("无效选择，请重试。");
                wait_for_enter();
        }

        // 返回主菜单时刷新时间显示
        update_time_display();
    }
    return 0;
}
