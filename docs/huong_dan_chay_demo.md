# Hướng dẫn chạy demo

Làm theo từng bước dưới đây trên máy mình. Mỗi bước có ghi số đúng, chạy ra khác thì báo A.

Thứ tự trình bày và câu nói lúc demo nằm ở `docs/kich_ban_demo.md`. File này chỉ hướng dẫn cách chạy.

## 1. Cài công cụ

Cần Linux, có `gcc`, `make` và `git`. Máy Windows thì cài WSL (Ubuntu) rồi làm mọi bước trong cửa sổ Ubuntu.

Ubuntu hoặc WSL:

```
sudo apt update
sudo apt install build-essential git
```

Arch:

```
sudo pacman -S base-devel git
```

Kiểm tra: `gcc --version` và `make --version` đều in ra phiên bản là được.

## 2. Lấy code

Repo để private, mọi người đã được mời vào. Cách dễ nhất là vào trang repo trên GitHub, bấm Code, chọn Download ZIP rồi giải nén.

Nếu đã cài SSH key cho GitHub thì clone:

```
git clone git@github.com:nmatu0810/CPU-Scheduling-Algorithms.git
```

Đã clone từ trước thì chỉ cần lấy bản mới:

```
git checkout main
git pull
```

## 3. Build và kiểm tra

Mở terminal ở thư mục repo:

```
make clean
make
make test
```

`make` không được có dòng `warning`. Dòng cuối của `make test` phải là `0 loi`. Sai một trong hai thì dừng lại, báo A.

Trước khi chạy, kéo cửa sổ terminal cho rộng. Cửa sổ hẹp thì sơ đồ Gantt bị xuống dòng và khó nhìn.

## 4. Chạy

```
./scheduler
```

Trong menu, gõ số rồi Enter. Sau mỗi lần chạy, chương trình quay về menu.

### Phần 1: dữ liệu cơ bản

Gõ `2`, rồi gõ `tests/demo_co_ban.txt`. Màn hình phải báo `Da nap 5 process`. Gõ `3` để xem dữ liệu.

Chạy lần lượt và nhìn dòng `Avg WT` ở cuối mỗi kết quả:

| Gõ | Thuật toán | Avg WT |
|---|---|---|
| `4` | FCFS | 11.40 |
| `5` | SJF | 8.20 |
| `6` | SRTF | 6.60 |
| `7` rồi `0` | Priority không ngắt | 10.60 |
| `7` rồi `1` | Priority có ngắt | 9.60 |
| `8` rồi `2` | Round Robin, quantum 2 | 13.00 |

### Phần 2: convoy effect

Gõ `2`, rồi gõ `tests/demo_convoy.txt`.

- Gõ `4` (FCFS): Avg WT = 18.40
- Gõ `5` (SJF): Avg WT = 4.00

### Phần 3: starvation

Gõ `2`, rồi gõ `tests/demo_starvation.txt`.

- Gõ `7` rồi `1` (Priority có ngắt): dòng P1 trong bảng có WT = 15, RT = 15
- Gõ `8` rồi `2` (Round Robin): dòng P1 có WT = 4, RT = 0

### Phần 4: so sánh tất cả

Gõ `2`, rồi gõ `tests/demo_co_ban.txt`. Gõ `9`, quantum `2`.

Dưới bảng có phần `Danh gia so bo`. Kết quả phải là SRTF tốt nhất về WT (6.60) và TAT (12.20), Round Robin tốt nhất về RT (2.80).

### Phần 5: dữ liệu sai

- Gõ `2`, rồi gõ `tests/demo_loi.txt`. Chương trình báo `Loi: pid 2 bi trung.` và `giu nguyen du lieu cu`. Gõ `3`, dữ liệu cũ vẫn còn.
- Gõ `1` để nhập tay. Số process gõ `1`. Dòng process gõ `1 0 0 1` (pid 1, đến lúc 0, burst 0, priority 1). Chương trình báo lỗi burst và bắt nhập lại. Gõ `1 0 5 1`.

Gõ `0` để thoát.

## 5. Gặp lỗi thì làm gì

| Hiện tượng | Cách xử lý |
|---|---|
| `make: command not found` hoặc `gcc: command not found` | Chưa cài công cụ, làm lại bước 1 |
| `./scheduler: No such file or directory` | Chưa chạy `make`, hoặc đang đứng sai thư mục. Gõ `ls`, phải thấy `Makefile` |
| `Khong mo duoc file` khi đọc file | Gõ sai đường dẫn. Đường dẫn tính từ thư mục repo, ví dụ `tests/demo_co_ban.txt` |
| Gantt bị xuống dòng lộn xộn | Kéo cửa sổ terminal rộng ra |
| Số ra khác bảng trên | Chạy `git pull`, `make clean`, `make` rồi thử lại. Vẫn khác thì chụp màn hình gửi A |
