#include <stdio.h>
#include <string.h>
#include "scheduler.h"

#define N_ALGO 6

/* Chạy thuật toán thứ k trên bản sao của src, trả về số liệu trung bình. */
static void run_one(int k, const Process src[], int n, int quantum, Metrics *m)
{
    Process p[MAX_PROCESSES];
    GanttEntry g[MAX_GANTT];
    int g_len = 0;

    memcpy(p, src, (size_t)n * sizeof p[0]);
    switch (k) {
    case 0: schedule_fcfs(p, n, g, &g_len); break;
    case 1: schedule_sjf(p, n, g, &g_len); break;
    case 2: schedule_srtf(p, n, g, &g_len); break;
    case 3: schedule_priority(p, n, g, &g_len, 0); break;
    case 4: schedule_priority(p, n, g, &g_len, 1); break;
    case 5: schedule_rr(p, n, g, &g_len, quantum); break;
    }
    calc_metrics(p, n, g, g_len, m);
}

void compare_all(const Process src[], int n, int quantum)
{
    const char *name[N_ALGO] = {
        "FCFS", "SJF", "SRTF", "Priority (khong ngat)", "Priority (ngat)", "Round Robin"
    };
    Metrics m[N_ALGO];
    int best_wt = 0, best_tat = 0, best_rt = 0;

    if (n <= 0 || n > MAX_PROCESSES)
        return;
    for (int k = 0; k < N_ALGO; k++)
        run_one(k, src, n, quantum, &m[k]);

    printf("\n=== So sanh (%d process, RR quantum = %d) ===\n", n, quantum);
    printf("%-22s %8s %8s %8s %8s\n", "Thuat toan", "Avg WT", "Avg TAT", "Avg RT", "CPU %");
    for (int k = 0; k < N_ALGO; k++) {
        printf("%-22s %8.2f %8.2f %8.2f %8.2f\n", name[k], m[k].avg_waiting,
               m[k].avg_turnaround, m[k].avg_response, m[k].cpu_utilization);
        if (m[k].avg_waiting < m[best_wt].avg_waiting)
            best_wt = k;
        if (m[k].avg_turnaround < m[best_tat].avg_turnaround)
            best_tat = k;
        if (m[k].avg_response < m[best_rt].avg_response)
            best_rt = k;
    }
    /* hòa thì giữ thuật toán đứng trước trong bảng */
    printf("\nWT trung binh nho nhat : %s\n", name[best_wt]);
    printf("TAT trung binh nho nhat: %s\n", name[best_tat]);
    printf("RT trung binh nho nhat : %s\n", name[best_rt]);
}
