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

// ============ TUI 核心函数 ============

void init_console();                    // 初始化控制台 (启用 VT100)
void hide_cursor();                     // 隐藏光标
void show_cursor();                     // 显示光标

// 屏幕布局常量
const int MENU_START_ROW = 1;          // 菜单起始行
const int MENU_ROWS = 20;              // 菜单占用行数
const int OUTPUT_START_ROW = 22;       // 输出区域起始行
const int OUTPUT_ROWS = 20;            // 输出区域行数
const int INPUT_ROW = 43;              // 输入行位置

// 全屏绘制
void draw_full_screen();               // 绘制完整界面框架

// 区域操作
void clear_output_area();              // 清空输出区域
void move_to_output();                 // 移动光标到输出区域
void move_to_input();                  // 移动光标到输入区域
void set_cursor(int row, int col);    // 设置光标位置

// 输出函数 (自动在输出区域内显示)
void out_print(const std::string& text);           // 普通输出
void out_println(const std::string& text = "");    // 输出并换行
void out_success(const std::string& msg);          // 成功消息
void out_error(const std::string& msg);            // 错误消息
void out_info(const std::string& msg);             // 信息消息
void out_warning(const std::string& msg);          // 警告消息

// 进程显示
std::string state_display(ProcessState state);     // 获取状态显示文本
void out_process_table(const std::vector<PCB*>& processes);
void out_queue(const std::string& name, const std::vector<PCB*>& queue, const std::string& color);
void out_progress_bar(int current, int total, int width = 20);

// 动画
void spinner(const std::string& text, int duration_ms = 500);

// 辅助
int display_width(const std::string& s);
std::string pad_to_width(const std::string& s, int target_width);

// 全局状态：当前输出行位置
extern int g_output_row;

} // namespace cli
