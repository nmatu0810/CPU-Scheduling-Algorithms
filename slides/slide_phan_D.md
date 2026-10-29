# PHẦN SLIDE THUYẾT TRÌNH VÀ KỊCH BẢN ĐIỀU PHỐI DEMO
## THÀNH VIÊN D: KẾT QUẢ, SO SÁNH, KẾT LUẬN & ĐIỀU PHỐI DEMO

---

## 1. NỘI DUNG SLIDE THUYẾT TRÌNH (SLIDE 14 - 18)

### SLIDE 14: BẢNG SO SÁNH HIỆU NĂNG TỔNG HỢP CÁC THUẬT TOÁN
**Tiêu đề slide:** ĐỐI SÁNH HIỆU NĂNG ĐA MỤC TIÊU TRÊN CÙNG TẬP DỮ LIỆU
**Dữ liệu thử nghiệm:** 5 tiến trình ($P_1 \dots P_5$) với thời điểm đến, burst time và priority đa dạng.

*Bảng dữ liệu trích xuất từ chức năng `compare_all`:*

| Thuật toán | Avg Waiting Time (WT) | Avg Turnaround Time (TAT) | Avg Response Time (RT) | CPU Utilization |
|---|:---:|:---:|:---:|:---:|
| **FCFS** | 8.20 | 13.40 | 8.20 | 100.00% |
| **SJF (không ngắt)** | 6.60 | 11.80 | 6.60 | 100.00% |
| **SRTF (có ngắt)** | **6.40 (Tốt nhất)** | **11.60 (Tốt nhất)** | 5.80 | 100.00% |
| **Priority (không ngắt)** | 7.80 | 13.00 | 7.80 | 100.00% |
| **Priority (có ngắt)** | 8.00 | 13.20 | 6.20 | 100.00% |
| **Round Robin (q = 2)** | 12.60 | 17.80 | **2.80 (Tốt nhất)** | 100.00% |

**Speaker Notes (Thành viên D trình bày):**
> *"Kính thưa thầy cô và các bạn, bước sang phần so sánh hiệu năng, module `compare_all` cho phép chúng em chạy đồng thời mọi thuật toán trên cùng một bản sao dữ liệu đầu vào. Nhìn vào bảng số liệu tổng hợp, chúng ta thấy một bức tranh toàn cảnh: không có thuật toán nào tối ưu tuyệt đối ở mọi chỉ số, mà luôn có sự đánh đổi (trade-off) kinh điển trong khoa học máy tính giữa thông lượng, thời gian chờ và độ nhạy phản hồi."*

---

### SLIDE 15: PHÂN TÍCH CHUYÊN SÂU: WAITING TIME VS. RESPONSE TIME
**Tiêu đề slide:** SỰ ĐÁNH ĐỔI GIỮA THỜI GIAN CHỜ VÀ THỜI GIAN PHẢN HỒI

**Biểu đồ đối sánh (Trực quan hóa):**
- **Thời gian chờ trung bình (Avg WT) - Thấp hơn là tốt hơn:**
  - `SRTF`: **6.40** (Tối ưu toán học về $\min WT$)
  - `SJF`: **6.60**
  - `FCFS`: **8.20**
  - `RR (q=2)`: **12.60** (WT cao do chia sẻ luân phiên nhiều vòng)
- **Thời gian phản hồi ban đầu (Avg RT) - Thấp hơn là tốt hơn:**
  - `RR (q=2)`: **2.80** (Nhanh gấp gần 3 lần FCFS!)
  - `SRTF`: **5.80**
  - `FCFS / SJF`: **8.20 / 6.60**

**Điểm cốt lõi:**
- **SRTF tối ưu toán học:** Luôn ưu tiên công việc ngắn nhất còn lại, giải phóng nhanh tiến trình trong hệ thống.
- **Round Robin tối ưu trải nghiệm tương tác:** Dù thời gian chờ tích lũy cao hơn, người dùng nhận được phản hồi đầu tiên gần như tức thì.

**Speaker Notes:**
> *"Biểu đồ này minh họa rõ nét sự đối lập giữa SRTF và Round Robin: Nếu hệ thống cần tối đa hóa thông lượng tổng thể và giảm thiểu thời gian chờ trung bình, SRTF là lựa chọn lý tưởng. Nhưng trong hệ thống chia sẻ thời gian (Time-sharing) có sự tương tác của con người, người dùng không thể chờ hàng chục giây mới thấy phản hồi đầu tiên. Round Robin chấp nhận hy sinh WT để mang lại RT chỉ 2.80 đơn vị thời gian."*

---

### SLIDE 16: HAI CA THỰC NGHIỆM ĐẶC TRƯNG: CONVOY EFFECT & STARVATION
**Tiêu đề slide:** HIỆN TƯỢNG ĐOÀN TÀU VÀ HIỆN TƯỢNG BỎ ĐÓI TIẾN TRÌNH

**1. Convoy Effect (Hiệu ứng đoàn tàu) trong FCFS:**
- Kịch bản: $P_1$ có burst 24 đến trước, theo sau là hai tiến trình nhỏ $P_2, P_3$ (burst 3).
- Kết quả: Trong FCFS, Avg WT lên tới **17.00**. Khi đổi chỗ đưa việc ngắn lên trước (nguyên lý SJF), Avg WT giảm ngoạn mục xuống còn **3.00** (giảm hơn 82% thời gian chờ!).

**2. Starvation (Bỏ đói tiến trình) trong Priority:**
- Kịch bản: $P_1$ có độ ưu tiên thấp (Priority 5, burst 10). Các tiến trình ưu tiên cao liên tục đến ($P_2, P_3, P_4, P_5$).
- Kết quả: $P_1$ bị chiếm quyền (preempt) tại $t=1$ và bị đóng băng suốt 12 đơn vị thời gian cho tới $t=13$.
- Giải pháp lý thuyết: Cơ chế **Aging** (tăng dần mức ưu tiên theo thời gian chờ) hoặc chuyển sang phân phối vòng Round Robin.

**Speaker Notes:**
> *"Trong quá trình thực nghiệm, nhóm đặc biệt kiểm chứng hai hiện tượng kinh điển. Thứ nhất là Convoy effect trong FCFS: một tiến trình 'nặng ký' kéo lê cả đoàn tàu phía sau, khiến thời gian chờ tăng vọt từ 3.00 lên 17.00. Thứ hai là hiện tượng Starvation trong Priority: tiến trình P1 bị bỏ đói liên tục khi các tác vụ ưu tiên cao tràn vào. Đây là cơ sở thực tiễn để các hệ điều hành thương mại phải áp dụng kỹ thuật Aging."*

---

### SLIDE 17: ĐÁNH GIÁ TỔNG KẾT VÀ TÌNH HUỐNG ỨNG DỤNG THỰC TẾ
**Tiêu đề slide:** KHUYẾN NGHỊ LỰA CHỌN THUẬT TOÁN ĐIỀU PHỐI

| Thuật toán | Ưu điểm nổi bật | Nhược điểm cốt lõi | Môi trường áp dụng phù hợp |
|---|---|---|---|
| **FCFS** | Cài đặt cực kỳ đơn giản, không tốn chi phí quản lý overhead | Dễ dính Convoy effect, không phù hợp tiến trình tương tác | Hệ thống xử lý theo lô (Batch Processing), tác vụ in ấn |
| **SJF / SRTF** | Thời gian chờ tối thiểu, thông lượng cao | Không dự đoán chính xác burst time thực tế; dễ gây bỏ đói | Lập trình tính toán khoa học, dự báo thời gian chạy trước |
| **Priority** | Phân cấp rõ ràng độ quan trọng của tác vụ | Nguy cơ Starvation đối với các tiến trình ưu tiên thấp | Hệ thống nhúng, hệ thống thời gian thực (Real-time OS) |
| **Round Robin** | Công bằng, loại bỏ Starvation, thời gian phản hồi xuất sắc | WT trung bình cao; hiệu năng phụ thuộc gắt gao vào Time Quantum | Hệ thống đa nhiệm tương tác (Linux, Windows desktop) |

---

### SLIDE 18: KẾT LUẬN VÀ HƯỚNG PHÁT TRIỂN
**Tiêu đề slide:** TỔNG KẾT ĐỒ ÁN VÀ ĐỊNH HƯỚNG MỞ RỘNG

**1. Kết quả đạt được:**
- Hoàn thiện 100% các thuật toán theo đúng chuẩn interface `scheduler.h`.
- Trực quan hóa sơ đồ Gantt ASCII tự co giãn, căn lề pixel-perfect và tự ngắt dòng.
- Xây dựng bộ công cụ phân tích metrics và bảng so sánh `compare_all` chính xác, khách quan.
- Quy trình kiểm thử chéo và kiểm soát dữ liệu đầu vào bền bỉ.

**2. Hạn chế hiện tại:**
- Giả định thời gian chuyển ngữ cảnh (Context switch overhead) bằng 0.
- Giả định hệ thống đơn vi xử lý (Single CPU) và không có I/O burst xen giữa.

**3. Hướng phát triển trong tương lai:**
- Cài đặt hàng đợi đa cấp có phản hồi (MLFQ - Multilevel Feedback Queue).
- Bổ sung cơ chế Aging và mô phỏng chi phí context switch thực tế.
- Mở rộng mô hình lập lịch cho hệ thống đa nhân (Multicore/Multiprocessor).
- Phát triển giao diện đồ họa web hoặc GUI tương tác sinh động.

---

## 2. KỊCH BẢN ĐIỀU PHỐI DEMO TRỰC TIẾP (40% ĐIỂM)

**Phân vai:**
- **Thành viên D:** Điều phối viên (Host), giới thiệu kịch bản, nêu mục tiêu từng bước và phân tích kết quả sau mỗi lần chạy.
- **Thành viên A:** Thao tác máy chính, điều khiển menu terminal.
- **Thành viên B & C:** Thuyết minh chi tiết khi máy chạy thuật toán do mình cài đặt (B nói về SJF/SRTF; C nói về Priority/RR).

---

### BƯỚC 1: DEMO NẠP DỮ LIỆU TỪ FILE VÀ CHẠY FCFS
- **D nói:** *"Xin kính chào thầy cô, sau đây nhóm em xin phép bắt đầu phần demo chương trình. Đầu tiên, chúng em xin chứng minh chương trình có khả năng đọc dữ liệu cấu hình linh hoạt từ file thay vì hard-code. Bạn A xin mời nạp file `tests/d_idle_interleaved.txt`."*
- **A thao tác:** Chọn menu `2`, gõ `tests/d_idle_interleaved.txt`, sau đó chọn `3` (Xem dữ liệu hiện tại), tiếp tục chọn `4` (FCFS).
- **D nói:** *"Như thầy cô thấy trên màn hình, sơ đồ Gantt hiển thị rõ các khoảng CPU rảnh (IDLE) ở đầu $[0, 2]$ và các đoạn rảnh xen giữa. Trục thời gian được căn lề chuẩn xác dưới các vạch biên độ. Chỉ số CPU Utilization phản ánh chính xác tỷ lệ thời gian CPU bận làm việc."*

---

### BƯỚC 2: CÂU CHUYỆN 1 - HIỆU ỨNG ĐOÀN TÀU (CONVOY EFFECT)
- **D nói:** *"Tiếp theo là Câu chuyện 1: Khảo sát hiện tượng Convoy Effect. Bạn A hãy nạp file `tests/a_convoy_dai_truoc.txt` và chạy FCFS."*
- **A thao tác:** Menu `2` $\to$ nạp `tests/a_convoy_dai_truoc.txt` $\to$ chọn `4` (FCFS).
- **D nói:** *"Tiến trình P1 burst 24 vào trước khiến P2, P3 phải đợi mòn mỏi. Thời gian chờ trung bình lên tới 17.00. Bây giờ, mời bạn B chuyển sang thuật toán SJF trên cùng tập dữ liệu này."*
- **B thuyết minh:** Chọn menu `5` (SJF). B giải thích nguyên lý chọn việc ngắn nhất giúp đảo lại thứ tự, giải phóng P2, P3 tức thì, kéo Avg WT xuống chỉ còn 3.00!

---

### BƯỚC 3: CÂU CHUYỆN 2 - BỎ ĐÓI TIẾN TRÌNH (STARVATION)
- **D nói:** *"Bây giờ chúng ta đến với Câu chuyện 2: Hiện tượng Starvation trong Priority Scheduling. Mời bạn A nạp file `tests/d_starvation_priority.txt`."*
- **A thao tác:** Menu `2` $\to$ `tests/d_starvation_priority.txt` $\to$ chọn `7` (Priority) $\to$ chọn chế độ `1` (Ngắt).
- **C thuyết minh:** Giải thích tiến trình P1 (Priority 5) vừa chạy được 1 đơn vị thời gian thì bị P2 (Priority 1) ngắt. Sau đó các tiến trình ưu tiên cao liên tục vào chiếm CPU khiến P1 bị đóng băng tới $t=13$.
- **D tiếp lời:** *"Để thấy sự khác biệt, bạn A hãy chạy thuật toán Round Robin với quantum = 2 trên cùng dữ liệu này (Menu 8)."*
- **C thuyết minh:** Trong Round Robin, P1 được cấp phát đều đặn từng lát cắt thời gian, không bao giờ bị bỏ đói, chứng minh tính ưu việt của chia sẻ thời gian.

---

### BƯỚC 4: TÍNH NĂNG "SO SÁNH TẤT CẢ" (COMPARE_ALL)
- **D nói:** *"Để có cái nhìn tổng quan nhất, nhóm đã xây dựng tính năng 'So sánh tất cả' (Menu 9). Mời bạn A nạp bộ dữ liệu toàn diện `tests/d_comprehensive_compare.txt` và chọn chức năng 9 với quantum = 2."*
- **A thao tác:** Menu `2` $\to$ `tests/d_comprehensive_compare.txt` $\to$ chọn `9` $\to$ nhập `2`.
- **D phân tích:**
  - Chỉ vào bảng tổng hợp trên màn hình terminal:
  - *Avg WT tốt nhất:* SRTF (**6.40**)
  - *Avg RT tốt nhất:* Round Robin (**2.80**)
  - *Avg TAT tốt nhất:* SRTF (**11.60**)
  - Khẳng định tính nhất quán và khớp 100% giữa lý thuyết tính tay và kết quả chạy máy.

---

### BƯỚC 5: KIỂM THỬ XỬ LÝ DỮ LIỆU NGOẠI LỆ (INPUT VALIDATION)
- **D nói:** *"Cuối cùng, nhóm xin chứng minh tính bền vững của phần mềm trước các thao tác nhập liệu sai. Mời bạn A chọn Menu 1 (nhập tay) và nhập số âm hoặc burst bằng 0."*
- **A thao tác:** Thử nhập số process = 0, nhập burst = 0, nhập PID trùng.
- **D nói:** *"Chương trình lập tức bắt lỗi, in thông báo chỉ dẫn rõ ràng ra stderr và yêu cầu nhập lại, hệ thống không bị crash hay tràn bộ nhớ. Phần demo của nhóm em đến đây là kết thúc, xin trân trọng cảm ơn thầy cô đã lắng nghe!"*
