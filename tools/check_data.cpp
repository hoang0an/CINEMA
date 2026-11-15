#include "dichvu/DichVuDatVe.h"
#include <filesystem>

// Chỉ đọc dữ liệu. Không gọi khởi tạo ứng dụng vì khởi tạo có thể bổ sung lịch.
int main() {
  SetConsoleOutputCP(CP_UTF8);
  DichVuDatVe dv;
  if (!QuanLyFile::docDanhSachSuatChieu("dulieu/suatchieu.txt", dv.layDanhSachSuatChieu()) ||
      !dv.docDanhSachHoaDon("dulieu/hoadon.txt")) {
    std::cerr << "Không đọc được đầy đủ dữ liệu. Không tự sửa hoặc ghi đè.\n";
    return 1;
  }
  const auto loi = dv.kiemTraDoiSoat();
  for (const auto &dong : loi) std::cout << "Cần đối soát lịch/ghế: " << dong << "\n";
  // Tên phim trên vé là ảnh chụp tại thời điểm bán: báo để đối soát, không tự ghi đè.
  std::ifstream file("dulieu/hoadon.txt"); std::string dong;
  int tenLech = 0;
  while (std::getline(file, dong)) {
    std::stringstream ss(dong); std::vector<std::string> t; std::string token;
    while (std::getline(ss, token, '|')) t.push_back(token);
    if (t.size() < 11) continue;
    for (auto n = dv.layDanhSachSuatChieu().layDau(); n; n = n->tiepTheo)
      if (n->duLieu->layMaSuatChieu() == t[4] && n->duLieu->layTenPhim() != t[5]) {
        std::cout << "Tên phim khác lịch hiện tại: " << t[3] << " / " << t[4] << "\n";
        ++tenLech; break;
      }
  }
  std::cout << loi.size() << " vé cần đối soát lịch/ghế; " << tenLech << " vé khác tên phim.\n";
  std::cout << "Không thay đổi dữ liệu. Hóa đơn lịch sử được giữ nguyên.\n";
  return 0; // Dữ liệu đọc được; các sai lệch lịch sử được liệt kê để xử lý thủ công.
}
