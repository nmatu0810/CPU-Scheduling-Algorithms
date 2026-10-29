# Lỗi tìm được khi test chéo

Vòng test: A test code của B, B test C, C test D, D test A. Mỗi người tự nghĩ input để thử module của người kế tiếp, mỗi lỗi ghi một dòng vào bảng. Người viết module sửa xong thì đổi trạng thái thành "đã sửa" và ghi commit sửa.

| # | Người test | Module | Input gây lỗi (tên file hoặc dữ liệu) | Mong đợi | Thực tế | Trạng thái |
|---|---|---|---|---|---|---|
| 1 | D | validate_processes (input.c) | Tiến trình có `pid < 0` khác -1 (ví dụ `-2 0 5 1`) | Báo lỗi và từ chối vì PID phải là số không âm (`pid >= 0`), tránh xung đột quy ước `PID_IDLE = -1` | `validate_processes` chỉ chặn `pid == PID_IDLE (-1)` nên chấp nhận `pid = -2`, in ra `P-2` | Chưa sửa |
