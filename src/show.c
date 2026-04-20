#define _DEFAULT_SOURCE

#include "show.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define UI_RESET "\033[0m"
#define UI_MINT "\033[38;5;121m"
#define UI_IVORY "\033[38;5;230m"
#define UI_SLATE "\033[38;5;244m"
#define UI_GOLD "\033[38;5;221m"
#define UI_CORAL "\033[38;5;209m"
#define UI_TEAL "\033[38;5;80m"
#define UI_ROSE "\033[38;5;217m"
#define UI_HIDE_CURSOR "\033[?25l"
#define UI_SHOW_CURSOR "\033[?25h"
#define UI_CLEAR_SCREEN "\033[2J\033[H"
#define UI_CLEAR_LINE "\033[2K\r"
#define UI_BRAND_NAME "LIST and BST"
#define UI_CONSOLE_TITLE "LIST and BST 实验室 · 控制台"
#define UI_AUTHOR_EMAIL "tianyu030225@gmail.com"

static const char *UI_SPINNER[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

static const char *UI_BANNER[] = {
    " ██╗     ██╗███████╗████████╗     █████╗ ███╗   ██╗██████╗     ██████╗ ███████╗████████╗",
    " ██║     ██║██╔════╝╚══██╔══╝    ██╔══██╗████╗  ██║██╔══██╗    ██╔══██╗██╔════╝╚══██╔══╝",
    " ██║     ██║███████╗   ██║       ███████║██╔██╗ ██║██║  ██║    ██████╔╝███████╗   ██║   ",
    " ██║     ██║╚════██║   ██║       ██╔══██║██║╚██╗██║██║  ██║    ██╔══██╗╚════██║   ██║   ",
    " ███████╗██║███████║   ██║       ██║  ██║██║ ╚████║██████╔╝    ██████╔╝███████║   ██║   ",
    " ╚══════╝╚═╝╚══════╝   ╚═╝       ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝     ╚═════╝ ╚══════╝   ╚═╝   ",
    NULL};

static int ui_header_renders = 0;

static bool ui_fast_mode_enabled(FILE *stream);
static unsigned int ui_effective_delay(FILE *stream, unsigned int requested);
static void ui_puts_with_color(FILE *stream,
                               const char *color,
                               const char *text,
                               unsigned int delay_us);
static void ui_render_welcome_frame(FILE *stream);
static void ui_render_console_frame(FILE *stream, unsigned int title_delay);
static void ui_render_farewell_frame(FILE *stream);
static void ui_render_banner_staggered(FILE *stream);
static void ui_render_banner_plain(FILE *stream, unsigned int base_delay);
static void ui_render_banner_reverse(FILE *stream);
static void ui_center_text(const char *text, int width, char *buffer, size_t size);
static void ui_show_intro_sequence(const char *mode_line,
                                   const char *spinner_one,
                                   const char *spinner_two,
                                   const char *spinner_three,
                                   const char *ready_message,
                                   const char *enter_prompt,
                                   const char *author_email);
static void ui_render_shell_header(FILE *stream,
                                   const char *page_name,
                                   const char *mode_name)
{
    FILE *out = stream == NULL ? stdout : stream;
    unsigned int banner_delay = ui_header_renders == 0 ? UI_STREAM_SLOW : 0U;
    unsigned int meta_delay = ui_header_renders == 0 ? UI_STREAM_NORMAL : 0U;
    int row;

    if (isatty(fileno(out)))
    {
        fputs(UI_CLEAR_SCREEN, out);
        fputs("\n\n", out);
        fflush(out);
    }

    for (row = 0; UI_BANNER[row] != NULL; ++row)
    {
        ui_puts_with_color(out, UI_CORAL, UI_BANNER[row], banner_delay);
        fputc('\n', out);
        fflush(out);
        if (ui_effective_delay(out, banner_delay) > 0U)
        {
            usleep(ui_header_renders == 0 ? 70000U : 18000U);
        }
    }

    fputc('\n', out);
    ui_puts_with_color(out,
                       UI_MINT,
                       " ╭──────────────────────────────────────────────────────────╮\n",
                       meta_delay);
    ui_puts_with_color(out,
                       UI_MINT,
                       " │             LIST and BST 实验室 · 控制台               │\n",
                       meta_delay);
    ui_puts_with_color(out,
                       UI_MINT,
                       " ╰──────────────────────────────────────────────────────────╯\n",
                       meta_delay);

    if ((page_name != NULL && page_name[0] != '\0') ||
        (mode_name != NULL && mode_name[0] != '\0'))
    {
        fputs(UI_SLATE, out);
        ui_streamf(out,
                   meta_delay,
                   " 页面: %s   模式: %s\n",
                   (page_name != NULL && page_name[0] != '\0') ? page_name : "未命名",
                   (mode_name != NULL && mode_name[0] != '\0') ? mode_name : "标准");
        ui_streamf(out, meta_delay, " 作者: %s\n", UI_AUTHOR_EMAIL);
        fputs(UI_RESET, out);
    }

    ui_show_rule(out);
    ui_header_renders++;
}

static unsigned int ui_effective_delay(FILE *stream, unsigned int requested)
{
    FILE *out = stream == NULL ? stdout : stream;

    if (requested == 0U || ui_fast_mode_enabled(out))
    {
        return 0U;
    }

    return requested;
}

static bool ui_fast_mode_enabled(FILE *stream)
{
    FILE *out = stream == NULL ? stdout : stream;
    const char *value = getenv("LISTANDTREE_FAST_UI");

    if (!isatty(fileno(out)))
    {
        return true;
    }

    return value != NULL && strcmp(value, "1") == 0;
}

static void ui_puts_with_color(FILE *stream,
                               const char *color,
                               const char *text,
                               unsigned int delay_us)
{
    FILE *out = stream == NULL ? stdout : stream;

    fputs(color, out);
    ui_stream_text(out, text, delay_us);
    fputs(UI_RESET, out);
}

static void ui_center_text(const char *text, int width, char *buffer, size_t size)
{
    int text_len;
    int left;
    int right;

    if (buffer == NULL || size == 0U)
    {
        return;
    }

    if (text == NULL)
    {
        text = "";
    }

    text_len = (int)strlen(text);
    if (text_len >= width)
    {
        (void)snprintf(buffer, size, "%.*s", width, text);
        return;
    }

    left = (width - text_len) / 2;
    right = width - text_len - left;
    (void)snprintf(buffer,
                   size,
                   "%*s%s%*s",
                   left,
                   "",
                   text,
                   right,
                   "");
}

static void ui_render_welcome_frame(FILE *stream)
{
    FILE *out = stream == NULL ? stdout : stream;
    char centered[96];

    ui_center_text("Welcome to LIST and BST", 54, centered, sizeof(centered));

    if (ui_fast_mode_enabled(out))
    {
        fputs(UI_MINT, out);
        ui_stream_text(out,
                       " ╭──────────────────────────────────────────────────────────╮\n",
                       UI_STREAM_QUICK);
        fputs(UI_MINT, out);
        fputs(" │", out);
        fputs(UI_GOLD, out);
        ui_stream_text(out, centered, 2600U);
        fputs(UI_MINT, out);
        fputs("│\n", out);
        ui_stream_text(out,
                       " ╰──────────────────────────────────────────────────────────╯\n",
                       UI_STREAM_QUICK);
        fputs(UI_RESET, out);
        fflush(out);
        return;
    }

    fputs(UI_MINT, out);
    ui_stream_text(out,
                   " ╭──────────────────────────────────────────────────────────╮\n",
                   UI_STREAM_QUICK);
    ui_stream_text(out,
                   " │\n",
                   UI_STREAM_QUICK);
    ui_stream_text(out,
                   " ╰──────────────────────────────────────────────────────────╯\n",
                   UI_STREAM_QUICK);
    fputs(UI_RESET, out);
    fflush(out);

    if (!ui_fast_mode_enabled(out))
    {
        usleep(90000U);
        fprintf(out, "\033[2A\r");
        fputs(UI_MINT, out);
        fputs(" │", out);
        fputs(UI_GOLD, out);
        ui_stream_text(out, centered, 900U);
        fputs(UI_MINT, out);
        fputs("│\n", out);
        fputs(UI_RESET, out);
        fprintf(out, "\033[1B\r");
        fflush(out);
    }
}

static void ui_render_console_frame(FILE *stream, unsigned int title_delay)
{
    FILE *out = stream == NULL ? stdout : stream;

    fputs(UI_MINT, out);
    ui_stream_text(out,
                   " ╭──────────────────────────────────────────────────────────╮\n",
                   UI_STREAM_QUICK);
    fputs(UI_MINT, out);
    fputs(" │", out);
    fputs(UI_RESET, out);
    fputs(UI_IVORY, out);
    ui_stream_text(out,
                   "             LIST and BST 实验室 · 控制台               ",
                   title_delay);
    fputs(UI_MINT, out);
    fputs("│\n", out);
    ui_stream_text(out,
                   " ╰──────────────────────────────────────────────────────────╯\n",
                   UI_STREAM_QUICK);
    fputs(UI_RESET, out);
    fflush(out);
}

static void ui_render_farewell_frame(FILE *stream)
{
    FILE *out = stream == NULL ? stdout : stream;
    char centered[96];

    ui_center_text("Thanks for using LIST and BST", 54, centered, sizeof(centered));

    fputs(UI_MINT, out);
    ui_stream_text(out,
                   " ╭──────────────────────────────────────────────────────────╮\n",
                   UI_STREAM_QUICK);
    fputs(UI_MINT, out);
    fputs(" │", out);
    fputs(UI_ROSE, out);
    ui_stream_text(out, centered, 900U);
    fputs(UI_MINT, out);
    fputs("│\n", out);
    fputs(UI_MINT, out);
    ui_stream_text(out,
                   " ╰──────────────────────────────────────────────────────────╯\n",
                   UI_STREAM_QUICK);
    fputs(UI_RESET, out);
    fflush(out);
}

static void ui_show_intro_sequence(const char *mode_line,
                                   const char *spinner_one,
                                   const char *spinner_two,
                                   const char *spinner_three,
                                   const char *ready_message,
                                   const char *enter_prompt,
                                   const char *author_email)
{
    char author_line[128];
    bool fast_mode = ui_fast_mode_enabled(stdout);
    unsigned int title_delay = ui_effective_delay(stdout, UI_STREAM_QUICK);

    if (isatty(fileno(stdout)) && !fast_mode)
    {
        fputs(UI_HIDE_CURSOR, stdout);
    }

    ui_header_renders = 0;
    ui_clear_screen();
    ui_render_welcome_frame(stdout);
    fputc('\n', stdout);
    if (fast_mode)
    {
        ui_render_banner_plain(stdout, 0U);
    }
    else
    {
        ui_render_banner_staggered(stdout);
    }
    fputc('\n', stdout);
    ui_render_console_frame(stdout, title_delay);
    fputc('\n', stdout);
    fputs(UI_SLATE, stdout);
    ui_stream_line(stdout, mode_line, UI_STREAM_QUICK);
    fputs(UI_RESET, stdout);
    ui_show_rule(stdout);
    (void)snprintf(author_line,
                   sizeof(author_line),
                   "作者: %s",
                   author_email == NULL ? UI_AUTHOR_EMAIL : author_email);
    ui_show_message(stdout, UI_TONE_INFO, author_line);
    if (spinner_one != NULL && spinner_one[0] != '\0')
    {
        ui_run_spinner(stdout, spinner_one, 24);
    }
    if (spinner_two != NULL && spinner_two[0] != '\0')
    {
        ui_run_spinner(stdout, spinner_two, 22);
    }
    if (spinner_three != NULL && spinner_three[0] != '\0')
    {
        ui_run_spinner(stdout, spinner_three, 18);
    }
    ui_show_message(stdout, UI_TONE_SUCCESS, ready_message);
    if (!fast_mode && enter_prompt != NULL && enter_prompt[0] != '\0')
    {
        ui_wait_for_enter(stdout, enter_prompt);
    }
    ui_header_renders = 1;

    if (isatty(fileno(stdout)) && !fast_mode)
    {
        fputs(UI_SHOW_CURSOR, stdout);
        fflush(stdout);
    }
}

static void ui_render_banner_staggered(FILE *stream)
{
    FILE *out = stream == NULL ? stdout : stream;
    size_t lengths[8];
    size_t progress[8];
    int row_count = 0;
    int row;
    int frame = 0;
    bool done = false;

    while (UI_BANNER[row_count] != NULL)
    {
        lengths[row_count] = strlen(UI_BANNER[row_count]);
        progress[row_count] = 0U;
        row_count++;
    }

    if (!isatty(fileno(out)))
    {
        fputs(UI_CORAL, out);
        for (row = 0; row < row_count; ++row)
        {
            unsigned int line_delay = 1200U + (unsigned int)(row * 280U);
            ui_stream_text(out, UI_BANNER[row], line_delay);
            fputc('\n', out);
        }
        fputs(UI_RESET, out);
        fflush(out);
        return;
    }

    for (row = 0; row < row_count; ++row)
    {
        fputc('\n', out);
    }
    fflush(out);

    while (!done)
    {
        done = true;

        for (row = 0; row < row_count; ++row)
        {
            int start_frame = row;

            if (frame >= start_frame && progress[row] < lengths[row])
            {
                progress[row]++;
            }

            if (progress[row] < lengths[row])
            {
                done = false;
            }
        }

        fprintf(out, "\033[%dA", row_count);
        fputs(UI_CORAL, out);
        for (row = 0; row < row_count; ++row)
        {
            fprintf(out, "\r%.*s\033[K\n", (int)progress[row], UI_BANNER[row]);
        }
        fputs(UI_RESET, out);
        fflush(out);

        if (!done && !ui_fast_mode_enabled(out))
        {
            usleep(32000U);
        }
        frame++;
    }

    fputs(UI_RESET, out);
    fflush(out);
}

static void ui_render_banner_plain(FILE *stream, unsigned int base_delay)
{
    FILE *out = stream == NULL ? stdout : stream;
    int row;

    fputs(UI_CORAL, out);
    for (row = 0; UI_BANNER[row] != NULL; ++row)
    {
        unsigned int delay = base_delay + (unsigned int)(row * 180U);

        ui_stream_text(out, UI_BANNER[row], delay);
        fputc('\n', out);
        fflush(out);
        if (ui_effective_delay(out, delay) > 0U)
        {
            usleep(22000U);
        }
    }
    fputs(UI_RESET, out);
    fflush(out);
}

static void ui_render_banner_reverse(FILE *stream)
{
    FILE *out = stream == NULL ? stdout : stream;
    size_t lengths[8];
    size_t progress[8];
    int row_count = 0;
    int row;
    int frame = 0;
    bool done = false;

    while (UI_BANNER[row_count] != NULL)
    {
        lengths[row_count] = strlen(UI_BANNER[row_count]);
        progress[row_count] = lengths[row_count];
        row_count++;
    }

    if (!isatty(fileno(out)) || ui_fast_mode_enabled(out))
    {
        return;
    }

    while (!done)
    {
        done = true;

        for (row = row_count - 1; row >= 0; --row)
        {
            int start_frame = (row_count - 1 - row);

            if (frame >= start_frame && progress[row] > 0U)
            {
                progress[row]--;
            }

            if (progress[row] > 0U)
            {
                done = false;
            }
        }

        fprintf(out, "\033[%dA", row_count);
        fputs(UI_CORAL, out);
        for (row = 0; row < row_count; ++row)
        {
            fprintf(out, "\r%.*s\033[K\n", (int)progress[row], UI_BANNER[row]);
        }
        fputs(UI_RESET, out);
        fflush(out);

        if (!done && !ui_fast_mode_enabled(out))
        {
            usleep(28000U);
        }
        frame++;
    }
}

static const char *ui_tone_color(UiTone tone)
{
    switch (tone)
    {
    case UI_TONE_SUCCESS:
        return UI_MINT;
    case UI_TONE_WARNING:
        return UI_GOLD;
    case UI_TONE_ERROR:
        return UI_CORAL;
    case UI_TONE_INFO:
    default:
        return UI_TEAL;
    }
}

static const char *ui_tone_icon(UiTone tone)
{
    switch (tone)
    {
    case UI_TONE_SUCCESS:
        return "◆";
    case UI_TONE_WARNING:
        return "▲";
    case UI_TONE_ERROR:
        return "✕";
    case UI_TONE_INFO:
    default:
        return "●";
    }
}

static void ui_build_meter(int used,
                           int total,
                           int width,
                           char *buffer,
                           size_t size)
{
    int filled = 0;
    int i;

    if (buffer == NULL || size == 0U)
    {
        return;
    }

    buffer[0] = '\0';

    if (total > 0)
    {
        filled = (used * width) / total;
    }

    if (filled < 0)
    {
        filled = 0;
    }
    if (filled > width)
    {
        filled = width;
    }

    (void)snprintf(buffer, size, "[");
    for (i = 0; i < width; ++i)
    {
        size_t used_len = strlen(buffer);
        if (used_len >= size - 2U)
        {
            break;
        }
        (void)snprintf(buffer + used_len, size - used_len, "%s", i < filled ? "■" : "·");
    }

    {
        size_t used_len = strlen(buffer);
        if (used_len < size - 1U)
        {
            (void)snprintf(buffer + used_len, size - used_len, "]");
        }
    }
}

void ui_clear_screen(void)
{
    if (isatty(fileno(stdout)))
    {
        fputs(UI_CLEAR_SCREEN, stdout);
        fputs("\n\n", stdout);
        fflush(stdout);
    }
}

void ui_stream_text(FILE *stream, const char *text, unsigned int delay_us)
{
    FILE *out = stream == NULL ? stdout : stream;
    unsigned int actual_delay = ui_effective_delay(out, delay_us);

    if (text == NULL)
    {
        return;
    }

    if (actual_delay == 0U)
    {
        fputs(text, out);
        fflush(out);
        return;
    }

    while (*text != '\0')
    {
        int bytes = 1;

        if (((unsigned char)*text & 0xE0U) == 0xC0U)
        {
            bytes = 2;
        }
        else if (((unsigned char)*text & 0xF0U) == 0xE0U)
        {
            bytes = 3;
        }
        else if (((unsigned char)*text & 0xF8U) == 0xF0U)
        {
            bytes = 4;
        }

        while (bytes-- > 0 && *text != '\0')
        {
            fputc(*text++, out);
        }

        fflush(out);
        if (actual_delay > 0U)
        {
            usleep(actual_delay);
        }
    }
}

void ui_stream_line(FILE *stream, const char *text, unsigned int delay_us)
{
    FILE *out = stream == NULL ? stdout : stream;

    ui_stream_text(out, text, delay_us);
    fputc('\n', out);
    fflush(out);
}

void ui_streamf(FILE *stream, unsigned int delay_us, const char *format, ...)
{
    char buffer[2048];
    va_list args;

    va_start(args, format);
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ui_stream_text(stream, buffer, delay_us);
}

void ui_show_rule(FILE *stream)
{
    ui_puts_with_color(stream,
                       UI_SLATE,
                       " ──────────────────────────────────────────────────────────────\n",
                       UI_STREAM_QUICK);
}

void ui_show_section(FILE *stream,
                     const char *eyebrow,
                     const char *title,
                     const char *subtitle)
{
    FILE *out = stream == NULL ? stdout : stream;
    unsigned int title_delay;
    unsigned int subtitle_delay;
    const char *page_name = title != NULL && title[0] != '\0' ? title : "功能页";
    const char *mode_name = "交互";

    ui_render_shell_header(out, page_name, mode_name);
    title_delay = ui_header_renders <= 1 ? UI_STREAM_NORMAL : UI_STREAM_QUICK;
    subtitle_delay = ui_header_renders <= 1 ? UI_STREAM_QUICK : 0U;
    if (eyebrow != NULL && eyebrow[0] != '\0')
    {
        fputs(UI_GOLD, out);
        ui_streamf(out, subtitle_delay, " %s\n", eyebrow);
        fputs(UI_RESET, out);
    }

    if (title != NULL)
    {
        fputs(UI_IVORY, out);
        ui_streamf(out, title_delay, " %s\n", title);
        fputs(UI_RESET, out);
    }

    if (subtitle != NULL && subtitle[0] != '\0')
    {
        fputs(UI_SLATE, out);
        ui_streamf(out, subtitle_delay, " %s\n", subtitle);
        fputs(UI_RESET, out);
    }

    ui_show_rule(out);
}

void ui_show_message(FILE *stream, UiTone tone, const char *message)
{
    FILE *out = stream == NULL ? stdout : stream;

    fputs(ui_tone_color(tone), out);
    ui_streamf(out, UI_STREAM_QUICK, " %s ", ui_tone_icon(tone));
    fputs(UI_IVORY, out);
    ui_streamf(out, UI_STREAM_QUICK, "%s\n", message == NULL ? "" : message);
    fputs(UI_RESET, out);
}

void ui_prompt(FILE *stream, const char *label)
{
    FILE *out = stream == NULL ? stdout : stream;

    fputs(UI_MINT, out);
    ui_streamf(out, UI_STREAM_QUICK, " ➜ ");
    fputs(UI_SLATE, out);
    ui_streamf(out, UI_STREAM_QUICK, "%s", label == NULL ? "" : label);
    fputs(UI_IVORY, out);
    fflush(out);
}

void ui_wait_for_enter(FILE *stream, const char *message)
{
    FILE *out = stream == NULL ? stdout : stream;
    char buffer[8];

    if (!isatty(fileno(stdin)))
    {
        return;
    }

    fputs(UI_SLATE, out);
    ui_streamf(out,
               UI_STREAM_QUICK,
               "\n %s",
               message == NULL ? "按回车继续..." : message);
    fputs(UI_RESET, out);
    fflush(out);
    (void)fgets(buffer, sizeof(buffer), stdin);
}

void ui_run_spinner(FILE *stream, const char *label, int iterations)
{
    FILE *out = stream == NULL ? stdout : stream;
    int i;

    if (ui_fast_mode_enabled(out))
    {
        ui_show_message(out, UI_TONE_INFO, label);
        return;
    }

    for (i = 0; i < iterations; ++i)
    {
        fprintf(out,
                UI_CLEAR_LINE UI_MINT " %s " UI_SLATE "%s" UI_RESET,
                UI_SPINNER[i % 10],
                label == NULL ? "" : label);
        fflush(out);
        usleep(115000);
    }

    fprintf(out,
            UI_CLEAR_LINE UI_MINT " ◆ " UI_IVORY "%s" UI_RESET "\n",
            label == NULL ? "" : label);
    fflush(out);
}

void ui_show_welcome(void)
{
    ui_show_intro_sequence(" 页面: 启动欢迎页   模式: 开发版实验室",
                           "正在装载链表实验接口",
                           "正在装载树与排序样本",
                           "正在挂载终端交互外壳",
                           "控制台已就绪，加载动画已完成。",
                           "按回车进入主菜单...",
                           UI_AUTHOR_EMAIL);
}

void ui_show_export_intro(const char *author_email)
{
    ui_show_intro_sequence(" 页面: 分享导出器   模式: 快照发布",
                           "正在校验分享版导出器",
                           "正在装载源码快照资源",
                           "正在整理 README 与构建脚本",
                           "分享版已就绪，加载动画已完成。",
                           NULL,
                           author_email == NULL ? UI_AUTHOR_EMAIL : author_email);
}

void ui_show_main_menu(size_t structure_count,
                       size_t api_count,
                       size_t demo_page_count,
                       size_t test_group_count)
{
    char meter[128];
    char line[256];
    static int visits = 0;
    unsigned int delay = visits == 0 ? UI_STREAM_NORMAL : UI_STREAM_QUICK;

    ui_render_shell_header(stdout, "主菜单", "待命");
    ui_build_meter((int)structure_count, (int)demo_page_count, 18, meter, sizeof(meter));

    ui_puts_with_color(stdout,
                       UI_IVORY,
                       " 控制台仪表盘\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_SLATE,
                       " ──────────────────────────────────────────────────────────────\n",
                       delay);
    ui_puts_with_color(stdout, UI_IVORY, " 运行快照\n", delay);
    (void)snprintf(line, sizeof(line), "   数据结构: %zu 个\n", structure_count);
    ui_puts_with_color(stdout, UI_IVORY, line, delay);
    (void)snprintf(line, sizeof(line), "   API 总数: %zu 个\n", api_count);
    ui_puts_with_color(stdout, UI_GOLD, line, delay);
    (void)snprintf(line, sizeof(line), "   演示页面: %zu 页 / 测试组: %zu 组\n", demo_page_count, test_group_count);
    ui_puts_with_color(stdout, UI_TEAL, line, delay);
    (void)snprintf(line, sizeof(line), "   结构条  : %s\n", meter);
    ui_puts_with_color(stdout, UI_ROSE, line, delay);
    ui_puts_with_color(stdout,
                       UI_SLATE,
                       " \n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_IVORY,
                       " 数据结构与算法\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_IVORY,
                       "   [1] 单向链表操作   [2] 单向循环操作   [3] 双向链表操作\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_TEAL,
                       "   [4] 双向循环操作   [5] BST 操作台   [6] 算法演示\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_SLATE,
                       " \n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_IVORY,
                       " 工具与说明\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_GOLD,
                       "   [7] API 总览   [8] 结构对比   [9] 构建测试\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_GOLD,
                       "   [10] 项目结构\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_CORAL,
                       "   [0] 退出程序\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_SLATE,
                       " ──────────────────────────────────────────────────────────────\n",
                       delay);
    ui_puts_with_color(stdout,
                       UI_SLATE,
                       "  输入编号后回车即可执行，所有结果页都会保留顶部会话栏。\n",
                       delay);
    fputc('\n', stdout);
    fflush(stdout);

    visits++;
}

void ui_show_shutdown(void)
{
    char author_line[128];

    if (isatty(fileno(stdout)))
    {
        fputs(UI_HIDE_CURSOR, stdout);
    }

    ui_clear_screen();
    ui_render_banner_plain(stdout, 900U);
    ui_render_banner_reverse(stdout);
    fputc('\n', stdout);
    ui_render_farewell_frame(stdout);
    (void)snprintf(author_line, sizeof(author_line), "作者: %s", UI_AUTHOR_EMAIL);
    ui_show_message(stdout, UI_TONE_INFO, author_line);

    if (isatty(fileno(stdout)))
    {
        fputs(UI_SHOW_CURSOR, stdout);
        fflush(stdout);
    }
}
