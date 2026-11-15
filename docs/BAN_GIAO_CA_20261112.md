# Bàn giao ca — tạm dừng theo yêu cầu người dùng

> **Cập nhật sau khi người dùng yêu cầu tiếp tục:** ca đã được tiếp tục, ba nhóm kiểm tra bổ sung bên dưới đã sửa và thêm test. Gói độc lập tại `artifacts/standalone-final-20261112` đã build thành công, 28/28 test đạt. Xem `SUA_LOI_20261112.md` cho kết quả hiện tại. Nội dung bên dưới giữ làm lịch sử bàn giao, không còn là trạng thái tạm dừng.

## Yêu cầu và phạm vi

Người dùng yêu cầu rà soát, sau đó sửa lỗi và viết lại README vì chỉ đẩy nội dung `Rap chieu phim` lên GitHub. Ngày 12/11/2026, người dùng yêu cầu **tạm dừng và bàn giao**. Không tiếp tục sửa/kiểm thử cho đến khi người dùng yêu cầu tiếp tục.

Workspace: `D:\code\project\pbl2`. Repository Git thực tế: `D:\code\project\pbl2\Rap chieu phim`.

Không commit/push. Không dùng subagent trong ca này. Các skill đã đọc và áp dụng: `ecc:cpp-testing`, các nguyên tắc liên quan của `ecc:error-handling`. Đã dùng test C++ độc lập, không thêm GoogleTest hay dependency bên ngoài.

## Lựa chọn đã được người dùng xác nhận

Về dữ liệu lịch sử: **“Giữ lịch sử, thêm kiểm tra và cảnh báo; chỉ sửa các ghế khớp đủ thông tin.”** Không tạo lại dữ liệu demo và không tự suy đoán bên nào đúng khi lịch/hóa đơn mâu thuẫn.

## Đã thực hiện

1. Viết lại README: toàn bộ lệnh build/chạy/test/đối soát chạy tại gốc repository này; không phụ thuộc thư mục cha.
2. Thêm `include/luutru/GhiFileAnToan.h`: ghi `.writing`, đóng/kiểm tra stream rồi thay file bằng Win32 `MoveFileExW`.
3. Sửa báo cáo thu ngân thành doanh số hôm nay theo mã nhân viên, chỉ đọc; bỏ cộng doanh thu mỗi lần xem.
4. Hóa đơn mới có 14 cột: 11 cột cũ + mã nhân viên + mã ca + giờ lập. Vẫn đọc hóa đơn cũ, không đoán định danh/ca. Đối soát doanh thu ca bằng tổng hóa đơn có đúng mã ca.
5. Sửa lỗi ghi ca/lương/nhân viên: phục hồi bộ nhớ và báo lỗi; UI không báo thành công vô điều kiện. Sửa một số đường lưu phim/lịch thành thông báo rõ trạng thái một phần, khóa bán khi lưu lịch thất bại.
6. Chặn xóa nhân viên có tham chiếu ca/lịch mẫu/hóa đơn và ca có hóa đơn mới tham chiếu.
7. Sửa đồng bộ ca mẫu: dùng đúng ca đích, trả lỗi xung đột/ghi thất bại; bỏ suy luận cả tuần từ thứ Hai.
8. Parser hóa đơn từ chối số âm/NaN/Inf/chuỗi dư, ngày sai, vé trùng và header cùng hóa đơn không nhất quán. Đọc hóa đơn lỗi khóa ghi, không nạp một phần rồi ghi đè.
9. Kiểm tra bitmap ghế; nạp/đối soát trước bổ sung và ghi lịch. Sai lệch hiện tại/tương lai khóa bán vé; sai lệch lịch sử cảnh báo.
10. Số ngày phim tính cả ngày đầu. Phim file cũ đọc thêm một ngày để giữ nguyên ngày kết thúc; ghi marker `days-v2` để không chuyển đổi lặp. Đầu vào thêm phim không âm thầm mặc định khi sai; trạng thái chuẩn hóa trước quota.
11. Bỏ một số API nghiệp vụ chưa dùng, giữ thao tác tổng quát của cấu trúc dữ liệu phục vụ môn học. Không tự sinh dữ liệu doanh thu/lương giả khi file ca rỗng. Đọc ca/lịch mẫu lỗi thì khóa ghi các file đó.
12. Thêm `tests/regression.cpp`, `tests/run.ps1`, `tools/check_data.cpp`, `docs/SUA_LOI_20261112.md`; thêm `*.writing` vào `.gitignore`.

## Dữ liệu đã sửa và đã giữ

- Chỉ đánh dấu ghế đã đặt cho VE008–VE019: **12 ghế** có mã suất, tên phim, phòng, ngày, giờ và tọa độ đều khớp.
- VE001–VE007: giữ nguyên vì thiếu suất trong lịch.
- VE020–VE031: giữ nguyên vì khác thông tin phim/phòng.
- Còn **19 vé lịch sử chưa đối soát xong**, không báo là đã sửa hết.
- `hoadon.txt` không đổi. SHA-256: `c49550bac9621b16cf6ca8e013e476874eecfe79297c3db4ec1b74ed7519e279`.
- Trước ca sửa, Git đã có thay đổi ở README và `dulieu/suatchieu.txt`; không reset về HEAD vì sẽ mất thay đổi có trước.

## Đã xác minh

- Build ứng dụng bằng `g++ -std=c++17 -Wall -Wextra -Wpedantic -static -I include source/main.cpp -o rapchieuphim.exe`: thành công, không cảnh báo compiler. EXE tại gốc repo đã build lại.
- `powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run.ps1`: **25 passed; 0 failed**. Test tạo và dọn thư mục tạm riêng, không ghi dữ liệu demo.
- Test gồm dữ liệu cũ, ngày chiếu, mã nhân viên/ngày, báo cáo lặp, giá sai, hóa đơn trùng, rollback ca/lương/lịch mẫu, writer lỗi, ghế trùng, rollback giao dịch, sai lệch tương lai khóa bán, marker `.blocked`, parser ca/mẫu/bitmap và rollback hủy suất.
- Build `tools/check_data.cpp` thành `build/check_data.exe`: thành công. Chạy trên dữ liệu demo đã sửa: **19 vé lệch lịch/ghế, 12 vé khác tên phim** (12 vé là tập con của 19, không cộng thành 31).
- `git diff --check`: không có lỗi whitespace; chỉ thông báo chuyển LF/CRLF.
- README đã đọc lại, tiếng Việt UTF-8 hiển thị đúng.
- Hóa đơn đã kiểm tra hash như trên.

## Đang dở lúc tạm dừng

Đã tạo bản sao độc lập chỉ chứa nội dung repo tại:

`D:\code\project\pbl2\artifacts\standalone-20261112`

Đã copy `include`, `source`, `tests`, `tools`, `docs`, `dulieu`, README và `.gitignore`. **Chưa chạy build/test trong bản sao này.** File bàn giao hiện tại được tạo sau lần copy nên chưa nằm trong bản sao. Không kết luận đã hoàn tất kiểm chứng gói độc lập.

Không có tiến trình build/test đang chạy cần đợi tại thời điểm bàn giao. Không mở/chạy ứng dụng console tương tác trên dữ liệu gốc.

## Ca sau cần làm khi người dùng cho tiếp tục

1. Đọc bàn giao, `docs/SUA_LOI_20261112.md`, README và kiểm tra `git status/diff` hiện tại; không chạy lại các script sửa một lần.
2. Rà soát bổ sung tính đúng của thay đổi, đặc biệt các luồng lưu nhiều file và khởi động. Các điểm cần kiểm tra, chưa khẳng định là lỗi đã tái hiện:
   - Nếu đã lưu trạng thái phim ngừng nhưng hủy lịch thất bại, sau khởi động lại còn suất tương lai chưa bán không? `hoanTatKhoiTao` hiện đồng bộ tên/sinh lịch/lưu, chưa chủ động chạy lại hủy suất của phim ngừng.
   - Khi ca/lịch mẫu đọc lỗi, dữ liệu trong bộ nhớ có thể rỗng. Cần xem guard xóa nhân viên có phải từ chối thêm khi phụ thuộc chưa nạp hợp lệ, tránh bỏ sót tham chiếu chỉ còn trong file lỗi.
   - Parser hóa đơn kiểm tra vé theo phần số nhưng nhóm hóa đơn theo chuỗi. Xem trường hợp hai mã hóa đơn khác zero-padding nhưng cùng phần số; thêm test nếu sửa.
   - Kiểm tra callback đồng bộ tên phim sau lưu phim và luồng cập nhật trạng thái để đảm bảo UI không che giấu trạng thái một phần.
3. Build/chạy test đúng README trong bản sao độc lập. Nếu tiếp tục sửa source, cập nhật bản sao hoặc tạo bản sao mới trước kiểm tra cuối.
4. Nếu cần, nghiệm thu UI trên bản sao dữ liệu, không dùng kết quả gọi hàm thay cho bằng chứng thao tác UI.
5. Cập nhật ghi chú kết quả, báo cáo rõ phần hoàn thành và 19 vé lịch sử còn giữ theo quyết định của người dùng. Không tự commit/push.

## Bản sao và bằng chứng ngoài repo

- Sao lưu trước sửa: `D:\code\project\pbl2\artifacts\before-fixes-20261112.zip` (include/source/dulieu/README/.gitignore).
- Đối soát ghế: `artifacts/audit_20261112/reconciliation.json`.
- Script đã chạy một lần: `artifacts/fix_pbl2.py`, `fix_booking.py`, `fix_ui.py`, `reconcile_seats.py`. **Không chạy lại**: dùng thay chuỗi với giả định source cũ; script đối soát chạy lại cũng ghi lại báo cáo danh sách ghế đã sửa.
- Báo cáo rà soát ban đầu `artifacts/audit_20261112/BAO_CAO_RASOAT.md` bị mất dấu do encoding lúc ghi qua PowerShell pipe. Dùng nội dung cuộc trò chuyện và tài liệu UTF-8 trong repo hiện tại làm tham chiếu, không sao chép lỗi encoding sang README.

## Giới hạn

Chưa nghiệm thu toàn bộ UI, crash/mất điện, nhiều tiến trình đồng thời. Dịch vụ vẫn còn phần hiển thị console; chưa refactor toàn bộ kiến trúc (F12 của rà soát). Không tuyên bố mọi lỗi đã được sửa hoặc gói GitHub đã kiểm chứng xong.
