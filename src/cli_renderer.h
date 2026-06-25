#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "pcb.h"

namespace cli {

// ANSI Color codes
namespace color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";

    const std::string BLACK   = "\033[30m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    const std::string GRAY    = "\033[90m";
}

// Status icons
namespace icon {
    const std::string READY      = "●";
    const std::string RUNNING    = "▶";
    const std::string BLOCKED    = "✖";
    const std::string TERMINATED = "✓";
    const std::string ARROW      = "→";
    const std::string BULLET     = "•";
    const std::string SPARKLE    = "✦";
}

// Spinner frames
const std::vector<std::string> SPINNER_FRAMES = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

// ============ 核心函数 ============

void init_console();
void hide_cursor();
void show_cursor();
void clear_screen();
void set_cursor(int row, int col);

// 页面绘制
void draw_main_menu(int current_time);     // 绘制主菜单
void draw_page_header(const std::string& title);  // 绘制子页面标题
void draw_page_footer();                    // 绘制子页面底部（按回车返回）

// 输出函数
void println(const std::string& text = "");
void print_success(const std::string& msg);
void print_error(const std::string& msg);
void print_info(const std::string& msg);
void print_warning(const std::string& msg);
void print_separator();

// 进程显示
std::string state_display(ProcessState state);
void print_process_table(const std::vector<PCB*>& processes);
void print_queue(const std::string& name, const std::vector<PCB*>& queue, const std::string& color);

// 输入函数
int read_int(const std::string& prompt);
std::string read_str(const std::string& prompt);
void wait_enter();  // 等待按回车

// 动画
void spinner(const std::string& text, int duration_ms = 500);

// 辅助
int display_width(const std::string& s);
std::string pad_to_width(const std::string& s, int target_width);

} // namespace cli
