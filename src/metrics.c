#include "scheduler.h"

/*
 * Suy ra các chỉ số hiệu năng (metrics) cho từng tiến trình từ biểu đồ Gantt:
 *   - completion_time (CT): thời điểm kết thúc entry cuối cùng của tiến trình
 *   - turnaround_time (TAT): CT - arrival_time
 *   - waiting_time (WT): TAT - burst_time
 *   - response_time (RT): thời điểm bắt đầu entry đầu tiên - arrival_time
 *
 * Đồng thời tính toán các giá trị trung bình và hiệu suất sử dụng CPU (CPU utilization).
 */
void calc_metrics(Process p[], int n, const GanttEntry g[], int g_len,
                  Metrics *out)
{
    if (!out)
        return;

    if (n <= 0) {
        out->avg_waiting = 0.0;
        out->avg_turnaround = 0.0;
        out->avg_response = 0.0;
        out->cpu_utilization = 0.0;
        return;
    }

    double sum_wt = 0.0;
    double sum_tat = 0.0;
    double sum_rt = 0.0;

    for (int i = 0; i < n; i++) {
        int first_start = -1;
        int last_end = 0;

        for (int k = 0; k < g_len; k++) {
            if (g[k].pid == p[i].pid) {
                if (first_start < 0)
                    first_start = g[k].start;
                last_end = g[k].end;
            }
        }

        if (first_start >= 0) {
            p[i].completion_time = last_end;
            p[i].turnaround_time = last_end - p[i].arrival_time;
            p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
            p[i].response_time = first_start - p[i].arrival_time;
        } else {
            p[i].completion_time = 0;
            p[i].turnaround_time = 0;
            p[i].waiting_time = 0;
            p[i].response_time = 0;
        }

        sum_wt += p[i].waiting_time;
        sum_tat += p[i].turnaround_time;
        sum_rt += p[i].response_time;
    }

    out->avg_waiting = sum_wt / n;
    out->avg_turnaround = sum_tat / n;
    out->avg_response = sum_rt / n;

    /* Tính CPU utilization: tổng thời gian CPU bận / tổng thời gian Gantt */
    int busy_time = 0;
    for (int k = 0; k < g_len; k++) {
        if (g[k].pid != PID_IDLE)
            busy_time += (g[k].end - g[k].start);
    }

    int total_time = (g_len > 0) ? g[g_len - 1].end : 0;
    out->cpu_utilization = (total_time > 0)
                               ? (100.0 * busy_time / total_time)
                               : 0.0;
}
