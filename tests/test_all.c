/*
 * test_all.c - kiểm tra tự động mọi thuật toán. Chạy: make test
 *
 * Phần 1: ví dụ có đáp án tính tay (Gantt và WT trung bình).
 * Phần 2: dữ liệu ngẫu nhiên, kiểm tra các tính chất mọi thuật toán phải có.
 * Dùng được để kiểm tra code của bất kỳ ai, miễn đúng interface scheduler.h.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

enum { FCFS, SJF, SRTF, PRIO_NP, PRIO_P, RR, N_ALGO };
static const char *NAME[N_ALGO] = { "FCFS", "SJF", "SRTF", "Priority NP", "Priority P", "RR" };

static int failures = 0, checks = 0;

static void run(int algo, Process p[], int n, int q, GanttEntry g[], int *g_len)
{
    switch (algo) {
    case FCFS: schedule_fcfs(p, n, g, g_len); break;
    case SJF: schedule_sjf(p, n, g, g_len); break;
    case SRTF: schedule_srtf(p, n, g, g_len); break;
    case PRIO_NP: schedule_priority(p, n, g, g_len, 0); break;
    case PRIO_P: schedule_priority(p, n, g, g_len, 1); break;
    case RR: schedule_rr(p, n, g, g_len, q); break;
    }
}

static void gantt_str(const GanttEntry g[], int g_len, char *buf, size_t size)
{
    size_t used = 0;
    buf[0] = '\0';
    for (int k = 0; k < g_len && used < size; k++) {
        if (g[k].pid == PID_IDLE)
            used += snprintf(buf + used, size - used, "%sidle %d-%d", k ? ", " : "", g[k].start, g[k].end);
        else
            used += snprintf(buf + used, size - used, "%sP%d %d-%d", k ? ", " : "", g[k].pid, g[k].start, g[k].end);
    }
}

/* Mỗi dòng của data: pid arrival burst priority. */
static int make(Process p[], const int data[][4], int n)
{
    for (int i = 0; i < n; i++) {
        memset(&p[i], 0, sizeof p[i]);
        p[i].pid = data[i][0];
        p[i].arrival_time = data[i][1];
        p[i].burst_time = data[i][2];
        p[i].priority = data[i][3];
    }
    return n;
}

static void expect(const char *name, int algo, const int data[][4], int n, int q,
                   const char *want_gantt, double want_wt)
{
    Process p[MAX_PROCESSES];
    GanttEntry g[MAX_GANTT];
    int g_len = 0;
    Metrics m;
    char got[1024];

    make(p, data, n);
    run(algo, p, n, q, g, &g_len);
    run(algo, p, n, q, g, &g_len);   /* chạy lần hai trên cùng mảng: phải ra y như cũ */
    calc_metrics(p, n, g, g_len, &m);
    gantt_str(g, g_len, got, sizeof got);
    checks++;
    if (strcmp(got, want_gantt) != 0 || m.avg_waiting < want_wt - 0.005 || m.avg_waiting > want_wt + 0.005) {
        failures++;
        printf("SAI  %-34s %s\n     muon: %s (WT %.2f)\n     ra  : %s (WT %.2f)\n",
               name, NAME[algo], want_gantt, want_wt, got, m.avg_waiting);
    } else {
        printf("dung %-34s %s\n", name, NAME[algo]);
    }
}

/* So đủ bốn số trung bình, để bắt lỗi trong calc_metrics (RT, TAT, CPU%). */
static void expect_metrics(const char *name, int algo, const int data[][4], int n, int q,
                           double wt, double tat, double rt, double cpu)
{
    Process p[MAX_PROCESSES];
    GanttEntry g[MAX_GANTT];
    int g_len = 0;
    Metrics m;
    double want[4] = { wt, tat, rt, cpu }, got[4];
    int bad = 0;

    make(p, data, n);
    run(algo, p, n, q, g, &g_len);
    calc_metrics(p, n, g, g_len, &m);
    got[0] = m.avg_waiting; got[1] = m.avg_turnaround; got[2] = m.avg_response; got[3] = m.cpu_utilization;
    for (int k = 0; k < 4; k++)
        if (got[k] < want[k] - 0.005 || got[k] > want[k] + 0.005)
            bad = 1;
    checks++;
    if (bad) {
        failures++;
        printf("SAI  %-34s %s\n     muon WT %.2f TAT %.2f RT %.2f CPU %.2f\n     ra   WT %.2f TAT %.2f RT %.2f CPU %.2f\n",
               name, NAME[algo], want[0], want[1], want[2], want[3], got[0], got[1], got[2], got[3]);
    } else {
        printf("dung %-34s %s\n", name, NAME[algo]);
    }
}

static void known_examples(void)
{
    /* Silberschatz: SJF trung bình WT = 4, SRTF = 3 */
    static const int s1[][4] = { {1, 0, 7, 0}, {2, 2, 4, 0}, {3, 4, 1, 0}, {4, 5, 4, 0} };
    expect("sach: SJF co arrival", SJF, s1, 4, 0, "P1 0-7, P3 7-8, P2 8-12, P4 12-16", 4.0);
    expect("sach: SRTF co arrival", SRTF, s1, 4, 0, "P1 0-2, P2 2-4, P3 4-5, P2 5-7, P4 7-11, P1 11-16", 3.0);

    /* Silberschatz: Priority không ngắt, cùng đến lúc 0, WT = 8.2 */
    static const int s2[][4] = { {1, 0, 10, 3}, {2, 0, 1, 1}, {3, 0, 2, 4}, {4, 0, 1, 5}, {5, 0, 5, 2} };
    expect("sach: Priority cung luc 0", PRIO_NP, s2, 5, 0, "P2 0-1, P5 1-6, P1 6-16, P3 16-18, P4 18-19", 8.2);

    /* Priority có ngắt: P2 ưu tiên cao đến lúc 1 giành CPU của P1 */
    static const int s3[][4] = { {1, 0, 5, 3}, {2, 1, 3, 1}, {3, 2, 1, 2} };
    expect("tinh tay: Priority ngat", PRIO_P, s3, 3, 0, "P1 0-1, P2 1-4, P3 4-5, P1 5-9", 2.0);
    expect("tinh tay: Priority khong ngat", PRIO_NP, s3, 3, 0, "P1 0-5, P2 5-8, P3 8-9", (0 + 4 + 6) / 3.0);

    /* Silberschatz: RR q = 4, WT = 17/3 */
    static const int s4[][4] = { {1, 0, 24, 0}, {2, 0, 3, 0}, {3, 0, 3, 0} };
    expect("sach: RR q=4", RR, s4, 3, 4, "P1 0-4, P2 4-7, P3 7-10, P1 10-30", 17 / 3.0);
    expect("convoy: FCFS", FCFS, s4, 3, 0, "P1 0-24, P2 24-27, P3 27-30", 17.0);
    expect("convoy: SJF", SJF, s4, 3, 0, "P2 0-3, P3 3-6, P1 6-30", 3.0);
    expect("RR quantum lon hon moi burst = FCFS", RR, s4, 3, 100, "P1 0-24, P2 24-27, P3 27-30", 17.0);

    /* RR: process đến đúng lúc hết quantum vào hàng trước process vừa bị ngắt */
    static const int s5[][4] = { {1, 0, 5, 0}, {2, 1, 3, 0}, {3, 2, 1, 0} };
    expect("RR q=2, den dung luc het quantum", RR, s5, 3, 2, "P1 0-2, P2 2-4, P3 4-5, P1 5-7, P2 7-8, P1 8-9", 10 / 3.0);

    /* CPU rảnh ở đầu và ở giữa, mọi thuật toán phải tự ghi idle */
    static const int s6[][4] = { {1, 3, 2, 1}, {2, 8, 1, 0} };
    for (int a = 0; a < N_ALGO; a++)
        expect("idle dau va giua", a, s6, 2, 2, "idle 0-3, P1 3-5, idle 5-8, P2 8-9", 0.0);

    /* Hòa: cùng burst, cùng priority, cùng arrival thì theo thứ tự nhập */
    static const int s7[][4] = { {5, 0, 2, 1}, {3, 0, 2, 1}, {9, 0, 2, 1} };
    expect("hoa het: theo thu tu nhap", SJF, s7, 3, 0, "P5 0-2, P3 2-4, P9 4-6", 2.0);
    expect("hoa het: theo thu tu nhap", SRTF, s7, 3, 0, "P5 0-2, P3 2-4, P9 4-6", 2.0);
    expect("hoa het: theo thu tu nhap", PRIO_P, s7, 3, 0, "P5 0-2, P3 2-4, P9 4-6", 2.0);

    /* SJF hòa burst: process đến sớm hơn chạy trước dù nhập sau */
    static const int s9[][4] = { {1, 0, 5, 0}, {2, 3, 3, 0}, {3, 1, 3, 0} };
    expect("SJF hoa burst xet arrival", SJF, s9, 3, 0, "P1 0-5, P3 5-8, P2 8-11", 3.0);

    /* Priority hòa: process đến sớm hơn chạy trước dù nhập sau (quy ước trong scheduler.h) */
    static const int s10[][4] = { {1, 0, 5, 1}, {2, 3, 3, 2}, {3, 1, 3, 2} };
    expect("Priority hoa xet arrival", PRIO_NP, s10, 3, 0, "P1 0-5, P3 5-8, P2 8-11", 3.0);
    expect("Priority hoa xet arrival", PRIO_P, s10, 3, 0, "P1 0-5, P3 5-8, P2 8-11", 3.0);

    /* Đủ bốn chỉ số: RT khác WT với thuật toán có ngắt, CPU% có đoạn idle */
    expect_metrics("chi so: SRTF sach", SRTF, s1, 4, 0, 3.0, 7.0, 0.5, 100.0);
    expect_metrics("chi so: RR q=4 sach", RR, s4, 3, 4, 17 / 3.0, 47 / 3.0, 11 / 3.0, 100.0);
    expect_metrics("chi so: idle dau va giua", FCFS, s6, 2, 0, 0.0, 1.5, 0.0, 100.0 * 3 / 9);

    /* SRTF bằng thời gian còn lại thì không ngắt process đang chạy */
    static const int s8[][4] = { {1, 0, 4, 0}, {2, 2, 2, 0} };
    expect("SRTF bang nhau khong ngat", SRTF, s8, 2, 0, "P1 0-4, P2 4-6", 1.0);
}

/* ---------- Phần 2: tính chất trên dữ liệu ngẫu nhiên ---------- */

static void fail(const char *what, int algo, int seed)
{
    failures++;
    if (failures < 20)
        printf("SAI  ngau nhien seed %d, %s: %s\n", seed, NAME[algo], what);
}

static void check_props(int algo, const Process src[], int n, int q, int seed, double *wt)
{
    Process p[MAX_PROCESSES];
    GanttEntry g[MAX_GANTT];
    int g_len = 0, ran[MAX_PROCESSES] = { 0 }, segs[MAX_PROCESSES] = { 0 };
    Metrics m;

    memcpy(p, src, (size_t)n * sizeof p[0]);
    run(algo, p, n, q, g, &g_len);
    checks++;
    if (g_len == 0 || g[0].start != 0) { fail("Gantt khong bat dau tu 0", algo, seed); return; }
    for (int k = 0; k < g_len; k++) {
        if (g[k].start >= g[k].end) fail("o rong hoac nguoc", algo, seed);
        if (k > 0 && g[k].start != g[k - 1].end) fail("Gantt co lo hong", algo, seed);
        if (k > 0 && g[k].pid == g[k - 1].pid) fail("hai o lien nhau cung pid chua gop", algo, seed);
        if (g[k].pid == PID_IDLE) {
            /* rảnh chỉ khi không có process nào đã đến mà chưa xong */
            for (int i = 0; i < n; i++)
                if (src[i].arrival_time <= g[k].start && ran[i] < src[i].burst_time)
                    fail("CPU ranh trong khi co process dang cho", algo, seed);
            continue;
        }
        for (int i = 0; i < n; i++) {
            if (src[i].pid != g[k].pid) continue;
            if (g[k].start < src[i].arrival_time) fail("chay truoc khi den", algo, seed);
            ran[i] += g[k].end - g[k].start;
            segs[i]++;
        }
    }
    for (int i = 0; i < n; i++) {
        if (ran[i] != src[i].burst_time) fail("tong thoi gian chay khac burst", algo, seed);
        if ((algo == FCFS || algo == SJF || algo == PRIO_NP) && segs[i] != 1)
            fail("thuat toan khong ngat ma process bi chia nhieu o", algo, seed);
        if (p[i].pid != src[i].pid || p[i].arrival_time != src[i].arrival_time ||
            p[i].burst_time != src[i].burst_time || p[i].priority != src[i].priority)
            fail("thuat toan sua hoac dao p[]", algo, seed);
    }
    calc_metrics(p, n, g, g_len, &m);
    *wt = m.avg_waiting;
}

static void random_tests(int rounds)
{
    srand(12345);
    for (int seed = 0; seed < rounds; seed++) {
        Process src[MAX_PROCESSES];
        int n = 1 + rand() % 8, q = 1 + rand() % 4;
        double wt[N_ALGO];

        for (int i = 0; i < n; i++) {
            memset(&src[i], 0, sizeof src[i]);
            src[i].pid = 10 + i * 3;          /* pid khác chỉ số mảng */
            src[i].arrival_time = rand() % 12;
            src[i].burst_time = 1 + rand() % 8;
            src[i].priority = rand() % 4;
        }
        for (int a = 0; a < N_ALGO; a++)
            check_props(a, src, n, q, seed, &wt[a]);
        /* SRTF tối ưu WT trung bình: không thuật toán nào được thấp hơn */
        for (int a = 0; a < N_ALGO; a++)
            if (wt[a] < wt[SRTF] - 1e-9)
                fail("co thuat toan cho WT trung binh thap hon SRTF", a, seed);
    }
    printf("da chay %d bo du lieu ngau nhien x %d thuat toan\n", rounds, N_ALGO);
}

int main(void)
{
    printf("== Vi du co dap an ==\n");
    known_examples();
    printf("\n== Tinh chat tren du lieu ngau nhien ==\n");
    random_tests(3000);
    printf("\nTong: %d lan kiem tra, %d loi\n", checks, failures);
    return failures ? 1 : 0;
}
