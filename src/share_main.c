#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "api_catalog.h"
#include "share_assets.h"
#include "show.h"

#define AUTHOR_EMAIL "tianyu030225@gmail.com"
#define EXPORT_DIR_NAME "listandtree"
#define EXPORT_README_NAME "README.md"
#define EXPORT_ROOT_MAKEFILE_NAME "Makefile"

static void print_status_line(const char *label, const char *value)
{
    ui_streamf(stdout, UI_STREAM_QUICK, " %s%s%s\n", label, value == NULL ? "" : value, "");
}

static void set_error(char *buffer, size_t size, const char *format, const char *detail)
{
    if (buffer == NULL || size == 0U)
    {
        return;
    }

    (void)snprintf(buffer, size, format, detail == NULL ? "" : detail);
}

static bool build_absolute_path(const char *leaf, char *buffer, size_t size)
{
    char cwd[2048];

    if (buffer == NULL || size == 0U || leaf == NULL)
    {
        return false;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        (void)snprintf(buffer, size, "%s", leaf);
        return false;
    }

    (void)snprintf(buffer, size, "%s/%s", cwd, leaf);
    return true;
}

static bool ensure_export_directory(const char *path, bool *created, char *error, size_t error_size)
{
    struct stat st;

    if (created != NULL)
    {
        *created = false;
    }

    if (stat(path, &st) == 0)
    {
        if (!S_ISDIR(st.st_mode))
        {
            set_error(error, error_size, "导出路径已存在且不是目录: %s", path);
            return false;
        }

        return true;
    }

    if (mkdir(path, 0755) != 0)
    {
        set_error(error, error_size, "无法创建导出目录: %s", strerror(errno));
        return false;
    }

    if (created != NULL)
    {
        *created = true;
    }

    return true;
}

static bool write_asset_file(const char *path, const unsigned char *data, size_t size, char *error, size_t error_size)
{
    FILE *stream = fopen(path, "wb");

    if (stream == NULL)
    {
        set_error(error, error_size, "无法写入导出文件: %s", strerror(errno));
        return false;
    }

    if (size > 0U && fwrite(data, 1U, size, stream) != size)
    {
        fclose(stream);
        set_error(error, error_size, "导出文件写入不完整: %s", path);
        return false;
    }

    if (fclose(stream) != 0)
    {
        set_error(error, error_size, "无法完成文件保存: %s", path);
        return false;
    }

    return true;
}

static bool export_library_snapshot(const char *directory,
                                    size_t *written_files,
                                    char *error,
                                    size_t error_size)
{
    char path[4096];
    size_t i;

    if (written_files != NULL)
    {
        *written_files = 0U;
    }

    for (i = 0U; i < SHARE_ASSET_COUNT; ++i)
    {
        (void)snprintf(path, sizeof(path), "%s/%s", directory, SHARE_ASSETS[i].relative_path);
        if (!write_asset_file(path, SHARE_ASSETS[i].data, SHARE_ASSETS[i].size, error, error_size))
        {
            return false;
        }

        if (written_files != NULL)
        {
            (*written_files)++;
        }
    }

    return true;
}

static bool export_readme(const char *path, bool *created, char *error, size_t error_size)
{
    struct stat st;

    if (created != NULL)
    {
        *created = false;
    }

    if (stat(path, &st) == 0)
    {
        if (!S_ISREG(st.st_mode))
        {
            set_error(error, error_size, "README 路径已存在且不是普通文件: %s", path);
            return false;
        }

        return true;
    }

    if (!write_asset_file(path, SHARE_README_DATA, SHARE_README_SIZE, error, error_size))
    {
        return false;
    }

    if (created != NULL)
    {
        *created = true;
    }

    return true;
}

static bool export_root_makefile(const char *path, bool *created, char *error, size_t error_size)
{
    struct stat st;

    if (created != NULL)
    {
        *created = false;
    }

    if (stat(path, &st) == 0)
    {
        if (!S_ISREG(st.st_mode))
        {
            set_error(error, error_size, "根 Makefile 路径已存在且不是普通文件: %s", path);
            return false;
        }

        return true;
    }

    if (!write_asset_file(path, SHARE_ROOT_MAKEFILE_DATA, SHARE_ROOT_MAKEFILE_SIZE, error, error_size))
    {
        return false;
    }

    if (created != NULL)
    {
        *created = true;
    }

    return true;
}

int main(void)
{
    char export_dir_display[4096];
    char readme_display[4096];
    char makefile_display[4096];
    char error[512];
    bool dir_created = false;
    bool readme_created = false;
    bool root_makefile_created = false;
    size_t written_files = 0U;
    size_t api_count = api_catalog_total_count();

    build_absolute_path(EXPORT_DIR_NAME, export_dir_display, sizeof(export_dir_display));
    build_absolute_path(EXPORT_README_NAME, readme_display, sizeof(readme_display));
    build_absolute_path(EXPORT_ROOT_MAKEFILE_NAME, makefile_display, sizeof(makefile_display));

    ui_show_export_intro(AUTHOR_EMAIL);

    error[0] = '\0';
    if (!ensure_export_directory(EXPORT_DIR_NAME, &dir_created, error, sizeof(error)))
    {
        ui_show_section(stdout,
                        NULL,
                        "导出失败",
                        "LIST and BST 分享版未能完成导出目录检查。");
        ui_show_message(stdout, UI_TONE_ERROR, error);
        print_status_line("分享目录: ", export_dir_display);
        print_status_line("README.md: ", readme_display);
        print_status_line("根 Makefile: ", makefile_display);
        return EXIT_FAILURE;
    }

    ui_show_message(stdout,
                    dir_created ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                    dir_created ? "[1/4] 已创建导出目录。" : "[1/4] 导出目录已存在，保持不覆盖。");

    if (dir_created)
    {
        if (!export_library_snapshot(EXPORT_DIR_NAME, &written_files, error, sizeof(error)))
        {
            ui_show_section(stdout,
                            NULL,
                            "导出失败",
                            "LIST and BST 分享版未能完整写出分享库资源。");
            ui_show_message(stdout, UI_TONE_ERROR, error);
            print_status_line("分享目录: ", export_dir_display);
            print_status_line("README.md: ", readme_display);
            print_status_line("根 Makefile: ", makefile_display);
            return EXIT_FAILURE;
        }

        ui_streamf(stdout, UI_STREAM_QUICK, " [2/4] 分享库资源已写出，共 %zu 个文件。\n", written_files);
    }
    else
    {
        ui_show_message(stdout, UI_TONE_WARNING, "[2/4] 分享目录已存在，分享库资源未改写。");
    }

    if (!export_readme(EXPORT_README_NAME, &readme_created, error, sizeof(error)))
    {
        ui_show_section(stdout,
                        NULL,
                        "导出失败",
                        "LIST and BST 分享版未能写出 README.md。");
        ui_show_message(stdout, UI_TONE_ERROR, error);
        print_status_line("分享目录: ", export_dir_display);
        print_status_line("README.md: ", readme_display);
        print_status_line("根 Makefile: ", makefile_display);
        return EXIT_FAILURE;
    }

    ui_show_message(stdout,
                    readme_created ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                    readme_created ? "[3/4] README.md 已新建。" : "[3/4] README.md 已存在，未做改写。");

    if (!export_root_makefile(EXPORT_ROOT_MAKEFILE_NAME, &root_makefile_created, error, sizeof(error)))
    {
        ui_show_section(stdout,
                        NULL,
                        "导出失败",
                        "LIST and BST 分享版未能写出根 Makefile。");
        ui_show_message(stdout, UI_TONE_ERROR, error);
        print_status_line("分享目录: ", export_dir_display);
        print_status_line("README.md: ", readme_display);
        print_status_line("根 Makefile: ", makefile_display);
        return EXIT_FAILURE;
    }

    ui_show_message(stdout,
                    root_makefile_created ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                    root_makefile_created ? "[4/4] 根目录 Makefile 已新建。" : "[4/4] 根目录 Makefile 已存在，未做改写。");

    ui_show_section(stdout,
                    NULL,
                    "导出完成",
                    "LIST and BST 分享版已完成当前目录下的分享库检查与写出。");
    ui_streamf(stdout, UI_STREAM_QUICK, " 共导出 %zu 个 API 函数\n", api_count);
    ui_streamf(stdout, UI_STREAM_QUICK, " 写出库文件: %zu 个\n", dir_created ? written_files : 0U);
    print_status_line("分享目录: ", export_dir_display);
    print_status_line("README.md: ", readme_display);
    print_status_line("根 Makefile: ", makefile_display);
    return EXIT_SUCCESS;
}
