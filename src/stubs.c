/*
 * stubs.c - bản TẠM của các hàm nhóm E để A link và chạy được FCFS ở M1.
 *
 * KHÔNG dùng __attribute__((weak)) một cách cố ý: khi E thêm gantt.c /
 * metrics.c thật, hai định nghĩa trùng sẽ báo "multiple definition" lúc link,
 * buộc E xóa file này thay vì để stub âm thầm đè lên bản thật.
 * E: khi cài xong một hàm, xóa hàm tương ứng ở đây; xóa cả file khi hết.
 */
#include <stdio.h>
#include "scheduler.h"

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
        if (last->pid == pid) {
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

void calc_metrics(Process p[], int n, const GanttEntry g[], int g_len,
                  Metrics *out)
{
    int busy = 0;
    double sw = 0, st = 0, sr = 0;

    for (int i = 0; i < n; i++) {
        int first = -1, last_end = 0;
        for (int k = 0; k < g_len; k++) {
            if (g[k].pid != p[i].pid)
                continue;
            if (first < 0)
                first = g[k].start;
            last_end = g[k].end;
        }
        p[i].completion_time = last_end;
        p[i].turnaround_time = last_end - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
        p[i].response_time = (first < 0 ? 0 : first) - p[i].arrival_time;
        sw += p[i].waiting_time;
        st += p[i].turnaround_time;
        sr += p[i].response_time;
    }
    for (int k = 0; k < g_len; k++)
        if (g[k].pid != PID_IDLE)
            busy += g[k].end - g[k].start;

    out->avg_waiting = n ? sw / n : 0;
    out->avg_turnaround = n ? st / n : 0;
    out->avg_response = n ? sr / n : 0;
    out->cpu_utilization =
        g_len ? 100.0 * busy / g[g_len - 1].end : 0;
}

void print_gantt(const GanttEntry g[], int g_len)
{
    for (int k = 0; k < g_len; k++) {
        if (g[k].pid == PID_IDLE)
            printf("[%d idle %d] ", g[k].start, g[k].end);
        else
            printf("[%d P%d %d] ", g[k].start, g[k].pid, g[k].end);
    }
    printf("\n");
}

void compare_all(const Process src[], int n, int quantum)
{
    (void)src; (void)n; (void)quantum;
    fprintf(stderr, "compare_all: chua cai dat (stub)\n");
}
