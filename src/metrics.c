#include <stdio.h>
#include "scheduler.h"

/*
 * Mọi chỉ số đều suy ra từ Gantt (quy ước 3), thuật toán không tự tính.
 * Với mỗi process: CT = end của ô cuối cùng của nó, RT = start ô đầu - AT,
 * TAT = CT - AT, WT = TAT - BT. Tổng thời gian chạy trong Gantt phải bằng
 * burst, nếu không là thuật toán ghi sai, in cảnh báo để dễ tìm lỗi.
 */
void calc_metrics(Process p[], int n, const GanttEntry g[], int g_len,
                  Metrics *out)
{
    double sum_wt = 0, sum_tat = 0, sum_rt = 0;
    int busy = 0;

    for (int i = 0; i < n; i++) {
        int first = -1, last_end = -1, ran = 0;

        for (int k = 0; k < g_len; k++) {
            if (g[k].pid != p[i].pid)
                continue;
            if (first < 0)
                first = g[k].start;
            last_end = g[k].end;
            ran += g[k].end - g[k].start;
        }
        if (first < 0) {
            fprintf(stderr, "Canh bao: P%d khong co trong Gantt.\n", p[i].pid);
            first = last_end = p[i].arrival_time;
        } else if (ran != p[i].burst_time) {
            fprintf(stderr, "Canh bao: P%d chay %d don vi, burst la %d.\n",
                    p[i].pid, ran, p[i].burst_time);
        }
        p[i].completion_time = last_end;
        p[i].turnaround_time = last_end - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
        p[i].response_time = first - p[i].arrival_time;
        sum_wt += p[i].waiting_time;
        sum_tat += p[i].turnaround_time;
        sum_rt += p[i].response_time;
    }

    for (int k = 0; k < g_len; k++)
        if (g[k].pid != PID_IDLE)
            busy += g[k].end - g[k].start;

    out->avg_waiting = n > 0 ? sum_wt / n : 0;
    out->avg_turnaround = n > 0 ? sum_tat / n : 0;
    out->avg_response = n > 0 ? sum_rt / n : 0;
    /* mẫu số là end của ô cuối, tính cả đoạn idle đầu Gantt (quy ước 5) */
    out->cpu_utilization = g_len > 0 ? 100.0 * busy / g[g_len - 1].end : 0;
}
