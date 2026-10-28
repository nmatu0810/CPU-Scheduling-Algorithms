#include "scheduler.h"

/* Round Robin dung hang doi vong luu CHI SO process, khong phai PID.
 * Process den trong lat chay (ke ca dung luc het quantum) vao hang doi
 * truoc khi dua process vua chay chua xong ve cuoi hang doi.
 * Cung arrival thi theo vi tri trong p[]. */
void schedule_rr(Process p[], int n, GanttEntry g[], int *g_len, int quantum)
{
    int order[MAX_PROCESSES];
    int queue[MAX_PROCESSES];
    int head = 0, tail = 0, count = 0;
    int next = 0, completed = 0, time = 0;

    *g_len = 0;
    if (n <= 0 || n > MAX_PROCESSES)
        return;
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;
    if (quantum <= 0)
        return;

    sort_by_arrival(p, n, order);

    while (completed < n) {
        /* Neu hang doi rong, nhay den lan den tiep theo va ghi idle. */
        if (count == 0 && next < n && time < p[order[next]].arrival_time) {
            int arrival = p[order[next]].arrival_time;
            if (gantt_append(g, g_len, PID_IDLE, time, arrival) != 0)
                return;
            time = arrival;
        }

        while (next < n && p[order[next]].arrival_time <= time) {
            queue[tail] = order[next++];
            tail = (tail + 1) % MAX_PROCESSES;
            count++;
        }
        if (count == 0)
            return;

        int current = queue[head];
        head = (head + 1) % MAX_PROCESSES;
        count--;

        int duration = p[current].remaining_time < quantum
                     ? p[current].remaining_time : quantum;
        if (gantt_append(g, g_len, p[current].pid, time, time + duration) != 0)
            return;
        time += duration;
        p[current].remaining_time -= duration;

        /* Them process moi den truoc khi xep lai process vua chay. */
        while (next < n && p[order[next]].arrival_time <= time) {
            queue[tail] = order[next++];
            tail = (tail + 1) % MAX_PROCESSES;
            count++;
        }

        if (p[current].remaining_time > 0) {
            queue[tail] = current;
            tail = (tail + 1) % MAX_PROCESSES;
            count++;
        } else {
            completed++;
        }
    }
}
