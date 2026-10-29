#include "scheduler.h"

/*
 * Round Robin. Hàng đợi vòng chứa chỉ số process. Process lấy ra đầu hàng được
 * chạy tối đa một quantum; chưa xong thì xếp lại cuối hàng. Quy ước: process
 * mới đến đúng lúc một process hết quantum thì vào hàng TRƯỚC process vừa bị
 * ngắt. Hàng rỗng mà còn process chưa đến thì CPU rảnh tới lúc có process.
 */

/* Mỗi process nằm trong hàng nhiều nhất một lần nên MAX_PROCESSES ô là đủ. */
typedef struct {
    int item[MAX_PROCESSES];
    int head, count;
} Queue;

static void push(Queue *q, int i)
{
    q->item[(q->head + q->count) % MAX_PROCESSES] = i;
    q->count++;
}

static int pop(Queue *q)
{
    int i = q->item[q->head];
    q->head = (q->head + 1) % MAX_PROCESSES;
    q->count--;
    return i;
}

void schedule_rr(Process p[], int n, GanttEntry g[], int *g_len, int quantum)
{
    int order[MAX_PROCESSES];
    Queue q = { .head = 0, .count = 0 };
    int t = 0, done = 0, next = 0;

    *g_len = 0;
    if (n > MAX_PROCESSES || quantum < 1)   /* main đã chặn quantum < 1 */
        return;
    for (int i = 0; i < n; i++)
        p[i].remaining_time = p[i].burst_time;
    sort_by_arrival(p, n, order);

    while (done < n) {
        int i, run;

        /* đưa vào hàng mọi process đã đến tính tới thời điểm t */
        while (next < n && p[order[next]].arrival_time <= t)
            push(&q, order[next++]);

        if (q.count == 0) {
            int at = p[order[next]].arrival_time;
            if (gantt_append(g, g_len, PID_IDLE, t, at) != 0)
                return;
            t = at;
            continue;
        }

        i = pop(&q);
        run = p[i].remaining_time < quantum ? p[i].remaining_time : quantum;
        if (gantt_append(g, g_len, p[i].pid, t, t + run) != 0)
            return;
        t += run;
        p[i].remaining_time -= run;

        /* process đến trong lúc i chạy (kể cả đúng lúc hết quantum) vào trước i */
        while (next < n && p[order[next]].arrival_time <= t)
            push(&q, order[next++]);
        if (p[i].remaining_time > 0)
            push(&q, i);
        else
            done++;
    }
}
