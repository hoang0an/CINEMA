# Quản lý rạp chiếu phim — PBL2

**Nhóm 13:** Thái Nhật Hoàng Ân – Phan Nhật Hoàng.

Ứng dụng C++17 chạy trên Windows Console, phục vụ học tập lập trình hướng đối tượng và cấu trúc dữ liệu. Các lệnh bên dưới chạy tại thư mục chứa README này.

## Chức năng

- Đăng nhập, phân quyền quản trị viên và thu ngân.
- Thêm, sửa, xóa, tìm kiếm và sắp xếp phim; quản lý ngày khởi chiếu, ngày hết đợt và trạng thái.
- Tự sinh lịch cho 5 phòng trong 7 ngày tính cả hôm nay; bổ sung khung trống phù hợp, giữ lịch/ghế đã có và kiểm tra chồng thời lượng.
- Xem lịch theo ngày, sơ đồ ghế, bán nhiều vé trong một hóa đơn và chặn ghế trùng/đã bán.
- Quản lý nhân viên, ca trực, lịch tuần mẫu, tiền lương và báo cáo doanh thu theo kỳ.
- Lưu dữ liệu bằng file văn bản UTF-8.

## Yêu cầu và cách chạy

- Windows và terminal hỗ trợ UTF-8; khuyến nghị Windows Terminal.
- MinGW-w64 g++ hỗ trợ C++17. Dự án đã được kiểm tra với GCC 15.2.0 trên MSYS2 UCRT64.
- Giao diện dùng Win32 API nên không chạy trực tiếp trên Linux/macOS.

Mở PowerShell tại thư mục repository:

```powershell
g++ --version
g++ -std=c++17 -Wall -Wextra -Wpedantic -static -I include source/main.cpp -o rapchieuphim.exe
if ($LASTEXITCODE -eq 0) { .\rapchieuphim.exe }
```

Ứng dụng đọc/ghi `dulieu/` theo thư mục làm việc hiện tại. Hãy chạy từ thư mục repository, không chỉ sao chép EXE sang nơi khác. EXE được build lại từ source, không cần đưa vào Git.

## Tài khoản demo

| Vai trò | Tài khoản | Mật khẩu |
|---|---|---|
| Quản trị viên | admin | admin123 |
| Thu ngân | nhanvien1 | 123456 |
| Thu ngân | nhanvien2 | 123456 |
| Thu ngân | nhanvien3 | 123456 |

Thông tin nhân viên là dữ liệu giả. Mật khẩu lưu dạng văn bản, chỉ phục vụ demo offline; không dùng tài khoản/mật khẩu thật.

## Quy tắc nghiệp vụ hiện hành

- **E — Sửa phim:** Enter giữ nguyên từng trường. Ngày nhận `YYYY-MM-DD` hoặc `DD/MM/YYYY`. Không cho rút ngắn ngày hết đợt qua các suất đã xếp. Ngày cuối vẫn thuộc đợt chiếu.
- **S — Trạng thái:** `1: Sắp chiếu`, `0: Ngừng chiếu`. Mở lại phim cần ngày khởi chiếu tương lai; không bật Đang chiếu thủ công. Trạng thái đồng bộ theo ngày khi nạp dữ liệu.
- Quota danh mục là 5 Đang chiếu và 5 Sắp chiếu. Khi đồng bộ ngày làm vượt quota Đang chiếu, ứng dụng cảnh báo và chặn thêm theo giới hạn; không tự ngừng phim để ép đủ quota.
- Lịch tự sinh tối đa 7 ngày; lịch cũ ngoài phạm vi vẫn giữ nguyên. Ngày khởi chiếu không bảo đảm có suất nếu lịch kín hoặc chưa thuộc phạm vi sinh lịch.
- Không bán vé từ giờ bắt đầu suất chiếu. Mã suất rút gọn được tìm theo ngày đang chọn.
- Khi có file `.blocked`, dừng bán và đối soát dữ liệu với `.bak`; không tự xóa marker để tiếp tục ghi khi chưa xác nhận dữ liệu nhất quán.

## Dữ liệu mẫu

Tại ngày **03/11/2026**, có 130 phim: 5 Đang chiếu, 5 Sắp chiếu và 120 Ngừng chiếu. Trạng thái sẽ thay đổi theo ngày hệ thống; các ngày trong dữ liệu là ngày demo.
## Cấu trúc repository

```text
include/
  cautruc/      # Cấu trúc dữ liệu tự viết
  models/       # Phim, phòng, ghế, vé, hóa đơn, người dùng...
  dichvu/       # Nghiệp vụ và báo cáo
  luutru/       # Đọc/ghi dữ liệu
  giaodien/     # Windows Console UI
  tienich/      # Xử lý ngày và thời gian
source/
  main.cpp
tests/          # Mã nguồn kiểm thử C++
dulieu/         # Dữ liệu mẫu UTF-8
```

Ứng dụng vận dụng đóng gói, kế thừa, đa hình, trừu tượng, template và nạp chồng toán tử. Danh sách liên kết và ma trận được dùng trong nghiệp vụ;

## Kiểm thử

Giữ file `.cpp` trong `tests/`; không cần commit EXE hoặc log. Một số test cũ ghi vào `dulieu/` và phụ thuộc ngày/dữ liệu cụ thể: **không chạy toàn bộ test trực tiếp trên dữ liệu đang dùng**.

Ví dụ kiểm tra quota bằng fixture tạo trong bộ nhớ:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Wpedantic -I include tests/test_quota_limits.cpp -o build/test_quota_limits.exe
if ($LASTEXITCODE -eq 0) { .\build\test_quota_limits.exe }
```

Không thêm `-DNDEBUG` khi chạy test dùng `assert`. Test có đọc/ghi file cần chạy trong thư mục làm việc riêng với bản sao dữ liệu. Runner và báo cáo ngoài thư mục dự án của môi trường phát triển trước đây không nằm trong repository này.

## Trạng thái và giới hạn

Các lỗi trọng yếu về xóa suất, chọn sai ngày, ghế trùng và xử lý lỗi lưu đã được sửa, cùng các cải tiến quản trị phim. Chưa nghiệm thu đầy đủ trên Windows sạch hoặc kiểm tra memory leak bằng sanitizer; không tuyên bố production-ready. Giao dịch hai file chưa bảo đảm nguyên tử xuyên mất điện/crash. Nhãn “lợi nhuận ròng” hiện chỉ tính doanh thu vé trừ tiền lương.

Tham khảo [Danh sách thuật toán](DANH_SACH_THUAT_TOAN.md), [Lịch sử cải tiến](LICH_SU_CAI_TIEN.md) và [Bối cảnh dự án](PROJECT-CONTEXT.md). Tài liệu lịch sử có thể nhắc tới bằng chứng và bản build cục bộ không được đưa vào repository; dùng hướng dẫn build trong README này cho source hiện tại.
