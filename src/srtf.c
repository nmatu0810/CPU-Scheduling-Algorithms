
#include "scheduler.h"

/*
 * SRTF (Shortest Remaining Time First) - co ngat.
 *
 * Tai moi don vi thoi gian:
 * - Chon process da den co remaining_time nho nhat.
 * - Hoa remaining_time: uu tien arrival_time nho hon.
 * - Hoa tiep: uu tien vi tri nhap truoc.
 *
 * Khong thay doi thu tu p[].
 * Gantt duoc ghi bang gantt_append().
 */
void schedule_srtf(Process p[], int n, GanttEntry g[], int *g_len)
{
    int t = 0;
    int completed = 0;

    *g_len = 0;

    if (n <= 0 || n > MAX_PROCESSES)
        return;

    /* Dat lai thoi gian con lai cho moi process. */
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;

    while (completed < n) {
        int best = -1;

        /* Tim process da den co remaining_time nho nhat. */
        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time <= 0)
                continue;

            if (p[i].arrival_time > t)
                continue;

            if (best == -1 ||
                p[i].remaining_time < p[best].remaining_time ||
                (p[i].remaining_time == p[best].remaining_time &&
                 p[i].arrival_time < p[best].arrival_time)) {
                best = i;
            }
        }

        /* CPU chua co process nao san sang. */
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

        /* Chay process duoc chon trong 1 don vi thoi gian. */
        if (gantt_append(g, g_len, p[best].pid,
                         t, t + 1) != 0)
            return;

        p[best].remaining_time--;
        t++;

        /* Process hoan thanh khi remaining_time = 0. */
        if (p[best].remaining_time == 0)
            completed++;
    }
}
