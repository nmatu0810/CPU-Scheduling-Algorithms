#include <stdio.h>
#include <string.h>
#include "scheduler.h"

#define MAX_LINE_COLS 72

/*
 * Thêm khoảng [start, end) của pid (hoặc PID_IDLE) vào biểu đồ Gantt.
 * Tự động gộp với entry cuối nếu cùng pid và end cũ == start mới (Quy ước 2).
 * Trả về 0 nếu thành công, -1 nếu vi phạm các ràng buộc:
 *   - start >= end
 *   - start != 0 khi Gantt đang rỗng
 *   - start != g[last].end (thời gian bị hở hoặc chồng lấn)
 *   - Không thể gộp và *g_len >= MAX_GANTT
 */
int gantt_append(GanttEntry g[], int *g_len, int pid, int start, int end)
{
    if (start >= end)
        return -1;

    if (*g_len == 0) {
        if (start != 0)
            return -1;
    } else {
        GanttEntry *last = &g[*g_len - 1];
        if (start != last->end)
            return -1;

        /* Cùng tiến trình và nối tiếp thời gian -> gộp khối */
        if (last->pid == pid) {
            last->end = end;
            return 0;
        }
    }

    /* Kiểm tra giới hạn mảng Gantt trước khi thêm entry mới */
    if (*g_len >= MAX_GANTT)
        return -1;

    g[*g_len].pid = pid;
    g[*g_len].start = start;
    g[*g_len].end = end;
    (*g_len)++;
    return 0;
}

/*
 * Tính độ rộng hiển thị cho một khối Gantt.
 * Đảm bảo đủ chỗ cho nhãn tiến trình và tỷ lệ tương đối với thời gian chạy.
 */
static int calc_block_width(int pid, int dur)
{
    char label[16];
    int label_len, w;

    if (pid == PID_IDLE)
        snprintf(label, sizeof(label), "IDLE");
    else
        snprintf(label, sizeof(label), "P%d", pid);

    label_len = (int)strlen(label);
    w = dur * 2;
    if (w < label_len + 2)
        w = label_len + 2;
    if (w < 6)
        w = 6;
    if (w > 16)
        w = 16;

    return w;
}

/*
 * In biểu đồ Gantt dạng văn bản (text Gantt chart) với khung ASCII đẹp mắt.
 * Tự động xuống dòng khi biểu đồ quá dài để tránh vỡ giao diện trên console.
 */
void print_gantt(const GanttEntry g[], int g_len)
{
    if (g_len <= 0) {
        printf("(Gantt chart trong)\n");
        return;
    }

    printf("\n--- So do Gantt ---\n");

    int i = 0;
    while (i < g_len) {
        int start_idx = i;
        int line_width = 1; /* tính ký tự '+' đầu dòng */
        int widths[MAX_GANTT];

        /* Gom các khối vừa trong giới hạn MAX_LINE_COLS */
        while (i < g_len) {
            int dur = g[i].end - g[i].start;
            int w = calc_block_width(g[i].pid, dur);
            if (i > start_idx && line_width + w + 1 > MAX_LINE_COLS)
                break;
            widths[i] = w;
            line_width += w + 1;
            i++;
        }
        int end_idx = i;

        /* 1. Viền trên */
        for (int k = start_idx; k < end_idx; k++) {
            printf("+");
            for (int s = 0; s < widths[k]; s++)
                printf("-");
        }
        printf("+\n");

        /* 2. Dòng nhãn tiến trình */
        for (int k = start_idx; k < end_idx; k++) {
            char label[16];
            if (g[k].pid == PID_IDLE)
                snprintf(label, sizeof(label), "IDLE");
            else
                snprintf(label, sizeof(label), "P%d", g[k].pid);

            int len = (int)strlen(label);
            int pad_left = (widths[k] - len) / 2;
            int pad_right = widths[k] - len - pad_left;

            printf("|");
            for (int s = 0; s < pad_left; s++)
                printf(" ");
            printf("%s", label);
            for (int s = 0; s < pad_right; s++)
                printf(" ");
        }
        printf("|\n");

        /* 3. Viền dưới */
        for (int k = start_idx; k < end_idx; k++) {
            printf("+");
            for (int s = 0; s < widths[k]; s++)
                printf("-");
        }
        printf("+\n");

        /* 4. Trục mốc thời gian */
        char time_line[256];
        memset(time_line, ' ', sizeof(time_line));

        int pos = 0;
        char num_str[16];

        /* Mốc bắt đầu của dòng */
        snprintf(num_str, sizeof(num_str), "%d", g[start_idx].start);
        memcpy(&time_line[pos], num_str, strlen(num_str));

        for (int k = start_idx; k < end_idx; k++) {
            pos += widths[k] + 1;
            snprintf(num_str, sizeof(num_str), "%d", g[k].end);
            memcpy(&time_line[pos], num_str, strlen(num_str));
        }

        int last_char_pos = pos + (int)strlen(num_str);
        time_line[last_char_pos] = '\0';
        printf("%s\n\n", time_line);
    }
}

