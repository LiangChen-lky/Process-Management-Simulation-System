#include "cli_renderer.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cli {

void init_console() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

void hide_cursor() {
    std::cout << "\033[?25l" << std::flush;
}

void show_cursor() {
    std::cout << "\033[?25h" << std::flush;
}

void clear_screen() {
#ifdef _WIN32
    // 使用 Windows API 清屏
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
#else
    std::cout << "\033[2J\033[H" << std::flush;
#endif
}

void set_cursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

// ============ 主菜单 ============

void draw_main_menu(int current_time) {
    clear_screen();

    std::cout << color::CYAN << color::BOLD;
    std::cout << "┌──────────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│                                                                              │" << std::endl;
    std::cout << "│                      ✦  进 程 管 理 模 拟 系 统  ✦                          │" << std::endl;
    std::cout << "│                                                                              │" << std::endl;
    std::cout << "└──────────────────────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << color::RESET << std::endl;

    std::cout << color::GRAY << "  [系统时间: " << color::CYAN << current_time << color::GRAY << "]" << color::RESET << std::endl;
    std::cout << std::endl;

    std::cout << "  " << color::CYAN << color::BOLD << "1" << color::RESET << ". " << color::WHITE << "创建进程" << color::RESET << color::GRAY << "          输入名称、运行时间" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "2" << color::RESET << ". " << color::WHITE << "终止进程" << color::RESET << color::GRAY << "          按PID终止" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "3" << color::RESET << ". " << color::WHITE << "阻塞进程" << color::RESET << color::GRAY << "          阻塞运行中的进程" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "4" << color::RESET << ". " << color::WHITE << "唤醒进程" << color::RESET << color::GRAY << "          唤醒阻塞队列中的进程" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "5" << color::RESET << ". " << color::WHITE << "运行调度" << color::RESET << color::GRAY << "          FCFS / RR 算法" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "6" << color::RESET << ". " << color::WHITE << "进程同步" << color::RESET << color::GRAY << "          PV操作 / 生产者-消费者" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "7" << color::RESET << ". " << color::WHITE << "进程通信" << color::RESET << color::GRAY << "          消息队列收发" << color::RESET << std::endl;
    std::cout << "  " << color::CYAN << color::BOLD << "8" << color::RESET << ". " << color::WHITE << "显示所有进程" << color::RESET << color::GRAY << "      查看进程状态表" << color::RESET << std::endl;
    std::cout << std::endl;
    std::cout << "  " << color::RED << color::BOLD << "0" << color::RESET << ". " << color::WHITE << "退出" << color::RESET << color::GRAY << "              退出系统" << color::RESET << std::endl;
    std::cout << std::endl;

    std::cout << color::GRAY << "────────────────────────────────────────────────────────────────────────────────" << color::RESET << std::endl;
    std::cout << std::endl;
}

// ============ 子页面 ============

void draw_page_header(const std::string& title) {
    clear_screen();

    std::cout << color::CYAN << color::BOLD;
    std::cout << "┌──────────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│  " << icon::SPARKLE << " " << pad_to_width(title, 74) << "│" << std::endl;
    std::cout << "└──────────────────────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << color::RESET << std::endl;
}

void draw_page_footer() {
    std::cout << std::endl;
    std::cout << color::GRAY << "────────────────────────────────────────────────────────────────────────────────" << color::RESET << std::endl;
    std::cout << std::endl;
}

// ============ 输出函数 ============

void println(const std::string& text) {
    std::cout << "  " << text << std::endl;
}

void print_success(const std::string& msg) {
    std::cout << "  " << color::GREEN << icon::TERMINATED << " " << msg << color::RESET << std::endl;
}

void print_error(const std::string& msg) {
    std::cout << "  " << color::RED << icon::BLOCKED << " " << msg << color::RESET << std::endl;
}

void print_info(const std::string& msg) {
    std::cout << "  " << color::CYAN << icon::BULLET << " " << msg << color::RESET << std::endl;
}

void print_warning(const std::string& msg) {
    std::cout << "  " << color::YELLOW << "⚠ " << msg << color::RESET << std::endl;
}

void print_separator() {
    std::cout << color::GRAY << "  ──────────────────────────────────────────────────────────────────────────" << color::RESET << std::endl;
}

// ============ 状态显示 ============

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

void print_process_table(const std::vector<PCB*>& processes) {
    if (processes.empty()) {
        println(color::GRAY + "当前没有进程。" + color::RESET);
        return;
    }

    println(color::CYAN + "┌──────┬────────┬────────┬────────┬────────┬────────┬────────┐" + color::RESET);
    println(color::CYAN + "│" + pad_to_width(" PID", 6) +
            "│" + pad_to_width(" 名称", 8) +
            "│" + pad_to_width(" 状态", 8) +
            "│" + pad_to_width(" 总时间", 8) +
            "│" + pad_to_width(" 剩余", 8) +
            "│" + pad_to_width(" 创建", 8) +
            "│" + pad_to_width(" 完成", 8) + "│" + color::RESET);
    println(color::CYAN + "├──────┼────────┼────────┼────────┼────────┼────────┼────────┤" + color::RESET);

    for (const auto* p : processes) {
        std::string row_color;
        switch (p->state) {
            case RUNNING:    row_color = color::YELLOW; break;
            case BLOCKED:    row_color = color::RED; break;
            case TERMINATED: row_color = color::GRAY; break;
            default:         row_color = color::RESET; break;
        }

        std::string finish = p->finish_time >= 0 ? std::to_string(p->finish_time) : "-";
        println(row_color + "│" + pad_to_width(" " + std::to_string(p->pid), 6) +
                "│" + pad_to_width(" " + p->name, 8) +
                "│" + pad_to_width(p->state_str(), 8) +
                "│" + pad_to_width(" " + std::to_string(p->total_time), 8) +
                "│" + pad_to_width(" " + std::to_string(p->remaining_time), 8) +
                "│" + pad_to_width(" " + std::to_string(p->create_time), 8) +
                "│" + pad_to_width(" " + finish, 8) + "│" + color::RESET);
    }

    println(color::CYAN + "└──────┴────────┴────────┴────────┴────────┴────────┴────────┘" + color::RESET);
}

void print_queue(const std::string& name, const std::vector<PCB*>& queue, const std::string& color) {
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
    println(line);
}

// ============ 输入函数 ============

void clear_input_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int read_int(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << color::CYAN << "  " << prompt << color::RESET;
        if (std::cin >> val) {
            clear_input_buffer();
            return val;
        }
        print_error("输入无效，请重试。");
        clear_input_buffer();
    }
}

std::string read_str(const std::string& prompt) {
    std::string val;
    std::cout << color::CYAN << "  " << prompt << color::RESET;
    std::getline(std::cin, val);
    return val;
}

void wait_enter() {
    std::cout << std::endl;
    std::cout << color::GRAY << "  [按回车返回主菜单...]" << color::RESET << std::flush;
    std::cin.get();
}

// ============ 动画 ============

void spinner(const std::string& text, int duration_ms) {
    int frames = duration_ms / 80;
    for (int i = 0; i < frames; i++) {
        std::cout << "\r  " << color::CYAN << SPINNER_FRAMES[i % SPINNER_FRAMES.size()]
                  << " " << text << color::RESET << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    std::cout << "\r" << std::string(60, ' ') << "\r" << std::flush;
}

} // namespace cli
