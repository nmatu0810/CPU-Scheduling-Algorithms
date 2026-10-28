/*
 * stubs_algo.c - bản tạm của SJF, SRTF, Priority, RR để menu build được khi
 * B và C chưa push code. Các hàm này không ghi Gantt (g_len = 0), main hiểu
 * là "chưa cài đặt".
 *
 * Không dùng weak: viết xong hàm thật mà quên xóa hàm ở đây thì link báo
 * "multiple definition". B, C: viết xong hàm nào thì xóa hàm đó ở đây, hết
 * thì xóa cả file.
 */
#include "scheduler.h"

void schedule_sjf(Process p[], int n, GanttEntry g[], int *g_len)
{
    (void)p; (void)n; (void)g;
    *g_len = 0;
}

void schedule_srtf(Process p[], int n, GanttEntry g[], int *g_len)
{
    (void)p; (void)n; (void)g;
    *g_len = 0;
}

