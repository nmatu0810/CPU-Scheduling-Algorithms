#include <stdio.h>
#include <string.h>
#include "scheduler.h"

typedef struct {
    const char *name;
    int available;
    Metrics metrics;
    int total_duration;
} AlgoResult;

/*
 * Chạy mọi thuật toán trên bản sao dữ liệu của src[] và in bảng so sánh hiệu năng.
 * Giữ nguyên mảng src[] ban đầu (Quy ước 4).
 * Hỗ trợ hiển thị trạng thái linh hoạt khi một số thuật toán chưa hoàn thiện.
 */
void compare_all(const Process src[], int n, int quantum)
{
    if (n <= 0) {
        printf("Khong co tien trinh de so sanh.\n");
        return;
    }

    char rr_name[64];
    snprintf(rr_name, sizeof(rr_name), "Round Robin (q = %d)", quantum);

    AlgoResult results[6];
    memset(results, 0, sizeof(results));

    results[0].name = "FCFS";
    results[1].name = "SJF (khong ngat)";
    results[2].name = "SRTF";
    results[3].name = "Priority (khong ngat)";
    results[4].name = "Priority (ngat)";
    results[5].name = rr_name;

    for (int i = 0; i < 6; i++) {
        Process p[MAX_PROCESSES];
        GanttEntry g[MAX_GANTT];
        int g_len = 0;

        memcpy(p, src, (size_t)n * sizeof(p[0]));

        switch (i) {
        case 0:
            schedule_fcfs(p, n, g, &g_len);
            break;
        case 1:
            schedule_sjf(p, n, g, &g_len);
            break;
        case 2:
            schedule_srtf(p, n, g, &g_len);
            break;
        case 3:
            schedule_priority(p, n, g, &g_len, 0);
            break;
        case 4:
            schedule_priority(p, n, g, &g_len, 1);
            break;
        case 5:
            schedule_rr(p, n, g, &g_len, quantum);
            break;
        default:
            break;
        }

        if (g_len > 0) {
            results[i].available = 1;
            calc_metrics(p, n, g, g_len, &results[i].metrics);
            results[i].total_duration = g[g_len - 1].end;
        } else {
            results[i].available = 0;
        }
    }

    printf("\n========================================================================================\n");
    printf("                         BANG SO SANH CAC THUAT TOAN LAP LICH CPU\n");
    printf("                             (Tong so process: %d, RR quantum: %d)\n", n, quantum);
    printf("========================================================================================\n");
    printf(" %-24s | %10s | %10s | %10s | %10s | %-12s\n",
           "Thuat toan", "Avg WT", "Avg TAT", "Avg RT", "CPU Util", "Trang thai");
    printf("----------------------------------------------------------------------------------------\n");

    int valid_count = 0;
    int best_wt_idx = -1;
    int best_tat_idx = -1;
    int best_rt_idx = -1;

    for (int i = 0; i < 6; i++) {
        if (results[i].available) {
            printf(" %-24s | %10.2f | %10.2f | %10.2f | %9.2f%% | %-12s\n",
                   results[i].name,
                   results[i].metrics.avg_waiting,
                   results[i].metrics.avg_turnaround,
                   results[i].metrics.avg_response,
                   results[i].metrics.cpu_utilization,
                   "San sang");

            if (valid_count == 0 || results[i].metrics.avg_waiting < results[best_wt_idx].metrics.avg_waiting)
                best_wt_idx = i;
            if (valid_count == 0 || results[i].metrics.avg_turnaround < results[best_tat_idx].metrics.avg_turnaround)
                best_tat_idx = i;
            if (valid_count == 0 || results[i].metrics.avg_response < results[best_rt_idx].metrics.avg_response)
                best_rt_idx = i;

            valid_count++;
        } else {
            printf(" %-24s | %10s | %10s | %10s | %10s | %-12s\n",
                   results[i].name, "-", "-", "-", "-", "Chua cai dat");
        }
    }
    printf("========================================================================================\n");

    if (valid_count > 0) {
        printf("\n--- Danh gia so bo ---\n");
        printf(" - Thoi gian cho trung binh (Avg WT) tot nhat     : %-22s (%.2f)\n",
               results[best_wt_idx].name, results[best_wt_idx].metrics.avg_waiting);
        printf(" - Thoi gian luu lai trung binh (Avg TAT) tot nhat: %-22s (%.2f)\n",
               results[best_tat_idx].name, results[best_tat_idx].metrics.avg_turnaround);
        printf(" - Thoi gian dap ung trung binh (Avg RT) tot nhat : %-22s (%.2f)\n",
               results[best_rt_idx].name, results[best_rt_idx].metrics.avg_response);
    }
    printf("\n");
}

