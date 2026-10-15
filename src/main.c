#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

#define LINE_LEN 256

enum {
    MENU_EXIT, MENU_KEYBOARD, MENU_FILE, MENU_SHOW,
    MENU_FCFS, MENU_SJF, MENU_SRTF, MENU_PRIORITY, MENU_RR,
    MENU_COMPARE
};

/* Đọc một số nguyên trong [lo, hi], nhập sai thì hỏi lại. Trả về -1 nếu hết
 * input (EOF), nên lo phải >= 0. */
static int ask_int(const char *prompt, int lo, int hi)
{
    char line[LINE_LEN];

    for (;;) {
        char *end;
        long v;

        printf("%s", prompt);
        if (!fgets(line, sizeof line, stdin))
            return -1;
        if (!strchr(line, '\n') && !feof(stdin)) {   /* dòng quá dài: bỏ phần dư */
            int c;
            while ((c = fgetc(stdin)) != '\n' && c != EOF)
                ;
        }
        errno = 0;
        v = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
            end++;
        if (end != line && *end == '\0' && errno == 0 && v >= lo && v <= hi)
            return (int)v;
        printf("Nhap so nguyen tu %d den %d.\n", lo, hi);
    }
}

static void print_menu(int n)
{
    printf("\n===== CPU SCHEDULING (%d process) =====\n", n);
    printf(" 1. Nhap process bang tay\n");
    printf(" 2. Doc process tu file\n");
    printf(" 3. Xem du lieu hien tai\n");
    printf(" 4. FCFS\n");
    printf(" 5. SJF (khong ngat)\n");
    printf(" 6. SRTF\n");
    printf(" 7. Priority\n");
    printf(" 8. Round Robin\n");
    printf(" 9. So sanh tat ca\n");
    printf(" 0. Thoat\n");
}

/* Chỉ in phần input, chưa chạy thuật toán nên chưa có CT/TAT/WT/RT. */
static void print_input(const Process p[], int n)
{
    printf("\nPID  AT  BT  PRIO\n");
    for (int i = 0; i < n; i++)
        printf("P%-3d %2d  %2d  %4d\n", p[i].pid, p[i].arrival_time,
               p[i].burst_time, p[i].priority);
}

static void print_table(const Process p[], int n, const Metrics *m)
{
    printf("\nPID  AT  BT  CT  TAT  WT  RT\n");
    for (int i = 0; i < n; i++)
        printf("P%-3d %2d  %2d  %2d  %3d  %2d  %2d\n", p[i].pid,
               p[i].arrival_time, p[i].burst_time, p[i].completion_time,
               p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
    printf("\nAvg WT=%.2f  TAT=%.2f  RT=%.2f  CPU=%.2f%%\n", m->avg_waiting,
           m->avg_turnaround, m->avg_response, m->cpu_utilization);
}

/* Nạp dữ liệu từ bàn phím hoặc file. Dữ liệu mới phải qua validate_processes
 * (cổng chung cho mọi nguồn), nếu không thì giữ nguyên dữ liệu cũ. */
static void load_data(Process dst[], int *n, int from_file)
{
    Process tmp[MAX_PROCESSES];
    char path[LINE_LEN];
    int k;

    if (from_file) {
        printf("Duong dan file: ");
        if (!fgets(path, sizeof path, stdin))
            return;
        path[strcspn(path, "\r\n")] = '\0';
        k = input_from_file(path, tmp, MAX_PROCESSES);
    } else {
        k = input_from_keyboard(tmp, MAX_PROCESSES);
    }

    if (k < 0 || !validate_processes(tmp, k)) {
        printf("Khong nap duoc du lieu, giu nguyen du lieu cu.\n");
        return;
    }
    memcpy(dst, tmp, (size_t)k * sizeof tmp[0]);
    *n = k;
    printf("Da nap %d process.\n", k);
}

/* Chạy một thuật toán trên bản sao của dữ liệu rồi in Gantt và bảng kết quả.
 * Thuật toán chưa có code (stub) không ghi Gantt nên g_len = 0. */
static void run_algorithm(int choice, const Process src[], int n)
{
    Process p[MAX_PROCESSES];
    GanttEntry g[MAX_GANTT];
    int g_len = 0;
    char title[48];
    Metrics m;

    memcpy(p, src, (size_t)n * sizeof p[0]);

    switch (choice) {
    case MENU_FCFS:
        snprintf(title, sizeof title, "FCFS");
        schedule_fcfs(p, n, g, &g_len);
        break;
    case MENU_SJF:
        snprintf(title, sizeof title, "SJF");
        schedule_sjf(p, n, g, &g_len);
        break;
    case MENU_SRTF:
        snprintf(title, sizeof title, "SRTF");
        schedule_srtf(p, n, g, &g_len);
        break;
    case MENU_PRIORITY: {
        int pre = ask_int("Che do (0 = khong ngat, 1 = ngat): ", 0, 1);
        if (pre < 0)
            return;
        snprintf(title, sizeof title, "Priority (%s)",
                 pre ? "ngat" : "khong ngat");
        schedule_priority(p, n, g, &g_len, pre);
        break;
    }
    case MENU_RR: {
        int q = ask_int("Time quantum: ", 1, MAX_TOTAL_TIME);
        if (q < 0)
            return;
        snprintf(title, sizeof title, "Round Robin (q = %d)", q);
        schedule_rr(p, n, g, &g_len, q);
        break;
    }
    default:
        return;
    }

    if (g_len == 0) {
        printf("%s: chua cai dat (hoac khong co ket qua).\n", title);
        return;
    }
    calc_metrics(p, n, g, g_len, &m);
    printf("\n=== %s ===\n", title);
    print_gantt(g, g_len);
    print_table(p, n, &m);
}

int main(void)
{
    Process procs[MAX_PROCESSES];
    int n = 0;

    for (;;) {
        int c;

        print_menu(n);
        c = ask_int("Chon: ", 0, MENU_COMPARE);
        if (c <= MENU_EXIT)          /* 0 hoặc hết input */
            break;

        if (c == MENU_KEYBOARD) {
            load_data(procs, &n, 0);
        } else if (c == MENU_FILE) {
            load_data(procs, &n, 1);
        } else if (n == 0) {
            printf("Chua co du lieu, chon 1 hoac 2 truoc.\n");
        } else if (c == MENU_SHOW) {
            print_input(procs, n);
        } else if (c == MENU_COMPARE) {
            int q = ask_int("Time quantum cho Round Robin: ", 1, MAX_TOTAL_TIME);
            if (q < 0)
                break;
            compare_all(procs, n, q);
        } else {
            run_algorithm(c, procs, n);
        }
    }
    return 0;
}
