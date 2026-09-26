#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
 * scheduler.h - struct dùng chung và interface hàm cho toàn nhóm.
 *
 * QUY ƯỚC CHUNG (mọi module thuật toán phải tuân thủ):
 *
 * 1. KHÔNG ĐẢO THỨ TỰ p[].
 *    Hàm schedule_xxx không được sort/hoán đổi các phần tử của p[]. Muốn duyệt
 *    theo thứ tự khác (arrival, burst, priority...) thì dùng mảng chỉ số phụ
 *    hoặc con trỏ. Nhờ vậy thứ tự in bảng kết quả luôn giống thứ tự nhập, và
 *    p[i] luôn là process thứ i người dùng đã nhập. Trường pid chỉ là nhãn,
 *    không đảm bảo pid == i, nên tra cứu theo pid phải duyệt tìm.
 *
 * 2. GỘP GANTT.
 *    Các khoảng chạy liên tiếp của cùng một process (kể cả idle) phải gộp
 *    thành một GanttEntry duy nhất, ví dụ P1 [0,2] + P1 [2,5] -> P1 [0,5].
 *    Không tự ghi g[(*g_len)++] mà luôn gọi gantt_append(), hàm này lo việc
 *    gộp. Thuật toán preemptive có thể cứ gọi gantt_append mỗi đơn vị thời
 *    gian mà vẫn ra Gantt đúng.
 *
 * 3. Scheduler chỉ có trách nhiệm điền Gantt (g, g_len). Các chỉ số
 *    completion/turnaround/waiting/response được calc_metrics() suy ra từ
 *    Gantt, nên scheduler không cần tự tính (được phép dùng remaining_time
 *    làm biến làm việc).
 *
 * 4. Scheduler sửa p[] (remaining_time), nên muốn chạy nhiều thuật toán trên
 *    cùng một input thì phải chạy trên bản sao (compare_all làm việc này).
 *    Ngoài ra MỖI schedule_xxx phải gán remaining_time = burst_time cho mọi
 *    process ở đầu hàm và đặt *g_len = 0, để gọi lại lần hai trên cùng một
 *    mảng vẫn cho kết quả đúng (không được giả định remaining_time còn nguyên).
 *
 * 5. GANTT LIỀN MẠCH TỪ t = 0, DO SCHEDULER GHI CẢ IDLE.
 *    Gantt luôn bắt đầu ở start = 0 và các entry nối liền nhau
 *    (g[i].end == g[i+1].start), không có lỗ hổng thời gian. Khi CPU rảnh
 *    (kể cả trước process đầu tiên đến, ví dụ đến ở t = 3 thì có khối
 *    PID_IDLE [0,3]), chính scheduler gọi gantt_append(..., PID_IDLE, ...).
 *    calc_metrics và print_gantt không tự chèn idle.
 *    cpu_utilization = tổng độ dài các entry khác PID_IDLE / end của entry
 *    cuối * 100 (khối idle đầu Gantt tính vào mẫu số).
 *
 * 6. GIỚI HẠN THỜI GIAN. Input đã qua validate_processes() đảm bảo
 *    max(arrival) + sum(burst) <= MAX_TOTAL_TIME. Vì mỗi entry (kể cả idle)
 *    dài ít nhất 1 đơn vị và các entry nối liền từ 0, Gantt có tối đa
 *    MAX_TOTAL_TIME entry, nên gantt_append không bao giờ tràn với input hợp
 *    lệ. Vẫn nên kiểm tra giá trị trả về và dừng an toàn nếu là -1.
 */

/* ---------- Hằng số ---------- */

#define MAX_PROCESSES 100
#define MAX_TOTAL_TIME 4096  /* giới hạn max(arrival) + sum(burst), xem quy ước 6 */
#define MAX_GANTT     MAX_TOTAL_TIME   /* đủ cho SRTF/RR chạy từng đơn vị thời gian */
#define PID_IDLE      (-1)   /* GanttEntry.pid == PID_IDLE nghĩa là CPU idle */

/* ---------- Kiểu dữ liệu ---------- */

typedef struct {
    int pid, arrival_time, burst_time, priority;
    int remaining_time, completion_time, turnaround_time,
        waiting_time, response_time;
} Process;

typedef struct { int pid, start, end; } GanttEntry;   /* pid = PID_IDLE: CPU idle */

/* Giá trị trung bình do calc_metrics trả về. */
typedef struct {
    double avg_waiting;
    double avg_turnaround;
    double avg_response;
    double cpu_utilization;   /* % busy / end cuối, xem quy ước 5 */
} Metrics;

/* ---------- Nhóm A: nhập liệu, validate, FCFS ---------- */

/* Nhập tay từ bàn phím. Trả về số process đọc được, hoặc -1 nếu lỗi. */
int input_from_keyboard(Process p[], int max_n);

/* Đọc từ file (mỗi dòng: pid arrival burst priority). Trả về n, hoặc -1. */
int input_from_file(const char *path, Process p[], int max_n);

/* Kiểm tra dữ liệu, trả về 1 nếu hợp lệ, 0 nếu không (và in lý do ra stderr):
 *   - 0 < n <= MAX_PROCESSES
 *   - arrival >= 0, burst > 0, priority >= 0
 *   - pid không trùng và không bằng PID_IDLE
 *   - max(arrival) + sum(burst) <= MAX_TOTAL_TIME (tính bằng long long hoặc
 *     kiểm tra từng bước để không tràn int) */
int validate_processes(const Process p[], int n);

void schedule_fcfs(Process p[], int n, GanttEntry g[], int *g_len);

/* ---------- Nhóm B, C, D: thuật toán ---------- */

/* B */
void schedule_sjf(Process p[], int n, GanttEntry g[], int *g_len);
void schedule_srtf(Process p[], int n, GanttEntry g[], int *g_len);

/* C: preemptive = 1 (ngắt khi có process priority cao hơn đến) hoặc 0.
 * Quy ước: priority NHỎ = ƯU TIÊN CAO; hòa priority thì FCFS theo arrival,
 * rồi theo vị trí trong p[]. */
void schedule_priority(Process p[], int n, GanttEntry g[], int *g_len,
                       int preemptive);

/* D */
void schedule_rr(Process p[], int n, GanttEntry g[], int *g_len, int quantum);
void schedule_mlfq(Process p[], int n, GanttEntry g[], int *g_len);

/* ---------- Nhóm E: Gantt, metrics, so sánh ---------- */

/* Thêm khoảng [start,end) của pid (hoặc PID_IDLE) vào Gantt, tự gộp với entry
 * cuối nếu cùng pid và end cũ == start mới. Thực thi quy ước 5. Trả về 0 nếu
 * thành công, -1 (và không ghi gì) nếu:
 *   - start >= end
 *   - tràn MAX_GANTT
 *   - start != end của entry cuối (có lỗ hổng hoặc chồng lấn thời gian)
 *   - Gantt đang rỗng mà start != 0
 * Cả scheduler lẫn E đều dùng hàm này. */
int gantt_append(GanttEntry g[], int *g_len, int pid, int start, int end);

/* Suy ra completion/turnaround/waiting/response cho từng p[i] từ Gantt
 * (completion = end cuối của pid; response = start đầu - arrival;
 * turnaround = completion - arrival; waiting = turnaround - burst), ghi vào
 * p[], và trả về số liệu trung bình qua *out. Dùng burst_time gốc, không phụ
 * thuộc remaining_time. */
void calc_metrics(Process p[], int n, const GanttEntry g[], int g_len,
                  Metrics *out);

/* In Gantt dạng text (thanh process và trục thời gian bên dưới). */
void print_gantt(const GanttEntry g[], int g_len);

/* Chạy MỌI thuật toán trên bản sao của src[] và in bảng so sánh.
 * quantum dùng cho RR; Priority được chạy cả hai chế độ. Vì các tham số phụ
 * (quantum, preemptive) do compare_all tự truyền, không cần con trỏ hàm với
 * chữ ký chung. */
void compare_all(const Process src[], int n, int quantum);

#endif /* SCHEDULER_H */
