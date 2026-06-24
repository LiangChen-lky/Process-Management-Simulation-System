#include "cli_renderer.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cli {

// 全局输出行位置
int g_output_row = OUTPUT_START_ROW;

void init_console() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // 设置控制台窗口大小
    SMALL_RECT windowSize = {0, 0, 79, 49};
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
    COORD bufferSize = {80, 50};
    SetConsoleScreenBufferSize(hOut, bufferSize);
#endif
}

void hide_cursor() {
    std::cout << "\033[?25l" << std::flush;
}

void show_cursor() {
    std::cout << "\033[?25h" << std::flush;
}

void set_cursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

void draw_full_screen() {
    // 清屏
    std::cout << "\033[2J\033[H";

    // 行1: 标题框顶部
    std::cout << color::CYAN << color::BOLD;
    std::cout << "┌──────────────────────────────────────────────────────────────────────────────┐" << std::endl;
    // 行2: 空行
    std::cout << "│                                                                              │" << std::endl;
    // 行3: 标题
    std::cout << "│                      ✦  进 程 管 理 模 拟 系 统  ✦                          │" << std::endl;
    // 行4: 空行
    std::cout << "│                                                                              │" << std::endl;
    // 行5: 标题框底部
    std::cout << "└──────────────────────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << color::RESET;

    // 行6: 系统时间
    std::cout << color::GRAY << "  [系统时间: " << color::CYAN << "0" << color::GRAY << "]" << color::RESET << std::endl;

    // 行7: 空行
    std::cout << std::endl;

    // 行8-17: 菜单项
    std::cout << "  " << color::CYAN << color::BOLD << "1" << color::RESET << ". " << color::WHITE << "创建进程" << color::RESET << color::GRAY << "          输入名称、运行时间" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "2" << color::RESET << ". " << color::WHITE << "终止进程" << color::RESET << color::GRAY << "          按PID终止" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "3" << color::RESET << ". " << color::WHITE << "阻塞进程" << color::RESET << color::GRAY << "          阻塞运行中的进程" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "4" << color::RESET << ". " << color::WHITE << "唤醒进程" << color::RESET << color::GRAY << "          唤醒阻塞队列中的进程" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "5" << color::RESET << ". " << color::WHITE << "运行调度" << color::RESET << color::GRAY << "          FCFS / RR 算法" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "6" << color::RESET << ". " << color::WHITE << "进程同步" << color::RESET << color::GRAY << "          PV操作 / 生产者-消费者" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "7" << color::RESET << ". " << color::WHITE << "进程通信" << color::RESET << color::GRAY << "          消息队列收发" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "8" << color::RESET << ". " << color::WHITE << "显示所有进程" << color::RESET << color::GRAY << "      查看进程状态表" << color::RESET << std::endl;

    // 行17: 空行
    std::cout << std::endl;

    // 行18: 退出选项
    std::cout << "  " << color::RED << color::BOLD << "0" << color::RESET << ". " << color::WHITE << "退出" << color::RESET << color::GRAY << "              退出系统" << color::RESET << std::endl;

    // 行19-20: 空行
    std::cout << std::endl;
    std::cout << std::endl;

    // 行21: 分隔线
    std::cout << color::GRAY << "────────────────────────────────────────────────────────────────────────────────" << color::RESET << std::endl;

    // 行22: 输出区域标题
    std::cout << color::GRAY << "  [输出区域]" << color::RESET << std::endl;

    // 行23-41: 输出区域 (空)
    for (int i = 23; i <= 41; i++) {
        std::cout << std::endl;
    }

    // 行42: 分隔线
    std::cout << color::GRAY << "────────────────────────────────────────────────────────────────────────────────" << color::RESET << std::endl;

    // 行43: 输入区域
    std::cout << color::CYAN << "  > " << color::RESET << std::flush;

    // 重置输出行位置
    g_output_row = 23;
}

void clear_output_area() {
    for (int row = 23; row <= 41; row++) {
        set_cursor(row, 1);
        std::cout << std::string(80, ' ');
    }
    g_output_row = 23;
    set_cursor(g_output_row, 1);
}

void move_to_output() {
    set_cursor(g_output_row, 1);
}

void move_to_input() {
    set_cursor(43, 1);
    std::cout << color::CYAN << "  > " << color::RESET << std::flush;
}

void out_print(const std::string& text) {
    if (g_output_row > 41) {
        clear_output_area();
    }
    set_cursor(g_output_row, 1);
    std::cout << "  " << text << std::flush;
}

void out_println(const std::string& text) {
    if (g_output_row > 41) {
        clear_output_area();
    }
    set_cursor(g_output_row, 1);
    std::cout << "  " << text << "                                        " << std::endl;
    g_output_row++;
}

void out_success(const std::string& msg) {
    out_println(color::GREEN + icon::TERMINATED + " " + msg + color::RESET);
}

void out_error(const std::string& msg) {
    out_println(color::RED + icon::BLOCKED + " " + msg + color::RESET);
}

void out_info(const std::string& msg) {
    out_println(color::CYAN + icon::BULLET + " " + msg + color::RESET);
}

void out_warning(const std::string& msg) {
    out_println(color::YELLOW + "⚠ " + msg + color::RESET);
}

std::string state_display(ProcessState state) {
    switch (state) {
        case READY:      return color::GREEN + icon::READY + " 就绪" + color::RESET;
        case RUNNING:    return color::YELLOW + color::BOLD + icon::RUNNING + " 运行" + color::RESET;
        case BLOCKED:    return color::RED + icon::BLOCKED + " 阻塞" + color::RESET;
        case TERMINATED: return color::GRAY + icon::TERMINATED + " 终止" + color::RESET;
        default:         return color::GRAY + "? 未知" + color::RESET;
    }
}

int display_width(const std::string& s) {
    int width = 0;
    for (size_t i = 0; i < s.length(); ) {
        unsigned char c = s[i];
        if (c < 0x80) { width += 1; i += 1; }
        else if ((c & 0xE0) == 0xC0) { width += 1; i += 2; }
        else if ((c & 0xF0) == 0xE0) { width += 2; i += 3; }
        else if ((c & 0xF8) == 0xF0) { width += 2; i += 4; }
        else { width += 1; i += 1; }
    }
    return width;
}

std::string pad_to_width(const std::string& s, int target_width) {
    int current = display_width(s);
    if (current >= target_width) return s;
    return s + std::string(target_width - current, ' ');
}

void out_progress_bar(int current, int total, int width) {
    if (total <= 0) {
        out_print(color::GRAY + "[" + std::string(width, ' ') + "] N/A" + color::RESET);
        return;
    }

    int progress = (current * width) / total;
    int percent = (current * 100) / total;

    std::string bar_color;
    if (percent >= 75) bar_color = color::GREEN;
    else if (percent >= 50) bar_color = color::YELLOW;
    else if (percent >= 25) bar_color = color::CYAN;
    else bar_color = color::RED;

    std::string bar = color::GRAY + "[" + color::RESET + bar_color;
    for (int i = 0; i < width; i++) {
        if (i < progress) bar += "█";
        else if (i == progress) bar += "▓";
        else bar += "░";
    }
    bar += color::RESET + color::GRAY + "] " + std::to_string(percent) + "%" + color::RESET;
    out_print(bar);
}

void out_process_table(const std::vector<PCB*>& processes) {
    if (processes.empty()) {
        out_println(color::GRAY + "当前没有进程。" + color::RESET);
        return;
    }

    out_println(color::CYAN + "┌──────┬────────┬────────┬────────┬────────┬────────┬────────┬────────────────┐" + color::RESET);
    out_println(color::CYAN + "│" + pad_to_width(" PID", 6) +
                "│" + pad_to_width(" 名称", 8) +
                "│" + pad_to_width(" 状态", 8) +
                "│" + pad_to_width(" 总时间", 8) +
                "│" + pad_to_width(" 剩余", 8) +
                "│" + pad_to_width(" 创建", 8) +
                "│" + pad_to_width(" 完成", 8) +
                "│" + pad_to_width(" 进度", 16) + "│" + color::RESET);
    out_println(color::CYAN + "├──────┼────────┼────────┼────────┼────────┼────────┼────────┼────────────────┤" + color::RESET);

    for (const auto* p : processes) {
        std::string row_color;
        switch (p->state) {
            case RUNNING:    row_color = color::YELLOW; break;
            case BLOCKED:    row_color = color::RED; break;
            case TERMINATED: row_color = color::GRAY; break;
            default:         row_color = color::RESET; break;
        }

        std::string row = row_color + "│";
        row += std::to_string(p->pid);
        row = pad_to_width(row, display_width(row) + 6 - display_width(std::to_string(p->pid)));
        row += "│" + pad_to_width(" " + p->name, 8);
        row += "│" + pad_to_width(p->state_str(), 8);
        row += "│" + pad_to_width(" " + std::to_string(p->total_time), 8);
        row += "│" + pad_to_width(" " + std::to_string(p->remaining_time), 8);
        row += "│" + pad_to_width(" " + std::to_string(p->create_time), 8);

        std::string finish = p->finish_time >= 0 ? std::to_string(p->finish_time) : "-";
        row += "│" + pad_to_width(" " + finish, 8);

        int done = p->total_time - p->remaining_time;
        int pct = p->total_time > 0 ? (done * 100) / p->total_time : 0;
        std::string progress_str = std::to_string(pct) + "%";
        row += "│" + pad_to_width(" " + progress_str, 16);
        row += "│" + color::RESET;

        out_println(row);
    }

    out_println(color::CYAN + "└──────┴────────┴────────┴────────┴────────┴────────┴────────┴────────────────┘" + color::RESET);
}

void out_queue(const std::string& name, const std::vector<PCB*>& queue, const std::string& color) {
    std::string line = color + color::BOLD + name + ": " + color::RESET;
    if (queue.empty()) {
        line += color::GRAY + "(空)" + color::RESET;
    } else {
        for (size_t i = 0; i < queue.size(); i++) {
            line += color + queue[i]->name + color::RESET;
            line += color::GRAY + "(P" + std::to_string(queue[i]->pid) + ")" + color::RESET;
            if (i < queue.size() - 1) line += " " + icon::ARROW + " ";
        }
    }
    out_println(line);
}

void spinner(const std::string& text, int duration_ms) {
    int frames = duration_ms / 80;
    for (int i = 0; i < frames; i++) {
        set_cursor(g_output_row, 1);
        std::cout << "  " << color::CYAN << SPINNER_FRAMES[i % SPINNER_FRAMES.size()]
                  << " " << text << color::RESET << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    set_cursor(g_output_row, 1);
    std::cout << std::string(60, ' ') << std::flush;
}

} // namespace cli
