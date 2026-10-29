#include "scheduler.h"

/*
 * SRTF (SJF có ngắt): xét từng đơn vị thời gian, chạy process có thời gian còn
 * lại ngắn nhất trong số đã đến. Process mới đến mà còn lại ngắn hơn thì giành
 * CPU ngay. Hòa thời gian còn lại thì process đến sớm hơn được chạy (nên
 * process đang chạy không bị ngắt khi bằng nhau), hòa nữa thì theo thứ tự nhập.
 * Mỗi đơn vị gọi gantt_append một lần, các đơn vị liền nhau tự được gộp.
 */
void schedule_srtf(Process p[], int n, GanttEntry g[], int *g_len)
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
            if (pick < 0 || p[i].remaining_time < p[pick].remaining_time ||
                (p[i].remaining_time == p[pick].remaining_time &&
                 p[i].arrival_time < p[pick].arrival_time))
                pick = i;
        }

        if (pick < 0) {
            if (gantt_append(g, g_len, PID_IDLE, t, next_arrival) != 0)
                return;
            t = next_arrival;
            continue;
        }
        if (gantt_append(g, g_len, p[pick].pid, t, t + 1) != 0)
            return;
        t++;
        if (--p[pick].remaining_time == 0)
            done++;
    }
}
