# Quản lý rạp chiếu phim — PBL2

**Nhóm 13:** Thái Nhật Hoàng Ân – Phan Nhật Hoàng.

Ứng dụng C++17 chạy trên Windows Console, phục vụ học tập lập trình hướng đối tượng và cấu trúc dữ liệu.

## Build và chạy

Repository chỉ cần nội dung của **`Rap chieu phim`**. Không cần báo cáo, `artifacts` hoặc công cụ ở thư mục cha.

Yêu cầu: Windows, PowerShell, terminal hỗ trợ UTF-8 và MinGW-w64 có `g++` hỗ trợ C++17. Máy kiểm thử sử dụng GCC 15.2.0. Không cần Python, CMake hay thư viện bên ngoài.

Giải nén repository nếu tải ZIP. Mở PowerShell **tại thư mục có README này, `include`, `source` và `dulieu`**, rồi chạy:

```powershell
g++ --version
g++ -std=c++17 -Wall -Wextra -Wpedantic -static -I include source/main.cpp -o rapchieuphim.exe
if ($LASTEXITCODE -eq 0) { .\rapchieuphim.exe }
```

Nếu không tìm thấy `g++`, thêm thư mục `bin` của MinGW-w64 vào `PATH`, mở lại PowerShell và kiểm tra `g++ --version`.

Ứng dụng dùng Win32 API nên không build trực tiếp trên Linux/macOS. Đường dẫn `dulieu/` tính theo thư mục làm việc hiện tại: luôn chạy EXE từ thư mục dự án như trên. Chỉ chạy một bản ứng dụng trên một bộ dữ liệu tại một thời điểm.

## Tài khoản demo

| Vai trò | Tài khoản | Mật khẩu |
|---|---|---|
| Quản trị viên | admin | admin123 |
| Thu ngân | nhanvien1 | 123456 |
| Thu ngân | nhanvien2 | 123456 |
| Thu ngân | nhanvien3 | 123456 |

Dữ liệu nhân viên là giả. Mật khẩu lưu văn bản cho demo offline; không sử dụng mật khẩu thật.

## Chức năng và quy tắc

- Quản lý phim: thêm, sửa, xóa, tìm kiếm và sắp xếp theo thời lượng. Khi thêm, chọn tự động xác định trạng thái theo ngày hoặc ngừng chiếu; `Q` hủy.
- Ngày nhận `YYYY-MM-DD` hoặc `DD/MM/YYYY`. Số ngày chiếu tính cả ngày đầu; một ngày chiếu có ngày kết thúc trùng ngày khởi chiếu. Đọc dữ liệu phim cũ vẫn giữ nguyên ngày kết thúc cũ.
- Sửa phim: Enter giữ nguyên trường; không thay thời lượng khi còn suất tương lai hoặc rút ngắn đợt qua suất đã xếp. Mở lại phim qua menu trạng thái cần ngày khởi chiếu tương lai.
- Quota khi thêm/đổi trạng thái: 5 phim đang chiếu và 5 phim sắp chiếu. Nếu chuyển ngày làm vượt quota thì cảnh báo, không tự ngừng phim.
- Tự bổ sung lịch cho 5 phòng trong 7 ngày, gồm hôm nay; giữ suất đã có và kiểm tra chồng thời lượng. VIP có suất chiếu sớm. Phim có thể chưa có suất nếu lịch kín.
- Bán nhiều vé trong một hóa đơn; chặn ghế trùng/đã bán và suất đã bắt đầu. Hóa đơn mới lưu mã nhân viên, mã ca nếu có ca đang làm và giờ lập. Bán ngoài ca vẫn ghi doanh số ngày, không tự gán vào ca khác.
- Báo cáo thu ngân là **doanh số hôm nay theo mã nhân viên**, chỉ đọc dữ liệu. Hóa đơn cũ thiếu mã nhân viên vẫn nằm trong báo cáo tổng/kỳ; không được đoán vào báo cáo cá nhân.
- Quản lý ca, làm thay, lịch tuần mẫu và quyết toán lương. Không xóa nhân viên còn tham chiếu ca/lịch mẫu/hóa đơn hoặc ca được hóa đơn mới tham chiếu.
- Sửa lịch mẫu đồng bộ đúng ca tương ứng tuần sau, giữ người làm thay đã có. Xung đột hoặc lỗi lưu được báo rõ, không thông báo thành công vô điều kiện.

## Kiểm thử

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run.ps1
```

Runner build C++ rồi chạy test trong thư mục tạm riêng; không sửa `dulieu` demo, không tải thêm test framework. Test bao phủ doanh số, ngày chiếu, dữ liệu cũ, giá vé sai, lỗi lưu và khôi phục bộ nhớ, lịch mẫu, ghế trùng, giao dịch thất bại và khóa ghi. Đây là kiểm thử nghiệp vụ, không thay thế nghiệm thu toàn bộ giao diện.

## Đối soát dữ liệu

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Wpedantic -static -I include tools/check_data.cpp -o build/check_data.exe
if ($LASTEXITCODE -eq 0) { .\build\check_data.exe }
```

Công cụ chỉ đọc, liệt kê sai lệch hóa đơn/lịch/ghế và khác biệt tên phim. Mã thoát `1`: không đọc được đầy đủ; `0`: đã kiểm tra xong, **không có nghĩa mọi dòng đều khớp**. Xem cảnh báo được in ra.

Dữ liệu demo có ngày cố định trong năm 2026; trạng thái và lịch thay đổi theo ngày máy. Ngày 12/11/2026 đã đối soát đủ 31 vé: khôi phục lịch và ghế theo hóa đơn, giữ nguyên nội dung hóa đơn và doanh thu. Công cụ đối soát báo **0 vé lệch lịch/ghế, 0 vé khác tên phim**. Xem [ghi chú sửa lỗi](docs/SUA_LOI_20261112.md).

Khởi động ứng dụng sẽ cảnh báo dữ liệu lệch. Sai lệch ngày hiện tại/tương lai khóa bán vé; sai lệch chỉ thuộc lịch sử được giữ để tra cứu. Không tự sửa hóa đơn nhằm làm mất cảnh báo.

## Lưu trữ và khôi phục

File UTF-8 trong `dulieu`, phân tách trường bằng `|`:

| File | Nội dung |
|---|---|
| `phim.txt` | Phim; bản mới thêm cột `days-v2` nhận diện cách tính ngày |
| `suatchieu.txt` | Suất, phòng, trạng thái ghế (`0`: trống, `1`: đã đặt) |
| `hoadon.txt` | Mỗi dòng một vé; 11 cột cũ hoặc 14 cột mới, thêm mã nhân viên, mã ca, giờ lập |
| `nguoidung.txt` | Tài khoản và nhân viên |
| `catruc.txt` | Ca, doanh thu, trạng thái lương |
| `catruc_mau.txt` | Lịch mẫu theo thứ |

Trước khi chỉnh dữ liệu/nâng phiên bản, đóng ứng dụng và sao lưu cả `dulieu`. Không dùng EXE cũ để ghi lại dữ liệu đã chuyển định dạng mới.

Ghi một file dùng `.writing` rồi thay file đích sau khi ghi thành công. Giao dịch vé dùng `.tmp`, `.bak`, `.blocked` cho lịch và hóa đơn. Nếu có `.blocked`, dừng bán, sao lưu hiện trạng, đối chiếu cả hai file với `.bak`; không xóa marker để tiếp tục bán khi chưa xác nhận dữ liệu nhất quán. Không cam kết giao dịch nguyên tử qua mất điện/crash giữa hai lần thay file.

Doanh thu ca đối soát từ hóa đơn mới có mã ca. Nếu vé đã lưu nhưng cập nhật ca thất bại, ứng dụng báo và đối soát lại khi mở; không thực hiện lại giao dịch bán vé đó.

## Cấu trúc và GitHub

```text
include/
  cautruc/     # Danh sách liên kết, ma trận
  models/      # Phim, suất, ghế, vé, hóa đơn, người dùng, ca
  dichvu/      # Nghiệp vụ
  luutru/      # Đọc dữ liệu, ghi file an toàn
  giaodien/    # Windows Console
  tienich/     # Ngày và thời gian
source/main.cpp
tests/         # Test C++ và runner PowerShell
tools/         # Đối soát chỉ đọc
docs/          # Ghi chú sửa lỗi
dulieu/        # Dữ liệu demo
```

Lấy nội dung thư mục này làm gốc repository GitHub. Giữ source, test, công cụ và dữ liệu demo; `.gitignore` loại EXE, build, file tạm và bản sao lưu. Người tải về build lại EXE bằng lệnh đầu README.
