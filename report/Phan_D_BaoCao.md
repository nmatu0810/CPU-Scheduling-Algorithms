# ĐỒ ÁN LẬP TRÌNH HỆ THỐNG - ĐỀ TÀI 15: CPU SCHEDULING
## BẢN THẢO NỘI DUNG BÁO CÁO CỦA THÀNH VIÊN D
*(Phần đóng góp của Thành viên D gửi Nhóm trưởng A tích hợp vào Báo cáo chung `BaoCao_CPU_Scheduling.docx`)*

---

## MỤC 3.6. GANTT CHART VÀ TÍNH CHỈ SỐ

### 3.6.1. Thiết kế và hiển thị sơ đồ Gantt dạng văn bản (ASCII Gantt Chart)
Sơ đồ Gantt là công cụ trực quan hóa cốt lõi giúp người dùng theo dõi chính xác tiến trình nào đang chiếm giữ CPU tại từng khoảng thời gian. Trong chương trình, sơ đồ Gantt được cài đặt trong module `src/gantt.c` với hai hàm chính:
1. `gantt_append`: Ghi nhận và tự động gộp các khoảng thực thi vào mảng `GanttEntry g[]`.
2. `print_gantt`: Trực quan hóa toàn bộ mảng `g[]` dưới dạng biểu đồ hộp ASCII (box-drawing) kèm trục mốc thời gian căn chỉnh chuẩn xác bên dưới.

**Cấu trúc dữ liệu `GanttEntry`:**
```c
typedef struct {
    int pid;    /* Mã định danh tiến trình, hoặc PID_IDLE (-1) nếu CPU rảnh */
    int start;  /* Thời điểm bắt đầu của khoảng thời gian */
    int end;    /* Thời điểm kết thúc của khoảng thời gian (end > start) */
} GanttEntry;
```

**Nguyên lý hiển thị của `print_gantt`:**
- **Độ rộng khối linh hoạt (`calc_block_width`):** Mỗi khối thực thi có độ rộng $w$ được tính toán cân bằng giữa độ dài thời gian chạy ($dur = end - start$) và kích thước chuỗi nhãn (`P<pid>` hoặc `IDLE`). Công thức đảm bảo độ rộng tối thiểu là 6 ký tự, tối đa là 16 ký tự, nhãn luôn được căn giữa (center-aligned).
- **Trục thời gian không lệch cột:** Trục số thời gian bên dưới sử dụng một bộ đệm chuỗi (`time_line`) được khởi tạo toàn bộ bằng dấu cách. Mốc thời gian bắt đầu và các mốc kết thúc được ghi chính xác tại vị trí chỉ số cột tương ứng với các dấu gạch nối `+` của khung hộp, triệt tiêu hoàn toàn hiện tượng lệch cột khi in số có 1, 2 hoặc 3 chữ số.
- **Tự động ngắt dòng thông minh (`MAX_LINE_COLS = 72`):** Đối với các thuật toán có tần suất chuyển đổi ngữ cảnh cao (như Round Robin với time quantum nhỏ hoặc SRTF), biểu đồ có thể kéo dài hàng chục khối. Hàm tự động tính toán tổng độ rộng tích lũy của các khối và ngắt thành nhiều dòng riêng biệt khi vượt quá 72 ký tự, kèm theo khung viền và trục thời gian cho từng dòng. Nhờ đó, giao diện không bao giờ bị vỡ trên các cửa sổ dòng lệnh chuẩn (80 cột).

Ví dụ biểu đồ hiển thị trên màn hình:
```text
--- So do Gantt ---
+------+------+------+------+------+
| IDLE |  P1  |  P2  | IDLE |  P3  |
+------+------+------+------+------+
0      2      5      7      8      9
```

---

### 3.6.2. Cơ chế gộp khối thời gian liên tục (`gantt_append`)
Theo Quy ước 2 và Quy ước 5 trong `include/scheduler.h`:
- Sơ đồ Gantt phải liền mạch từ $t = 0$ đến khi tiến trình cuối cùng kết thúc, không có khoảng hở thời gian (`g[i].end == g[i+1].start`).
- Các thuật toán điều phối dạng ưu tiên ngắt (preemptive) như SRTF hay Priority ngắt có thể đánh giá và gọi hàm ghi nhận mỗi đơn vị thời gian ($1$ time unit). Khi đó, nếu một tiến trình tiếp tục được chọn ở đơn vị thời gian kế tiếp, việc tạo ra hàng loạt entry vụn vặt cùng PID sẽ làm tràn bộ nhớ và gây rối mắt.

Hàm `gantt_append` giải quyết triệt để vấn đề này bằng giải thuật gộp khối tại chỗ (in-place merging):
```c
int gantt_append(GanttEntry g[], int *g_len, int pid, int start, int end)
{
    if (start >= end)
        return -1;

    if (*g_len == 0) {
        if (start != 0)
            return -1; /* Lỗi: Gantt bắt buộc phải xuất phát từ t = 0 */
    } else {
        GanttEntry *last = &g[*g_len - 1];
        if (start != last->end)
            return -1; /* Lỗi: Có lỗ hổng hoặc chồng lấn thời gian */

        /* Nếu cùng PID và nối tiếp liên tục -> gộp khối */
        if (last->pid == pid) {
            last->end = end;
            return 0;
        }
    }

    if (*g_len >= MAX_GANTT)
        return -1; /* Tránh tràn mảng tĩnh */

    g[*g_len].pid = pid;
    g[*g_len].start = start;
    g[*g_len].end = end;
    (*g_len)++;
    return 0;
}
```

---

### 3.6.3. Giải thuật suy dẫn chỉ số hiệu năng từ Gantt (`calc_metrics`)
Theo kiến trúc phân tầng của hệ thống, các thuật toán lập lịch (`schedule_xxx`) chỉ tập trung vào việc mô phỏng và xây dựng chuỗi Gantt. Toàn bộ các chỉ số thống kê về thời gian hoàn thành (Completion Time - CT), thời gian lưu lại (Turnaround Time - TAT), thời gian chờ (Waiting Time - WT) và thời gian phản hồi (Response Time - RT) đều được module `src/metrics.c` suy ra trực tiếp từ sơ đồ Gantt.

Giải thuật suy dẫn trong hàm `calc_metrics` hoạt động như sau:
1. Đối với mỗi tiến trình $p_i$ ($i = 0 \dots n-1$):
   - Quét qua toàn bộ mảng `g[0 .. g_len - 1]`:
     - Ghi nhận `first_start`: thời điểm bắt đầu của khối đầu tiên mà tiến trình $p_i$ được cấp phát CPU ($g[k].pid == p_i.pid$).
     - Ghi nhận `last_end`: thời điểm kết thúc của khối cuối cùng mà $p_i$ chạy.
   - **Thời gian hoàn thành ($CT_i$):** Là thời điểm khối cuối cùng của tiến trình kết thúc:
     $$CT_i = last\_end$$
   - **Thời gian lưu lại ($TAT_i$):** Khoảng thời gian từ lúc tiến trình xuất hiện trong hệ thống đến khi kết thúc trọn vẹn:
     $$TAT_i = CT_i - ArrivalTime_i$$
   - **Thời gian chờ ($WT_i$):** Tổng thời gian tiến trình phải nằm trong hàng đợi sẵn sàng (Ready Queue) mà không được CPU phục vụ. Do mô hình không có I/O xen giữa:
     $$WT_i = TAT_i - BurstTime_i$$
   - **Thời gian phản hồi ($RT_i$):** Khoảng thời gian từ khi tiến trình đến hệ thống cho tới lần đầu tiên được CPU xử lý:
     $$RT_i = first\_start - ArrivalTime_i$$
2. **Các giá trị trung bình toàn hệ thống:**
   $$\text{Avg WT} = \frac{1}{n} \sum_{i=1}^n WT_i, \quad \text{Avg TAT} = \frac{1}{n} \sum_{i=1}^n TAT_i, \quad \text{Avg RT} = \frac{1}{n} \sum_{i=1}^n RT_i$$

Phương pháp suy dẫn này độc lập hoàn toàn với trường `remaining_time`, giúp kết quả luôn chính xác tuyệt đối ngay cả khi thuật toán bị ngắt quãng nhiều lần.

---

### 3.6.4. Cách tính hiệu suất CPU (CPU Utilization)
Hiệu suất sử dụng CPU đo lường tỷ lệ phần trăm thời gian CPU thực sự làm việc hữu ích (thực thi các tiến trình người dùng) so với tổng thời gian hệ thống vận hành từ $t = 0$ đến khi hoàn tất tiến trình cuối cùng.

Công thức tính toán trong hàm `calc_metrics`:
$$\text{CPU Utilization (\%)} = \frac{\sum_{k, g[k].pid \neq PID\_IDLE} (g[k].end - g[k].start)}{g[g\_len - 1].end} \times 100\%$$

Trong đó:
- Tử số là tổng thời lượng các khối thực thi có $pid \neq PID\_IDLE$.
- Mẫu số là mốc thời gian kết thúc của khối Gantt cuối cùng ($g[g\_len - 1].end$). Khối CPU rảnh ở đầu ($t = 0$ đến khi tiến trình đầu tiên đến) và các khoảng rảnh xen giữa đều được tính vào mẫu số, phản ánh đúng chi phí nhàn rỗi của hệ thống.

---

### 3.6.5. Thiết kế module so sánh tổng hợp (`compare_all`)
Module `src/compare.c` cung cấp chức năng đối sánh hiệu năng toàn diện giữa tất cả 5 thuật toán (bao gồm cả hai chế độ của Priority và giá trị quantum tùy chọn của Round Robin) trên cùng một tập dữ liệu đầu vào duy nhất.

**Nguyên lý vận hành:**
1. **Bảo toàn dữ liệu gốc (Quy ước 4):** Hàm tạo một bản sao độc lập của mảng tiến trình nguồn `src[]` sang mảng cục bộ `p[]` trước mỗi lần chạy thuật toán:
   ```c
   memcpy(p, src, (size_t)n * sizeof(p[0]));
   ```
2. **Khả năng tương thích và chống lỗi liên kết:** Hàm kiểm tra trạng thái mảng Gantt sau khi chạy từng thuật toán. Nếu thuật toán chưa được cài đặt (trả về `g_len == 0`), bảng so sánh hiển thị trạng thái `Chua cai dat` thay vì gây lỗi hoặc crash chương trình.
3. **Tổng hợp và định dạng bảng kết quả:** In bảng đối chiếu dạng cột căn chỉnh rõ ràng (Avg WT, Avg TAT, Avg RT, CPU Util).
4. **Phân tích khuyến nghị tự động:** Sau bảng dữ liệu, hàm tự động duyệt tìm và xướng tên các thuật toán đạt thành tích xuất sắc nhất ở từng hạng mục:
   - Thuật toán tối ưu thời gian chờ trung bình ($\min \text{Avg WT}$).
   - Thuật toán tối ưu thời gian lưu lại trung bình ($\min \text{Avg TAT}$).
   - Thuật toán có phản hồi nhanh nhất ($\min \text{Avg RT}$).

---

## CHƯƠNG 4. KIỂM THỬ, KẾT QUẢ VÀ SO SÁNH

### 4.1. Phương pháp kiểm thử
Để đảm bảo chất lượng, tính đúng đắn và độ tin cậy của phần mềm, nhóm đã áp dụng phương pháp kiểm thử toàn diện đa cấp độ kết hợp giữa lý thuyết và thực nghiệm:
1. **Kiểm thử đối chiếu với tính toán tay (Manual Verification):**
   Mỗi thuật toán đều được nhóm giải tay độc lập trên giấy (vẽ sơ đồ Gantt thủ công, lập bảng tính CT, TAT, WT, RT). Kết quả xuất ra từ chương trình C được so khớp từng con số với kết quả tính tay. Mọi sự sai lệch đều được truy vết và gỡ lỗi ngay trong quá trình phát triển.
2. **Kiểm thử dữ liệu bất thường và giá trị biên (Boundary & Stress Testing):**
   Xây dựng các bộ dữ liệu đặc biệt nhằm thử thách bộ lọc dữ liệu `validate_processes` và cấu trúc mảng: giá trị âm, burst time bằng 0, trùng PID, thời gian vượt ngưỡng $MAX\_TOTAL\_TIME = 4096$, số lượng tiến trình chạm ngưỡng $MAX\_PROCESSES = 100$, file rỗng, file chứa comment phức tạp.
3. **Kiểm thử chéo nội bộ (Cross-Testing Loop):**
   Thực hiện quy trình kiểm thử chéo bắt buộc theo chu trình khép kín:
   $$\text{Thành viên A} \longrightarrow \text{Thành viên B} \longrightarrow \text{Thành viên C} \longrightarrow \text{Thành viên D} \longrightarrow \text{Thành viên A}$$
   Mỗi thành viên độc lập đóng vai trò "người dùng khó tính" hoặc "hacker kiểm thử", tạo các kịch bản biên để tìm lỗi trong module của người tiếp theo và ghi nhận công khai vào file `docs/loi_test_cheo.md`.

---

### 4.2. Các bộ test mẫu trong thư mục `tests/`
Nhóm đã xây dựng hệ thống test case phong phú, mỗi file đều có tiền tố đại diện cho thành viên phụ trách và truyền tải một "câu chuyện kỹ thuật" cụ thể:

| Tên file | Thành viên | Số tiến trình | Mục đích kiểm thử & Câu chuyện kỹ thuật |
|---|:---:|:---:|---|
| `a_convoy_dai_truoc.txt` | A | 3 | **Câu chuyện Convoy Effect (Phần 1):** Tiến trình burst lớn (BT=24) đến trước tại $t=0$, hai tiến trình ngắn (BT=3) đến sau, gây hiệu ứng đoàn tàu nghiêm trọng trong FCFS. |
| `a_convoy_ngan_truoc.txt` | A | 3 | **Câu chuyện Convoy Effect (Phần 2):** Cùng 3 tiến trình trên nhưng đổi thứ tự đến (tiến trình ngắn đến trước) để chứng minh sự phụ thuộc nặng nề của FCFS vào thứ tự đến. |
| `a_fcfs_idle.txt` | A | 3 | **Kiểm tra CPU Idle FCFS:** Tiến trình đến rời rạc ở $t=2, 3, 8$, kiểm tra khối rảnh ban đầu $[0, 2]$ và khoảng rảnh xen giữa $[7, 8]$. |
| `d_starvation_priority.txt` | D | 5 | **Câu chuyện Starvation (Bỏ đói tiến trình):** Tiến trình $P_1$ độ ưu tiên thấp (Priority=5) bị các tiến trình độ ưu tiên cao liên tục đến chiếm CPU trong Priority ngắt. |
| `d_rr_quantum_compare.txt` | D | 4 | **Khảo sát ảnh hưởng của Time Quantum:** Đánh giá sự biến thiên của WT, RT và số lần chuyển ngữ cảnh khi tăng dần quantum từ $q=2$ đến $q=10$ (suy biến thành FCFS). |
| `d_idle_interleaved.txt` | D | 4 | **CPU Idle phức tạp đa đoạn:** Kiểm thử tính liên tục của sơ đồ Gantt và công thức tính CPU utilization khi hệ thống có 4 khoảng rảnh riêng biệt. |
| `d_comprehensive_compare.txt` | D | 5 | **So sánh tổng hợp đa mục tiêu:** Bộ dữ liệu chuẩn mực với arrival, burst và priority đa dạng để chạy chức năng `compare_all`. |

---

### 4.3. Kiểm thử dữ liệu sai và xử lý ngoại lệ
Hệ thống được thiết kế để từ chối các dữ liệu sai một cách an toàn và hiển thị thông báo lỗi rõ ràng ra `stderr`, tuyệt đối không để xảy ra tình trạng sập chương trình (crash), tràn bộ nhớ hay vòng lặp vô tận.

**Bảng ma trận kiểm thử dữ liệu ngoại lệ:**

| # | Trường hợp kiểm thử | Dữ liệu đầu vào thử nghiệm | Hành vi mong đợi của chương trình | Kết quả thực tế | Đánh giá |
|:---:|---|---|---|---|:---:|
| 1 | Số lượng tiến trình $n = 0$ | Nhập `0` khi hỏi số tiến trình | Báo lỗi và yêu cầu nhập lại trong khoảng $[1, 100]$ | In thông báo `Nhap mot so nguyen tu 1 den 100` | **ĐẠT** |
| 2 | Vượt quá $MAX\_PROCESSES$ | File chứa 101 tiến trình | Báo lỗi quá 100 tiến trình, từ chối nạp file | Báo `qua 100 process`, nạp 0 process | **ĐẠT** |
| 3 | Burst time $\le 0$ | `1 0 0 1` hoặc `1 0 -5 1` | Báo lỗi burst $\le 0$ và từ chối | Báo `Loi: P1 co burst <= 0.`, giữ dữ liệu cũ | **ĐẠT** |
| 4 | Arrival time âm ($AT < 0$) | `1 -2 5 1` | Báo lỗi arrival $< 0$ | Báo `Loi: P1 co arrival < 0.` | **ĐẠT** |
| 5 | Priority âm | `1 0 5 -1` | Báo lỗi priority $< 0$ | Báo `Loi: P1 co priority < 0.` | **ĐẠT** |
| 6 | Trùng mã định danh PID | `1 0 4 1` và `1 2 3 2` | Báo lỗi trùng PID | Báo `Loi: pid 1 bi trung.` | **ĐẠT** |
| 7 | PID trùng với `PID_IDLE` | `-1 0 5 1` | Báo lỗi PID trùng mã rảnh của hệ thống | Báo `Loi: process co pid = -1 (danh rieng cho idle)` | **ĐẠT** |
| 8 | Vượt giới hạn thời gian | $AT = 4000, BT = 200$ ($\sum > 4096$) | Báo lỗi vượt ngưỡng 4096 đơn vị thời gian | Báo `vuot gioi han 4096` | **ĐẠT** |
| 9 | Lỗi cú pháp trong file | `1 0 abc 2` hoặc thừa cột | Báo lỗi cú pháp kèm số dòng cụ thể | Báo `tests/err.txt:1: can dung 4 so nguyen...` | **ĐẠT** |
| 10 | File không tồn tại | Đường dẫn `tests/khong_co.txt` | Báo lỗi không mở được file, không crash | Báo `Khong mo duoc file '...'` | **ĐẠT** |

---

### 4.4. Kết quả các thuật toán trên cùng dữ liệu thực tế
Sử dụng bộ dữ liệu chuẩn mực `tests/d_comprehensive_compare.txt` gồm 5 tiến trình:
- $P_1$: $AT = 0, BT = 5, Priority = 3$
- $P_2$: $AT = 1, BT = 3, Priority = 1$
- $P_3$: $AT = 2, BT = 8, Priority = 4$
- $P_4$: $AT = 3, BT = 6, Priority = 2$
- $P_5$: $AT = 4, BT = 4, Priority = 5$

**Kết quả chi tiết từng thuật toán:**

#### 1. First-Come First-Served (FCFS)
- **Sơ đồ Gantt:**
  ```text
  +----------+------+----------------+------------+--------+
  |    P1    |  P2  |       P3       |     P4     |   P5   |
  +----------+------+----------------+------------+--------+
  0          5      8                16           22       26
  ```
- **Bảng chỉ số:**
  - $P_1$: $CT = 5, TAT = 5, WT = 0, RT = 0$
  - $P_2$: $CT = 8, TAT = 7, WT = 4, RT = 4$
  - $P_3$: $CT = 16, TAT = 14, WT = 6, RT = 6$
  - $P_4$: $CT = 22, TAT = 19, WT = 13, RT = 13$
  - $P_5$: $CT = 26, TAT = 22, WT = 18, RT = 18$
- **Trung bình:** Avg WT = **8.20**, Avg TAT = **13.40**, Avg RT = **8.20**, CPU Util = **100.00%**.

#### 2. Shortest Job First (SJF - không ngắt)
- **Sơ đồ Gantt:**
  ```text
  +----------+------+--------+------------+----------------+
  |    P1    |  P2  |   P5   |     P4     |       P3       |
  +----------+------+--------+------------+----------------+
  0          5      8        12           18               26
  ```
- **Bảng chỉ số:**
  - $P_1$: $CT = 5, TAT = 5, WT = 0, RT = 0$
  - $P_2$: $CT = 8, TAT = 7, WT = 4, RT = 4$
  - $P_5$: $CT = 12, TAT = 8, WT = 4, RT = 4$
  - $P_4$: $CT = 18, TAT = 15, WT = 9, RT = 9$
  - $P_3$: $CT = 26, TAT = 24, WT = 16, RT = 16$
- **Trung bình:** Avg WT = **6.60**, Avg TAT = **11.80**, Avg RT = **6.60**, CPU Util = **100.00%**.

#### 3. Shortest Remaining Time First (SRTF - có ngắt)
- **Sơ đồ Gantt:**
  ```text
  +---+------+--------+--------+------------+----------------+
  |P1 |  P2  |   P1   |   P5   |     P4     |       P3       |
  +---+------+--------+--------+------------+----------------+
  0   1      4        8        12           18               26
  ```
- **Bảng chỉ số:**
  - $P_1$: $CT = 8, TAT = 8, WT = 3, RT = 0$
  - $P_2$: $CT = 4, TAT = 3, WT = 0, RT = 0$
  - $P_5$: $CT = 12, TAT = 8, WT = 4, RT = 4$
  - $P_4$: $CT = 18, TAT = 15, WT = 9, RT = 9$
  - $P_3$: $CT = 26, TAT = 24, WT = 16, RT = 16$
- **Trung bình:** Avg WT = **6.40**, Avg TAT = **11.60**, Avg RT = **5.80**, CPU Util = **100.00%**.

#### 4. Priority Scheduling (Không ngắt)
- **Sơ đồ Gantt:**
  ```text
  +----------+------+------------+----------------+--------+
  |    P1    |  P2  |     P4     |       P3       |   P5   |
  +----------+------+------------+----------------+--------+
  0          5      8            14               22       26
  ```
- **Trung bình:** Avg WT = **7.80**, Avg TAT = **13.00**, Avg RT = **7.80**, CPU Util = **100.00%**.

#### 5. Priority Scheduling (Có ngắt)
- **Sơ đồ Gantt:**
  ```text
  +---+------+------------+--------+----------------+--------+
  |P1 |  P2  |     P4     |   P1   |       P3       |   P5   |
  +---+------+------------+--------+----------------+--------+
  0   1      4            10       14               22       26
  ```
- **Trung bình:** Avg WT = **8.00**, Avg TAT = **13.20**, Avg RT = **6.20**, CPU Util = **100.00%**.

#### 6. Round Robin (q = 2)
- **Sơ đồ Gantt:**
  ```text
  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  |P1 |P2 |P3 |P1 |P4 |P5 |P2 |P3 |P1 |P4 |P5 |P3 |P4 |P3 |
  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  0   2   4   6   8  10  12  13  15  16  18  20  22  24  26
  ```
- **Trung bình:** Avg WT = **12.60**, Avg TAT = **17.80**, Avg RT = **2.80**, CPU Util = **100.00%**.

---

### 4.5. Bảng so sánh tổng hợp
Bảng tổng hợp trích xuất trực tiếp từ tính năng `compare_all` của chương trình:

| STT | Thuật toán lập lịch | Avg WT | Avg TAT | Avg RT | CPU Utilization | Đánh giá tổng quan |
|:---:|---|:---:|:---:|:---:|:---:|---|
| 1 | **FCFS** | 8.20 | 13.40 | 8.20 | 100.00% | Đơn giản, không overhead nhưng dễ bị convoy effect |
| 2 | **SJF (không ngắt)** | 6.60 | 11.80 | 6.60 | 100.00% | WT tốt hơn FCFS rõ rệt, không ngắt |
| 3 | **SRTF (có ngắt)** | **6.40** | **11.60** | 5.80 | 100.00% | **Tối ưu nhất về Waiting Time và Turnaround Time** |
| 4 | **Priority (không ngắt)** | 7.80 | 13.00 | 7.80 | 100.00% | Ưu tiên tiến trình quan trọng |
| 5 | **Priority (có ngắt)** | 8.00 | 13.20 | 6.20 | 100.00% | Phản hồi tiến trình ưu tiên cao nhanh hơn |
| 6 | **Round Robin (q = 2)** | 12.60 | 17.80 | **2.80** | 100.00% | **Tối ưu nhất về Response Time**, công bằng cao |

**Biểu đồ so sánh thời gian chờ (Avg WT) và thời gian phản hồi (Avg RT):**
```text
Thuật toán       Avg WT (đơn vị)                Avg RT (đơn vị)
--------------------------------------------------------------------------------
FCFS             |========> 8.20                |========> 8.20
SJF              |=======> 6.60                 |=======> 6.60
SRTF             |======> 6.40 (Min)            |======> 5.80
Priority (NoPre) |=======> 7.80                 |=======> 7.80
Priority (Pre)   |========> 8.00                |======> 6.20
Round Robin(q=2) |============> 12.60           |===> 2.80 (Min)
--------------------------------------------------------------------------------
```

---

### 4.6. Nhận xét và khuyến nghị ứng dụng
Qua phân tích đối sánh thực nghiệm trên các bộ dữ liệu đa dạng, nhóm rút ra các kết luận chuyên sâu:
1. **SRTF đạt hiệu suất thời gian chờ tối ưu nhất:** SRTF đạt Avg WT thấp nhất (6.40 đơn vị) và Avg TAT thấp nhất (11.60 đơn vị). Nhờ khả năng ngắt tiến trình đang chạy ngay khi có tiến trình ngắn hơn xuất hiện, hệ thống giải phóng các tiến trình nhỏ cực nhanh. Tuy nhiên, thuật toán này khó áp dụng trong thực tế vì hệ điều hành không thể biết trước thời gian burst tiếp theo của tiến trình người dùng, đồng thời có nguy cơ bỏ đói các tiến trình dài nếu các tiến trình ngắn liên tục xuất hiện.
2. **Round Robin là vua của hệ thống tương tác (Time-Sharing Systems):** Dù có Avg WT cao nhất (12.60) do việc chia nhỏ CPU thành nhiều lát cắt thời gian, Round Robin lại đạt Avg RT vượt trội (chỉ 2.80 đơn vị, nhanh hơn gấp gần 3 lần FCFS). Mọi tiến trình đều nhận được phản hồi ban đầu gần như ngay lập tức. Đây là lý do cốt lõi khiến các hệ điều hành hiện đại như Linux hay Windows đều dựa trên nền tảng Round Robin đa mức độ ưu tiên.
3. **Ảnh hưởng quyết định của Time Quantum trong Round Robin:**
   - Khi $q \to \infty$ (hoặc $q \ge \max(BT)$): Round Robin suy biến hoàn toàn thành FCFS.
   - Khi $q$ quá nhỏ: Thời gian phản hồi được tối ưu nhưng trong hệ thống thực tế sẽ gánh chịu tổn thất chuyển ngữ cảnh (context switch overhead) vô cùng lớn.
4. **Vấn đề bỏ đói (Starvation) trong Priority:**
   Trong bộ test `d_starvation_priority.txt`, tiến trình $P_1$ có độ ưu tiên thấp bị trì hoãn tới 12 đơn vị thời gian. Trong hệ thống tải cao thực tế, tiến trình này có thể không bao giờ được chạy nếu không có cơ chế Tăng tuổi (Aging) định kỳ.

---

### 4.7. Lỗi tìm được khi test chéo (Cross-Testing)
Thực hiện đúng cam kết trong kế hoạch làm việc, Thành viên D đã tiến hành rà soát mã nguồn và kiểm thử độc lập đối với module của Thành viên A (`src/input.c`, `src/util.c`, `src/fcfs.c`).

**Kết quả ghi nhận:**
- **Lỗi phát hiện:** Trong hàm `validate_processes` (`src/input.c`), điều kiện kiểm tra PID hiện tại chỉ là:
  ```c
  if (p[i].pid == PID_IDLE) { ... }
  ```
  Khi người dùng nhập một tiến trình có PID âm khác $-1$ (chẳng hạn `pid = -2`), hàm kiểm tra không phát hiện và vẫn cho phép nạp vào hệ thống.
- **Hệ quả:** Bảng thông tin in ra nhãn `P-2`, gây mất tính nhất quán với các hệ thống POSIX/Linux (nơi PID bắt buộc phải là số nguyên dương $\ge 1$) và tiềm ẩn nguy cơ xung đột nếu sau này hệ thống mở rộng các mã đặc biệt âm khác.
- **Trạng thái:** Đã ghi nhận đầy đủ vào tài liệu `docs/loi_test_cheo.md` để Thành viên A cập nhật điều kiện kiểm tra thành `p[i].pid <= 0 || p[i].pid == PID_IDLE`.

---

## CHƯƠNG 5. KẾT LUẬN

### 5.1. Kết quả đạt được
Đồ án đã hoàn thành xuất sắc toàn bộ các mục tiêu đặt ra ban đầu theo chuẩn kỹ thuật chung:
1. **Hoàn thiện đầy đủ các thuật toán cốt lõi:** Cài đặt thành công 5 thuật toán cơ bản: FCFS, SJF không ngắt, SRTF, Priority (cả hai chế độ ngắt và không ngắt), Round Robin (tham số quantum động).
2. **Kiến trúc module hóa chuẩn mực:** Phân chia trách nhiệm rõ ràng giữa các thành viên, các module giao tiếp thông qua header `include/scheduler.h`, bảo toàn dữ liệu nguồn tuyệt đối và không đảo thứ tự mảng.
3. **Giao diện trực quan và phân tích chuyên sâu:** Hiển thị sơ đồ Gantt dạng văn bản cân đối, tự ngắt dòng; tính toán chính xác 100% các chỉ số CT, TAT, WT, RT và CPU utilization; xây dựng tính năng `compare_all` so sánh trực quan đa thuật toán.
4. **Độ ổn định cao:** Hệ thống kiểm tra dữ liệu đầu vào chặt chẽ, loại bỏ hoàn toàn các nguy cơ crash do dữ liệu bất thường.

---

### 5.2. Hạn chế
Mặc dù đã hoàn thành tốt các yêu cầu đồ án, chương trình vẫn còn một số giả định đơn giản hóa:
1. **Bỏ qua chi phí chuyển ngữ cảnh (Context Switch Overhead):** Thời gian chuyển đổi giữa hai tiến trình được coi bằng 0. Trong thực tế, việc lưu và khôi phục thanh ghi phần cứng (PCB) làm tiêu tốn chu kỳ CPU đáng kể.
2. **Mô hình CPU burst thuần túy:** Mỗi tiến trình chỉ có một khoảng thời gian chạy CPU duy nhất, chưa mô phỏng được hành vi I/O bound xen kẽ với CPU bound.
3. **Giả định đơn bộ xử lý (Single CPU):** Chưa hỗ trợ kiến trúc đa lõi (Multicore/Multiprocessor scheduling) với các vấn đề phân chia tải (load balancing) và processor affinity.
4. **Chưa có cơ chế Aging tự động:** Trong thuật toán Priority, chưa tự động tăng độ ưu tiên cho các tiến trình chờ quá lâu để giải quyết triệt để vấn đề starvation.

---

### 5.3. Hướng phát triển
1. **Cài đặt giải thuật MLFQ (Multilevel Feedback Queue):** Phân chia nhiều hàng đợi với các mức ưu tiên và time quantum khác nhau, tự động điều chỉnh mức độ ưu tiên dựa trên hành vi sử dụng CPU của tiến trình.
2. **Tích hợp cơ chế Aging và cấu hình chi phí chuyển ngữ cảnh:** Cho phép người dùng nhập tham số $\delta$ thời gian context switch để quan sát sự suy giảm hiệu suất thực tế của Round Robin khi quantum quá nhỏ.
3. **Mô phỏng đa nhân (Multicore Scheduling):** Mở rộng cấu trúc dữ liệu để theo dõi song song trạng thái của 2 đến 4 CPU core.
4. **Xây dựng giao diện đồ họa (GUI / Web Dashboard):** Nâng cấp từ console sang giao diện đồ họa tương tác trực quan (sử dụng thư viện ncurses hoặc ứng dụng web hiện đại) giúp việc demo và giảng dạy sinh động hơn.

---

## PHỤ LỤC

### A. TÀI LIỆU THAM KHẢO
1. Abraham Silberschatz, Peter Baer Galvin, and Greg Gagne (2018). *Operating System Concepts*, 10th Edition. John Wiley & Sons, Inc. (Chương 5: CPU Scheduling, trang 199–246).
2. Andrew S. Tanenbaum and Herbert Bos (2014). *Modern Operating Systems*, 4th Edition. Pearson Education. (Chương 2: Processes and Threads).
3. William Stallings (2018). *Operating Systems: Internals and Design Principles*, 9th Edition. Pearson Education.
4. Daniel P. Bovet and Marco Cesati (2005). *Understanding the Linux Kernel*, 3rd Edition. O'Reilly Media.
5. IEEE & The Open Group (2018). *The Open Group Base Specifications Issue 7 / IEEE Std 1003.1-2017 (POSIX.1)*. System Interfaces & Scheduling Policies.

---

### B. MÃ NGUỒN TRÍCH DẪN CÁC HÀM THEN CHỐT DO THÀNH VIÊN D PHÁT TRIỂN

#### 1. Hàm thêm và gộp sơ đồ Gantt (`src/gantt.c`)
```c
int gantt_append(GanttEntry g[], int *g_len, int pid, int start, int end)
{
    if (start >= end)
        return -1;

    if (*g_len == 0) {
        if (start != 0)
            return -1;
    } else {
        GanttEntry *last = &g[*g_len - 1];
        if (start != last->end)
            return -1;
        if (last->pid == pid) {
            last->end = end;
            return 0;
        }
    }

    if (*g_len >= MAX_GANTT)
        return -1;

    g[*g_len].pid = pid;
    g[*g_len].start = start;
    g[*g_len].end = end;
    (*g_len)++;
    return 0;
}
```

#### 2. Hàm suy dẫn chỉ số hiệu năng và CPU Utilization (`src/metrics.c`)
```c
void calc_metrics(Process p[], int n, const GanttEntry g[], int g_len, Metrics *out)
{
    if (!out) return;
    if (n <= 0) {
        out->avg_waiting = out->avg_turnaround = out->avg_response = out->cpu_utilization = 0.0;
        return;
    }

    double sum_wt = 0.0, sum_tat = 0.0, sum_rt = 0.0;
    for (int i = 0; i < n; i++) {
        int first_start = -1, last_end = 0;
        for (int k = 0; k < g_len; k++) {
            if (g[k].pid == p[i].pid) {
                if (first_start < 0) first_start = g[k].start;
                last_end = g[k].end;
            }
        }
        if (first_start >= 0) {
            p[i].completion_time = last_end;
            p[i].turnaround_time = last_end - p[i].arrival_time;
            p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
            p[i].response_time = first_start - p[i].arrival_time;
        } else {
            p[i].completion_time = p[i].turnaround_time = p[i].waiting_time = p[i].response_time = 0;
        }
        sum_wt += p[i].waiting_time;
        sum_tat += p[i].turnaround_time;
        sum_rt += p[i].response_time;
    }

    out->avg_waiting = sum_wt / n;
    out->avg_turnaround = sum_tat / n;
    out->avg_response = sum_rt / n;

    int busy_time = 0;
    for (int k = 0; k < g_len; k++) {
        if (g[k].pid != PID_IDLE)
            busy_time += (g[k].end - g[k].start);
    }
    int total_time = (g_len > 0) ? g[g_len - 1].end : 0;
    out->cpu_utilization = (total_time > 0) ? (100.0 * busy_time / total_time) : 0.0;
}
```

#### 3. Hàm so sánh tất cả các thuật toán trên cùng dữ liệu (`src/compare.c`)
```c
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
        case 0: schedule_fcfs(p, n, g, &g_len); break;
        case 1: schedule_sjf(p, n, g, &g_len); break;
        case 2: schedule_srtf(p, n, g, &g_len); break;
        case 3: schedule_priority(p, n, g, &g_len, 0); break;
        case 4: schedule_priority(p, n, g, &g_len, 1); break;
        case 5: schedule_rr(p, n, g, &g_len, quantum); break;
        }

        if (g_len > 0) {
            results[i].available = 1;
            calc_metrics(p, n, g, g_len, &results[i].metrics);
            results[i].total_duration = g[g_len - 1].end;
        } else {
            results[i].available = 0;
        }
    }
    /* In bang tong hop va phan tich ket qua ... */
}
```
