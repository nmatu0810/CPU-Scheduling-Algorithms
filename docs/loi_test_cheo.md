# Lỗi tìm được khi test chéo

Vòng test: A test code của B, B test C, C test D, D test A. Mỗi người tự nghĩ input để thử module của người kế tiếp, mỗi lỗi ghi một dòng vào bảng. Người viết module sửa xong thì đổi trạng thái thành "đã sửa" và ghi commit sửa.

| # | Người test | Module | Input gây lỗi (tên file hoặc dữ liệu) | Mong đợi | Thực tế | Trạng thái |
|---|---|---|---|---|---|---|
| 1 | D | validate_processes (input.c) | Tiến trình có `pid < 0` khác -1 (ví dụ `-2 0 5 1`) | Báo lỗi và từ chối vì PID phải là số không âm (`pid >= 0`), tránh xung đột quy ước `PID_IDLE = -1` | `validate_processes` chỉ chặn `pid == PID_IDLE (-1)` nên chấp nhận `pid = -2`, in ra `P-2` | Đã sửa: chặn mọi `pid < 0`, thêm ca kiểm tra trong `tests/test_all.c` |

## Các vòng đã chạy

Vòng nào không tìm ra lỗi thì ghi ở đây cách đã thử, để biết module đó đã được kiểm tra những gì.

| Vòng | Module | Đã thử | Kết quả |
|---|---|---|---|
| A test B | sjf.c, srtf.c | So Gantt với bản cài độc lập trên 20000 bộ dữ liệu ngẫu nhiên (1 đến 100 process, nhiều ca hòa, pid không theo thứ tự); gọi hàm hai lần trên cùng mảng; n = 0; một process burst 4096; CPU rảnh 0 đến 4000; ba file `tests/b_*.txt` | Không có lỗi |
| B test C | priority.c, rr.c | So Gantt với bản cài độc lập như trên, cả hai chế độ Priority và RR quantum 1 đến 6; RR quantum 0 và âm; RR q = 1 với 100 process (4000 đoạn Gantt); chạy `tests/c_verify.py` | Không có lỗi |
| C test D | gantt.c, metrics.c, compare.c | `gantt_append` với start khác 0, start = end, hở, chồng, mảng đầy (không gộp được thì từ chối, gộp được thì vẫn nhận); `print_gantt` với 4096 đoạn và pid 2147483647, kiểm tra mốc thời gian nằm đúng dưới dấu +; `calc_metrics` với n = 0 và CPU rảnh dài; `compare_all` với 100 process | Không có lỗi. Ghi nhận: con số mốc cuối dòng Gantt có thể vượt 72 cột thêm 1 đến 3 ký tự, vẫn vừa terminal 80 cột nên không sửa |
| D test A | input.c, main.c | Xem lỗi số 1 ở bảng trên. Thêm sau khi sửa: file xuống dòng kiểu Windows (CRLF), cột cách bằng tab, pid 0, file 100 process | Lỗi số 1 đã sửa, các ca thêm đều đúng |

Mọi lần chạy đều dùng code trên `main` sau khi merge, có bật AddressSanitizer và UBSan.
