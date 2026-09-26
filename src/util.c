#include "scheduler.h"

/* Insertion sort trên mảng chỉ số: ổn định, n <= MAX_PROCESSES nên O(n^2) đủ. */
void sort_by_arrival(const Process p[], int n, int order[])
{
    for (int i = 0; i < n; i++)
        order[i] = i;

    for (int i = 1; i < n; i++) {
        int cur = order[i];
        int j = i - 1;
        /* Chỉ dịch khi arrival LỚN HƠN hẳn: hòa thì giữ nguyên thứ tự nhập. */
        while (j >= 0 && p[order[j]].arrival_time > p[cur].arrival_time) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = cur;
    }
}
