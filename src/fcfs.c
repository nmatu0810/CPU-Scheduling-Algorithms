#include "scheduler.h"

/*
 * FCFS (First Come First Served), không ngắt: process đến trước chạy trước,
 * chạy một mạch đến hết burst. Hòa arrival thì theo thứ tự nhập.
 */
void schedule_fcfs(Process p[], int n, GanttEntry g[], int *g_len)
{
    int order[MAX_PROCESSES];
    int t = 0;

    *g_len = 0;
    /* n <= MAX_PROCESSES do validate_processes đảm bảo; guard này chỉ để
     * order[] không bao giờ tràn nếu ai đó bỏ qua bước validate. */
    if (n > MAX_PROCESSES)
        return;

    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    sort_by_arrival(p, n, order);

    for (int k = 0; k < n; k++) {
        Process *pr = &p[order[k]];

        /* Khi gantt_append lỗi (chỉ xảy ra với input chưa qua validate, vd
         * tổng thời gian vượt MAX_TOTAL_TIME) ta dừng im lặng: hàm void nên
         * không có kênh báo lỗi, và input hợp lệ không bao giờ tới đây. */
        if (t < pr->arrival_time) {               /* CPU rảnh chờ process đến */
            if (gantt_append(g, g_len, PID_IDLE, t, pr->arrival_time) != 0)
                return;
            t = pr->arrival_time;
        }
        if (gantt_append(g, g_len, pr->pid, t, t + pr->burst_time) != 0)
            return;
        t += pr->burst_time;
        pr->remaining_time = 0;
    }
}
