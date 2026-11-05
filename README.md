# CPU-Scheduling-Algorithms

Đồ án Lập trình hệ thống, đề tài 15: chương trình C minh họa các thuật toán CPU scheduling (FCFS, SJF, SRTF, Priority, Round Robin). Chương trình in sơ đồ Gantt và bảng CT, TAT, WT, RT cho từng thuật toán, có chức năng chạy cùng một dữ liệu qua tất cả thuật toán để so sánh.

## Build và chạy

Cần gcc và make (Linux).

```
make            # tạo ./scheduler
./scheduler
make test       # chạy test tự động, dòng cuối phải là "0 loi"
make clean      # xóa build/ và ./scheduler
```

Trong menu, chọn 1 để nhập process bằng tay hoặc 2 để đọc từ file, rồi chọn thuật toán. Mục 9 chạy cùng dữ liệu qua cả sáu cấu hình và in bảng so sánh.

## Định dạng file input

Mỗi dòng một process: `pid arrival burst priority`. Dòng trống và dòng bắt đầu bằng `#` được bỏ qua. Không viết `#` sau dữ liệu trên cùng một dòng.

```
# pid arrival burst priority
2 3 2 1
3 8 1 2
1 2 3 3
```

Các file mẫu nằm trong `tests/`.

## Cấu trúc

```
include/scheduler.h   struct, hằng số, khai báo hàm, các quy ước chung
src/main.c            menu
src/input.c           nhập tay, đọc file, validate_processes
src/util.c            sort_by_arrival
src/fcfs.c            FCFS
src/sjf.c, srtf.c     B
src/priority.c, rr.c  C
src/gantt.c, metrics.c, compare.c   D
tests/                file input mẫu, test_all.c (make test)
docs/                 hướng dẫn chạy demo, kịch bản demo, lỗi tìm được khi test chéo
report/               báo cáo Word
slides/               slide
```

## Cho người viết module

Trước khi viết code, đọc block comment ở đầu `include/scheduler.h`. Các điểm chính:

- Hàm có dạng `void schedule_xxx(Process p[], int n, GanttEntry g[], int *g_len)`. Priority thêm tham số `preemptive`, Round Robin thêm `quantum`, đặt ở cuối.
- Không sort hay đảo `p[]`. Cần thứ tự khác thì dùng mảng chỉ số, ví dụ `sort_by_arrival()`.
- Ghi Gantt bằng `gantt_append()`. Gantt bắt đầu từ t = 0, liền mạch, đoạn CPU rảnh tự ghi với `PID_IDLE`.
- Thuật toán chỉ điền Gantt. CT, TAT, WT, RT do `calc_metrics()` tính.
- Đầu hàm phải đặt `*g_len = 0` và `remaining_time = burst_time`.
- Priority: số nhỏ là ưu tiên cao. Hòa thì xét arrival, rồi vị trí trong `p[]`.

Sửa module nào thì chạy `make test` trước khi gửi pull request.

## Làm việc với git

- Mỗi người một nhánh tạo từ `main`, không push thẳng lên `main`. A merge.
- `main` luôn phải build được. Trước khi push chạy `make` và đảm bảo không có warning.
- Muốn sửa `scheduler.h` thì báo cả nhóm trước, vì mọi module dựa vào nó.
- Nhóm 4 người, không làm MLFQ.
