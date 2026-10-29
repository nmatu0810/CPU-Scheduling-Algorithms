#include "scheduler.h"

/* Trả về 1 nếu p[a] được ưu tiên hơn p[b]: priority nhỏ hơn là ưu tiên cao
 * hơn, hòa thì xét process đến sớm hơn. Hòa cả hai thì người gọi giữ process
 * có chỉ số nhỏ hơn (duyệt i tăng dần). */
static int higher(const Process p[], int a, int b)
{
    if (p[a].priority != p[b].priority)
        return p[a].priority < p[b].priority;
    return p[a].arrival_time < p[b].arrival_time;
}

/*
 * Priority. preemptive = 0: chọn process ưu tiên cao nhất trong số đã đến rồi
 * cho chạy hết burst. preemptive = 1: xét từng đơn vị thời gian, process ưu
 * tiên cao hơn vừa đến thì giành CPU ngay; bằng ưu tiên thì không ngắt vì
 * process đang chạy đến sớm hơn.
 */
void schedule_priority(Process p[], int n, GanttEntry g[], int *g_len,
                       int preemptive)
{
    int t = 0, done = 0;

    *g_len = 0;
    if (n > MAX_PROCESSES)
        return;
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    while (done < n) {
        int pick = -1, next_arrival = -1, run;

        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time == 0)
                continue;
            if (p[i].arrival_time > t) {
                if (next_arrival < 0 || p[i].arrival_time < next_arrival)
                    next_arrival = p[i].arrival_time;
                continue;
            }
            if (pick < 0 || higher(p, i, pick))
                pick = i;
        }

        if (pick < 0) {
            if (gantt_append(g, g_len, PID_IDLE, t, next_arrival) != 0)
                return;
            t = next_arrival;
            continue;
        }
        run = preemptive ? 1 : p[pick].remaining_time;
        if (gantt_append(g, g_len, p[pick].pid, t, t + run) != 0)
            return;
        t += run;
        p[pick].remaining_time -= run;
        if (p[pick].remaining_time == 0)
            done++;
    }
}
