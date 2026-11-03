# Kịch bản demo

Demo chiếm 40% điểm. D điều phối, A cầm máy. Mỗi người chạy và giải thích thuật toán của mình. Tổng thời gian khoảng 10-12 phút, tùy giảng viên cho.

Mọi số dưới đây là output thật của chương trình. Lúc tập, nếu máy ra số khác thì dừng lại tìm lỗi trước, đừng đi tiếp.

## Chuẩn bị (trước buổi demo)

```
git checkout main && git pull
make clean && make
make test
```

- `make` không được có warning, `make test` phải in `0 loi` ở dòng cuối.
- Mở sẵn terminal ở thư mục repo, cỡ chữ to để cả lớp nhìn thấy. Thu nhỏ terminal thì Gantt bị xuống dòng, nên để cửa sổ rộng.
- Máy dự phòng đã clone và build sẵn, đã chạy thử đủ 5 phần dưới.

## 1. Demo cơ bản (khoảng 4 phút)

Chạy `./scheduler`, chọn `2`, gõ `tests/demo_co_ban.txt`. Chọn `3` để cả lớp thấy dữ liệu: 5 process, đến lần lượt từ 0 đến 4.

Mỗi người chạy thuật toán của mình và nói một câu về Gantt:

| Ai | Chọn | Điểm cần chỉ ra | Avg WT |
|---|---|---|---|
| A | `4` FCFS | Chạy đúng thứ tự đến, không ngắt | 11.40 |
| B | `5` SJF | Lúc 8, P5 (burst 2) được chọn trước P2, P4 | 8.20 |
| B | `6` SRTF | Có ngắt, WT thấp nhất cả bảng | 6.60 |
| C | `7`, rồi `0` | Priority không ngắt | 10.60 |
| C | `7`, rồi `1` | Priority có ngắt, so với không ngắt | 9.60 |
| C | `8`, quantum `2` | RR: RT thấp nhất nhưng WT cao nhất | 13.00 |

## 2. Câu chuyện convoy effect (khoảng 2 phút, A và B)

Chọn `2`, gõ `tests/demo_convoy.txt`. Có 5 process cùng đến lúc 0: P1 cần 20, bốn process còn lại cần 2.

- Chọn `4` (FCFS): P1 chạy trước, bốn process ngắn chờ. Avg WT = **18.40**.
- Chọn `5` (SJF): bốn process ngắn chạy trước, P1 chạy cuối. Avg WT = **4.00**.

Câu để nói: cùng một lượng công việc, chỉ đổi thứ tự mà thời gian chờ giảm hơn bốn lần. Đó là convoy effect.

## 3. Câu chuyện starvation (khoảng 2 phút, C)

Chọn `2`, gõ `tests/demo_starvation.txt`. P1 có độ ưu tiên thấp (5) và đến lúc 0. Các process ưu tiên cao (1) đến liên tục lúc 0, 2, 5, 8, 11.

- Chọn `7`, rồi `1` (Priority có ngắt): P1 nằm cuối Gantt, chờ **15** đơn vị mới được chạy lần đầu (dòng P1: WT 15, RT 15).
- Chọn `8`, quantum `2` (RR): P1 được chạy ngay lúc 0 (RT 0), WT còn **4**.

Câu để nói: nếu process ưu tiên cao cứ đến mãi thì P1 không bao giờ chạy. Có hai cách chữa: aging (tăng dần độ ưu tiên theo thời gian chờ), hoặc dùng RR để ai cũng được chia CPU. Chương trình không cài aging, chỉ trình bày trong báo cáo (mục 2.3).

## 4. So sánh tất cả (khoảng 2 phút, D)

Chọn `2`, gõ `tests/demo_co_ban.txt`, rồi chọn `9` với quantum `2`. Kết quả:

```
 Thuat toan               |     Avg WT |    Avg TAT |     Avg RT |   CPU Util | Trang thai
----------------------------------------------------------------------------------------
 FCFS                     |      11.40 |      17.00 |      11.40 |    100.00% | San sang
 SJF (khong ngat)         |       8.20 |      13.80 |       8.20 |    100.00% | San sang
 SRTF                     |       6.60 |      12.20 |       4.40 |    100.00% | San sang
 Priority (khong ngat)    |      10.60 |      16.20 |      10.60 |    100.00% | San sang
 Priority (ngat)          |       9.60 |      15.20 |       7.80 |    100.00% | San sang
 Round Robin (q = 2)      |      13.00 |      18.60 |       2.80 |    100.00% | San sang

--- Danh gia so bo ---
 - Thoi gian cho trung binh (Avg WT) tot nhat     : SRTF                   (6.60)
 - Thoi gian luu lai trung binh (Avg TAT) tot nhat: SRTF                   (12.20)
 - Thoi gian dap ung trung binh (Avg RT) tot nhat : Round Robin (q = 2)    (2.80)
```

Chương trình tự in phần "Danh gia so bo" bên dưới bảng, D chỉ vào đó khi nói.

Câu để nói: SRTF thấp nhất về WT và TAT, vì nó luôn ưu tiên việc sắp xong. RR thấp nhất về RT, vì ai cũng được chạy sớm, đổi lại WT cao. Không có thuật toán nào tốt nhất ở mọi tiêu chí.

## 5. Đọc file và dữ liệu sai (khoảng 1 phút, A)

- Chọn `2`, gõ `tests/demo_loi.txt` (pid 2 bị trùng). Chương trình báo `Loi: pid 2 bi trung.` và `giu nguyen du lieu cu`. Chọn `3` để thấy dữ liệu cũ vẫn còn.
- Chọn `1`, nhập tay một process có burst `0`. Chương trình báo lỗi và cho nhập lại đúng process đó.

Câu để nói: mọi nguồn dữ liệu đi qua `validate_processes` trước khi chạy, nên thuật toán không bao giờ nhận dữ liệu hỏng.

## Câu hỏi giảng viên hay hỏi

Mỗi người phải trả lời được câu hỏi về code của người khác, không chỉ phần của mình.

- **Vì sao WT = TAT - BT?** Từ lúc đến tới lúc xong, process hoặc đang chạy (đúng BT đơn vị) hoặc đang chờ.
- **Không ngắt thì RT bằng WT, có ngắt thì sao?** Process chạy một mạch thì lần đầu chạy cũng là lần duy nhất. Có ngắt thì process được chạy một phần rồi bị chen, nên RT nhỏ hơn WT.
- **Vì sao không sort thẳng mảng `p[]`?** Bảng kết quả phải theo thứ tự nhập, và các thuật toán chạy trên cùng dữ liệu. Muốn đổi thứ tự thì dùng mảng chỉ số.
- **Ai tính CT, TAT, WT?** Chỉ `calc_metrics`, tính từ Gantt. Thuật toán chỉ ghi Gantt, nên công thức chỉ có một chỗ.
- **Hai process hòa nhau thì sao?** SJF, SRTF hòa thì xét thời điểm đến rồi thứ tự nhập. Priority cũng vậy, số nhỏ là ưu tiên cao. RR: process mới đến đúng lúc hết quantum thì vào hàng trước process vừa bị ngắt.
- **Quantum ảnh hưởng thế nào?** Quantum nhỏ thì RT tốt nhưng chuyển ngữ cảnh nhiều. Với dữ liệu `tests/vd_rr.txt`, q = 1 chuyển 9 lần. Quantum lớn hơn mọi burst thì RR thành FCFS (q = 24 ra WT 17, bằng FCFS).
- **Làm sao biết thuật toán đúng?** So với ví dụ tính tay trong báo cáo, và chạy `make test`: test so 26 ví dụ có đáp án, thử 3000 bộ dữ liệu ngẫu nhiên, rồi thử 8 trường hợp dữ liệu vào sai. Trong đó có kiểm tra SRTF luôn cho WT trung bình thấp nhất (đây là tính chất đã chứng minh được).
