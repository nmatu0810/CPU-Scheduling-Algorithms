#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

#define LINE_MAX_LEN 256

/*
 * Phân vai validate:
 *  - input_from_file chỉ phân tích cú pháp; main gọi validate_processes cho
 *    mọi nguồn dữ liệu (cổng duy nhất, đảm bảo quy ước 6).
 *  - input_from_keyboard gọi validate_processes trên phần đã nhập + process vừa
 *    gõ để báo lỗi và cho nhập lại ngay, không viết lại logic kiểm tra.
 */

/* Đọc đúng một số nguyên từ *s (bỏ khoảng trắng đầu), tiến *s qua số đó.
 * Trả về 1 nếu thành công, 0 nếu không có số hợp lệ hoặc tràn int. */
static int parse_int(const char **s, int *out)
{
    char *end;
    long v;

    errno = 0;
    v = strtol(*s, &end, 10);
    if (end == *s || errno == ERANGE || v < INT_MIN || v > INT_MAX)
        return 0;
    *out = (int)v;
    *s = end;
    return 1;
}

/* Cả dòng phải là đúng cnt số nguyên, không thừa ký tự. */
static int parse_ints(const char *line, int vals[], int cnt)
{
    const char *s = line;

    for (int i = 0; i < cnt; i++)
        if (!parse_int(&s, &vals[i]))
            return 0;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
        s++;
    return *s == '\0';
}

static void make_process(Process *pr, const int v[4])
{
    memset(pr, 0, sizeof *pr);
    pr->pid = v[0];
    pr->arrival_time = v[1];
    pr->burst_time = v[2];
    pr->priority = v[3];
    pr->remaining_time = v[2];
}

int validate_processes(const Process p[], int n)
{
    long long max_arrival = 0, sum_burst = 0;

    if (n <= 0) {
        fprintf(stderr, "Loi: khong co process nao.\n");
        return 0;
    }
    if (n > MAX_PROCESSES) {
        fprintf(stderr, "Loi: qua %d process.\n", MAX_PROCESSES);
        return 0;
    }
    for (int i = 0; i < n; i++) {
        if (p[i].pid == PID_IDLE) {
            fprintf(stderr, "Loi: process thu %d co pid = %d (danh rieng cho idle).\n",
                    i + 1, PID_IDLE);
            return 0;
        }
        if (p[i].arrival_time < 0) {
            fprintf(stderr, "Loi: P%d co arrival < 0.\n", p[i].pid);
            return 0;
        }
        if (p[i].burst_time <= 0) {
            fprintf(stderr, "Loi: P%d co burst <= 0.\n", p[i].pid);
            return 0;
        }
        if (p[i].priority < 0) {
            fprintf(stderr, "Loi: P%d co priority < 0.\n", p[i].pid);
            return 0;
        }
        for (int j = 0; j < i; j++) {
            if (p[j].pid == p[i].pid) {
                fprintf(stderr, "Loi: pid %d bi trung.\n", p[i].pid);
                return 0;
            }
        }
        if (p[i].arrival_time > max_arrival)
            max_arrival = p[i].arrival_time;
        sum_burst += p[i].burst_time;   /* long long: moi burst < 2^31, n <= 100 */
    }
    if (max_arrival + sum_burst > MAX_TOTAL_TIME) {
        fprintf(stderr, "Loi: max(arrival) + tong burst = %lld vuot gioi han %d.\n",
                max_arrival + sum_burst, MAX_TOTAL_TIME);
        return 0;
    }
    return 1;
}

/* Đọc một dòng từ stream. Trả về 1 nếu được, 0 nếu EOF/lỗi. Nếu dòng dài hơn
 * buffer thì bỏ phần còn lại để không lệch sang lần đọc sau. */
static int read_line(FILE *f, char *buf, size_t size)
{
    if (!fgets(buf, (int)size, f))
        return 0;
    if (!strchr(buf, '\n') && !feof(f)) {
        int c;
        while ((c = fgetc(f)) != '\n' && c != EOF)
            ;
    }
    return 1;
}

int input_from_keyboard(Process p[], int max_n)
{
    char line[LINE_MAX_LEN];
    int n, v[4];

    /* validate_processes từ chối n > MAX_PROCESSES, nên max_n lớn hơn thì
     * vòng nhập process thứ 101 trở đi sẽ lặp mãi. */
    if (max_n > MAX_PROCESSES)
        max_n = MAX_PROCESSES;

    for (;;) {
        printf("So luong process (1-%d): ", max_n);
        if (!read_line(stdin, line, sizeof line))
            return -1;
        if (parse_ints(line, &n, 1) && n >= 1 && n <= max_n)
            break;
        fprintf(stderr, "Nhap mot so nguyen tu 1 den %d.\n", max_n);
    }

    for (int i = 0; i < n; ) {
        printf("Process %d/%d (pid arrival burst priority): ", i + 1, n);
        if (!read_line(stdin, line, sizeof line))
            return -1;
        if (!parse_ints(line, v, 4)) {
            fprintf(stderr, "Nhap dung 4 so nguyen: pid arrival burst priority.\n");
            continue;
        }
        make_process(&p[i], v);
        /* Validate phần đã nhập + process vừa gõ: sai thì nhập lại process này. */
        if (validate_processes(p, i + 1))
            i++;
    }
    return n;
}

int input_from_file(const char *path, Process p[], int max_n)
{
    FILE *f = fopen(path, "r");
    char line[LINE_MAX_LEN];
    int n = 0, lineno = 0, v[4];

    if (!f) {
        fprintf(stderr, "Khong mo duoc file '%s'.\n", path);
        return -1;
    }
    while (read_line(f, line, sizeof line)) {
        const char *s = line;

        lineno++;
        while (*s == ' ' || *s == '\t')
            s++;
        if (*s == '\0' || *s == '\n' || *s == '\r' || *s == '#')
            continue;                    /* dòng trống hoặc comment */
        if (!parse_ints(line, v, 4)) {
            fprintf(stderr, "%s:%d: can dung 4 so nguyen: pid arrival burst priority.\n",
                    path, lineno);
            fclose(f);
            return -1;
        }
        if (n >= max_n) {
            fprintf(stderr, "%s:%d: qua %d process.\n", path, lineno, max_n);
            fclose(f);
            return -1;
        }
        make_process(&p[n++], v);
    }
    fclose(f);
    return n;
}
