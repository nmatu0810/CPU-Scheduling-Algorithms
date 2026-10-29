#include "scheduler.h"

/*
 * SJF không ngắt: mỗi khi CPU rảnh, chọn process có burst ngắn nhất trong số
 * đã đến và chưa chạy, rồi cho chạy hết burst. Hòa burst thì process đến sớm
 * hơn chạy trước, hòa nữa thì theo thứ tự nhập.
 */
void schedule_sjf(Process p[], int n, GanttEntry g[], int *g_len)
{
    int t = 0, done = 0;

    *g_len = 0;
    if (n > MAX_PROCESSES)
        return;
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    while (done < n) {
        int pick = -1, next_arrival = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time == 0)
                continue;
            if (p[i].arrival_time > t) {
                if (next_arrival < 0 || p[i].arrival_time < next_arrival)
                    next_arrival = p[i].arrival_time;
                continue;
            }
            if (pick < 0 || p[i].burst_time < p[pick].burst_time ||
                (p[i].burst_time == p[pick].burst_time &&
                 p[i].arrival_time < p[pick].arrival_time))
                pick = i;   /* duyệt i tăng dần nên hòa hết thì giữ process nhập trước */
        }

        if (pick < 0) {   /* chưa process nào đến: CPU rảnh tới lúc có process */
            if (gantt_append(g, g_len, PID_IDLE, t, next_arrival) != 0)
                return;
            t = next_arrival;
            continue;
        }
        if (gantt_append(g, g_len, p[pick].pid, t, t + p[pick].burst_time) != 0)
            return;
        t += p[pick].burst_time;
        p[pick].remaining_time = 0;
        done++;
    }
}
