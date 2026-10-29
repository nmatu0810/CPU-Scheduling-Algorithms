
#include "scheduler.h"

/*
 * SJF (Shortest Job First) - khong ngat.
 * Chon tien trinh co burst_time nho nhat trong so
 * cac tien trinh da den.
 *
 * Neu bang burst_time:
 *   1. Uu tien arrival_time nho hon.
 *   2. Neu van bang, uu tien vi tri nhap truoc.
 */
void schedule_sjf(Process p[], int n, GanttEntry g[], int *g_len)
{
    int t = 0;
    int completed = 0;

    *g_len = 0;

    if (n <= 0 || n > MAX_PROCESSES)
        return;

    /* Khoi tao lai remaining_time cho moi tien trinh. */
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    while (completed < n) {
        int best = -1;

        /* Tim tien trinh phu hop nhat tai thoi diem t. */
        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time == 0)
                continue;

            if (p[i].arrival_time > t)
                continue;

            if (best == -1 ||
                p[i].burst_time < p[best].burst_time ||
                (p[i].burst_time == p[best].burst_time &&
                 p[i].arrival_time < p[best].arrival_time)) {
                best = i;
            }
        }

        /* Khong co tien trinh san sang: CPU idle. */
        if (best == -1) {
            int next_arrival = -1;

            for (int i = 0; i < n; i++) {
                if (p[i].remaining_time > 0 &&
                    (next_arrival == -1 ||
                     p[i].arrival_time < next_arrival)) {
                    next_arrival = p[i].arrival_time;
                }
            }

            if (next_arrival == -1)
                return;

            if (gantt_append(g, g_len, PID_IDLE,
                             t, next_arrival) != 0)
                return;

            t = next_arrival;
            continue;
        }

        /* SJF khong ngat: chay het burst_time. */
        int end = t + p[best].burst_time;

        if (gantt_append(g, g_len, p[best].pid,
                         t, end) != 0)
            return;

        t = end;
        p[best].remaining_time = 0;
        completed++;
    }
}
