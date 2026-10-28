#include "scheduler.h"

/* Priority nho hon nghia la uu tien cao hon.
 * Hoa priority: arrival som hon, roi vi tri trong mang p[].
 * Khong sap xep hay thay doi thu tu p[]. */
void schedule_priority(Process p[], int n, GanttEntry g[], int *g_len,
                       int preemptive)
{
    int time = 0;
    int completed = 0;

    *g_len = 0;
    if (n <= 0 || n > MAX_PROCESSES)
        return;
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    while (completed < n) {
        int best = -1;
        int next_arrival = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time <= 0)
                continue;

            if (p[i].arrival_time > time) {
                if (next_arrival == -1 || p[i].arrival_time < next_arrival)
                    next_arrival = p[i].arrival_time;
                continue;
            }

            /* Duyet i tang dan: hoa ca priority va arrival thi giu
             * process co vi tri nho hon trong mang. */
            if (best == -1 || p[i].priority < p[best].priority ||
                (p[i].priority == p[best].priority &&
                 p[i].arrival_time < p[best].arrival_time))
                best = i;
        }

        if (best == -1) {
            if (next_arrival == -1)
                return;
            if (gantt_append(g, g_len, PID_IDLE, time, next_arrival) != 0)
                return;
            time = next_arrival;
            continue;
        }

        /* Co ngat: danh gia lai sau moi don vi thoi gian.
         * Khong ngat: chay process da chon den khi hoan thanh.
         * gantt_append tu gop cac khoi lien tiep cung PID. */
        int duration = preemptive ? 1 : p[best].remaining_time;
        if (gantt_append(g, g_len, p[best].pid, time, time + duration) != 0)
            return;
        time += duration;
        p[best].remaining_time -= duration;
        if (p[best].remaining_time == 0)
            completed++;
    }
}
