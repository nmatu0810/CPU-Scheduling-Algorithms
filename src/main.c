#include <stdio.h>
#include "scheduler.h"

/* main tối giản M1: dữ liệu cứng, chỉ chạy FCFS. Menu và nhập liệu làm sau. */
int main(void)
{
    /* Cố ý nhập KHÔNG theo thứ tự arrival, và có khoảng CPU idle. */
    Process p[] = {
        /* pid arrival burst priority */
        { .pid = 2, .arrival_time = 3, .burst_time = 2, .priority = 1 },
        { .pid = 3, .arrival_time = 8, .burst_time = 1, .priority = 2 },
        { .pid = 1, .arrival_time = 2, .burst_time = 3, .priority = 3 },
    };
    int n = (int)(sizeof p / sizeof p[0]);
    GanttEntry g[MAX_GANTT];
    int g_len = 0;
    Metrics m;

    schedule_fcfs(p, n, g, &g_len);
    calc_metrics(p, n, g, g_len, &m);

    print_gantt(g, g_len);
    printf("\nPID  AT  BT  CT  TAT  WT  RT\n");
    for (int i = 0; i < n; i++)
        printf("P%-3d %2d  %2d  %2d  %3d  %2d  %2d\n", p[i].pid,
               p[i].arrival_time, p[i].burst_time, p[i].completion_time,
               p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
    printf("\nAvg WT=%.2f  TAT=%.2f  RT=%.2f  CPU=%.2f%%\n", m.avg_waiting,
           m.avg_turnaround, m.avg_response, m.cpu_utilization);
    return 0;
}
