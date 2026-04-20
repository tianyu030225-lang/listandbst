#ifndef SHOW_H
#define SHOW_H

#include <stdio.h>

/**
 * @brief 界面消息语气类型
 */
typedef enum
{
    UI_TONE_INFO = 0,
    UI_TONE_SUCCESS,
    UI_TONE_WARNING,
    UI_TONE_ERROR
} UiTone;

/**
 * @brief 界面流式输出速度常量
 */
enum
{
    UI_STREAM_QUICK = 1600,
    UI_STREAM_NORMAL = 4200,
    UI_STREAM_SLOW = 9000
};

/**
 * @brief 清空终端界面
 */
void ui_clear_screen(void);

/**
 * @brief 流式输出文本
 * @param stream 输出流
 * @param text 要输出的文本
 * @param delay_us 字符间延时，单位微秒
 */
void ui_stream_text(FILE *stream, const char *text, unsigned int delay_us);

/**
 * @brief 流式输出一整行文本
 * @param stream 输出流
 * @param text 要输出的文本
 * @param delay_us 输出延时，单位微秒
 */
void ui_stream_line(FILE *stream, const char *text, unsigned int delay_us);

/**
 * @brief 按格式流式输出文本
 * @param stream 输出流
 * @param delay_us 输出延时，单位微秒
 * @param format 格式字符串
 */
void ui_streamf(FILE *stream, unsigned int delay_us, const char *format, ...);

/**
 * @brief 输出分隔线
 * @param stream 输出流
 */
void ui_show_rule(FILE *stream);

/**
 * @brief 输出分区标题
 * @param stream 输出流
 * @param eyebrow 辅助标题
 * @param title 主标题
 * @param subtitle 副标题
 */
void ui_show_section(FILE *stream,
                     const char *eyebrow,
                     const char *title,
                     const char *subtitle);

/**
 * @brief 输出消息提示
 * @param stream 输出流
 * @param tone 消息语气
 * @param message 消息内容
 */
void ui_show_message(FILE *stream, UiTone tone, const char *message);

/**
 * @brief 输出输入提示
 * @param stream 输出流
 * @param label 提示标签
 */
void ui_prompt(FILE *stream, const char *label);

/**
 * @brief 等待用户按下回车
 * @param stream 输出流
 * @param message 提示消息
 */
void ui_wait_for_enter(FILE *stream, const char *message);

/**
 * @brief 运行加载动画
 * @param stream 输出流
 * @param label 动画标签
 * @param iterations 动画迭代次数
 */
void ui_run_spinner(FILE *stream, const char *label, int iterations);

/**
 * @brief 输出开发版欢迎界面
 */
void ui_show_welcome(void);

/**
 * @brief 输出分享版导出器欢迎界面
 * @param author_email 作者邮箱
 */
void ui_show_export_intro(const char *author_email);

/**
 * @brief 输出主菜单
 * @param structure_count 数据结构数量
 * @param api_count 公开 API 总数
 * @param demo_page_count 菜单演示页数量
 * @param test_group_count 测试组数量
 */
void ui_show_main_menu(size_t structure_count,
                       size_t api_count,
                       size_t demo_page_count,
                       size_t test_group_count);

/**
 * @brief 输出退出界面
 */
void ui_show_shutdown(void);

#endif
