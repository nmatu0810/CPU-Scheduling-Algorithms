#include <stdio.h>
#include <string.h>
#include "scheduler.h"

#define ROW_WIDTH 72   /* in Gantt quá dài thì xuống dòng */
#define CELL_MAX  12   /* ô dài nhất, để process chạy lâu không kéo dài cả dòng */

int gantt_append(GanttEntry g[], int *g_len, int pid, int start, int end)
{
    if (start >= end || *g_len >= MAX_GANTT)
        return -1;
    if (*g_len == 0) {
        if (start != 0)
            return -1;
    } else {
        GanttEntry *last = &g[*g_len - 1];
        if (start != last->end)
            return -1;
        if (last->pid == pid) {     /* cùng process chạy tiếp thì nối vào ô cũ */
            last->end = end;
            return 0;
        }
    }
    g[*g_len].pid = pid;
    g[*g_len].start = start;
    g[*g_len].end = end;
    (*g_len)++;
    return 0;
}

static void label_of(const GanttEntry *e, char *buf, size_t size)
{
    if (e->pid == PID_IDLE)
        snprintf(buf, size, "idle");
    else
        snprintf(buf, size, "P%d", e->pid);
}

/* Độ rộng ô: tỉ lệ với thời gian chạy nhưng có giới hạn, và đủ chỗ cho nhãn
 * cùng số thời điểm kết thúc in bên dưới. */
static int cell_width(const GanttEntry *e)
{
    char label[16], num[16];
    int w = (e->end - e->start) * 2;

    label_of(e, label, sizeof label);
    snprintf(num, sizeof num, "%d", e->end);
    if (w > CELL_MAX)
        w = CELL_MAX;
    if (w < (int)strlen(label) + 2)
        w = (int)strlen(label) + 2;
    if (w < (int)strlen(num) + 1)
        w = (int)strlen(num) + 1;
    return w;
}

/* In các ô từ from đến to - 1 thành 4 dòng: viền, nhãn, viền, mốc thời gian. */
static void print_row(const GanttEntry g[], int from, int to)
{
    char label[16];

    for (int k = from; k < to; k++) {
        int w = cell_width(&g[k]);
        putchar('+');
        for (int c = 0; c < w; c++)
            putchar('-');
    }
    printf("+\n");

    for (int k = from; k < to; k++) {
        int w = cell_width(&g[k]);
        int len, left;

        label_of(&g[k], label, sizeof label);
        len = (int)strlen(label);
        left = (w - len) / 2;
        printf("|%*s%s%*s", left, "", label, w - len - left, "");
    }
    printf("|\n");

    for (int k = from; k < to; k++) {
        int w = cell_width(&g[k]);
        putchar('+');
        for (int c = 0; c < w; c++)
            putchar('-');
    }
    printf("+\n");

    for (int k = from; k < to; k++)
        printf("%-*d", cell_width(&g[k]) + 1, g[k].start);
    printf("%d\n", g[to - 1].end);
}

void print_gantt(const GanttEntry g[], int g_len)
{
    int from = 0;

    if (g_len <= 0) {
        printf("(Gantt rong)\n");
        return;
    }
    while (from < g_len) {
        int to = from, used = 1;

        while (to < g_len && (to == from || used + cell_width(&g[to]) + 1 <= ROW_WIDTH)) {
            used += cell_width(&g[to]) + 1;
            to++;
        }
        print_row(g, from, to);
        from = to;
        if (from < g_len)
            printf("\n");
    }
}
