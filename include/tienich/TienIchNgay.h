#ifndef TIENICHNGAY_H
#define TIENICHNGAY_H

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

class TienIchNgay {
public:
  // Lấy ngày hiện tại dạng YYYY-MM-DD
  static std::string layNgayHienTaiISO() {
    std::time_t t = std::time(nullptr);
    std::tm *lt = std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return std::string(buf);
  }

  // Lấy giờ hiện tại dạng HH:MM
  static std::string layGioHienTai() {
    std::time_t t = std::time(nullptr);
    std::tm *lt = std::localtime(&t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M", lt);
    return std::string(buf);
  }

  // Phân tích ngày chuỗi YYYY-MM-DD thành std::tm
  static bool phanTichISO(const std::string &iso, std::tm &outTm) {
    if (iso.length() != 10 || iso[4] != '-' || iso[7] != '-') {
      return false;
    }
    for (size_t i = 0; i < iso.size(); ++i)
      if (i != 4 && i != 7 && (iso[i] < '0' || iso[i] > '9')) return false;
    try {
      int nam = std::stoi(iso.substr(0, 4));
      int thang = std::stoi(iso.substr(5, 2));
      int ngay = std::stoi(iso.substr(8, 2));
      if (nam < 1900 || thang < 1 || thang > 12 || ngay < 1 || ngay > 31) {
        return false;
      }
      outTm = {};
      outTm.tm_year = nam - 1900;
      outTm.tm_mon = thang - 1;
      outTm.tm_mday = ngay;
      outTm.tm_isdst = -1;
      if (std::mktime(&outTm) == -1) return false;
      return outTm.tm_year == nam - 1900 && outTm.tm_mon == thang - 1 && outTm.tm_mday == ngay;
    } catch (...) {
      return false;
    }
  }

  // Chuyển đổi định dạng DD/MM/YYYY hoặc YYYY-MM-DD sang chuẩn ISO YYYY-MM-DD
  static std::string chuanHoaISO(const std::string &ngayStr) {
    if (ngayStr.length() == 10 && ngayStr[4] == '-' && ngayStr[7] == '-') {
      return ngayStr; // Đã là chuẩn ISO
    }
    if (ngayStr.length() == 10 && ngayStr[2] == '/' && ngayStr[5] == '/') {
      // Dạng DD/MM/YYYY -> YYYY-MM-DD
      std::string ngay = ngayStr.substr(0, 2);
      std::string thang = ngayStr.substr(3, 2);
      std::string nam = ngayStr.substr(6, 4);
      return nam + "-" + thang + "-" + ngay;
    }
    return ngayStr;
  }

  // Định dạng ngày hiển thị thân thiện: YYYY-MM-DD -> DD/MM/YYYY
  static std::string dinhDangHienThi(const std::string &iso) {
    std::string s = chuanHoaISO(iso);
    if (s.length() == 10 && s[4] == '-' && s[7] == '-') {
      return s.substr(8, 2) + "/" + s.substr(5, 2) + "/" + s.substr(0, 4);
    }
    return iso;
  }

  // Cộng thêm n ngày từ ngày mốc (dùng mktime xử lý chính xác chuyển tháng/năm)
  static std::string congNgay(const std::string &iso, int soNgay) {
    std::tm t;
    if (!phanTichISO(chuanHoaISO(iso), t)) {
      return iso;
    }
    t.tm_mday += soNgay;
    std::mktime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return std::string(buf);
  }

  // Tính khoảng cách ngày (ngay2 - ngay1 theo số ngày)
  static int khoangCachNgay(const std::string &iso1, const std::string &iso2) {
    std::tm t1, t2;
    if (!phanTichISO(chuanHoaISO(iso1), t1) || !phanTichISO(chuanHoaISO(iso2), t2)) {
      return 0;
    }
    std::time_t time1 = std::mktime(&t1);
    std::time_t time2 = std::mktime(&t2);
    double diffSeconds = std::difftime(time2, time1);
    return static_cast<int>(diffSeconds / (60 * 60 * 24));
  }

  // Lấy thứ trong tuần (0: CN, 1: T2, 2: T3, 3: T4, 4: T5, 5: T6, 6: T7)
  static int layChiSoThu(const std::string &iso) {
    std::tm t;
    if (!phanTichISO(chuanHoaISO(iso), t)) {
      return 0;
    }
    return t.tm_wday;
  }

  // Lấy chuỗi thứ tiếng Việt ("T2", "T3", ..., "CN")
  static std::string layChuoiThu(const std::string &iso) {
    int wday = layChiSoThu(iso);
    switch (wday) {
    case 0:
      return "CN";
    case 1:
      return "T2";
    case 2:
      return "T3";
    case 3:
      return "T4";
    case 4:
      return "T5";
    case 5:
      return "T6";
    case 6:
      return "T7";
    default:
      return "CN";
    }
  }

  // Nhãn ngày kèm thứ: ví dụ "T6 - 30/10"
  static std::string layNhanNgayKemThu(const std::string &iso) {
    std::string s = chuanHoaISO(iso);
    std::string thu = layChuoiThu(s);
    if (s.length() == 10) {
      std::string ddmm = s.substr(8, 2) + "/" + s.substr(5, 2);
      return thu + " - " + ddmm;
    }
    return thu + " - " + s;
  }

  // Lấy ngày Thứ Hai đầu tuần của tuần chứa ngày iso (chuẩn ISO YYYY-MM-DD)
  static std::string layNgayThuHaiCuaTuan(const std::string &iso) {
    int wday = layChiSoThu(iso); // 0: CN, 1: T2, 2: T3, ..., 6: T7
    int offsetTuT2 = (wday == 0) ? 6 : (wday - 1);
    return congNgay(iso, -offsetTuT2);
  }

  // Lấy tên thứ đầy đủ: 2 -> "Thứ Hai", ..., 7 -> "Thứ Bảy", 8 -> "Chủ Nhật"
  static std::string layTenThuDayDu(int thu) {
    switch (thu) {
    case 2: return "Thứ Hai";
    case 3: return "Thứ Ba";
    case 4: return "Thứ Tư";
    case 5: return "Thứ Năm";
    case 6: return "Thứ Sáu";
    case 7: return "Thứ Bảy";
    case 8: return "Chủ Nhật";
    default: return "Thứ " + std::to_string(thu);
    }
  }

  // Sinh mã suất chiếu chuẩn ISO: SC + YYMMDD + "-" + XX (ví dụ SC261030-01)
  static std::string sinhMaSuatChieu(const std::string &iso, int caThu) {
    std::string s = chuanHoaISO(iso);
    std::string yy = (s.length() >= 4) ? s.substr(2, 2) : "26";
    std::string mm = (s.length() >= 7) ? s.substr(5, 2) : "10";
    std::string dd = (s.length() >= 10) ? s.substr(8, 2) : "01";
    char caBuf[8];
    std::snprintf(caBuf, sizeof(caBuf), "%02d", caThu);
    return "SC" + yy + mm + dd + "-" + caBuf;
  }

  // Khớp tìm kiếm thông minh (Fuzzy match) cho mã suất chiếu:
  // Cho phép nhập "SC261030-01", "261030-01", "1030-01", "30-01" hoặc chỉ "01"
  static bool khopMaSuatChieu(const std::string &maSCChuan, const std::string &inputRaw) {
    if (inputRaw.empty()) return false;
    // Chuyển cả 2 về chữ in hoa
    std::string maChuan = maSCChuan;
    for (char &c : maChuan) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    std::string in = inputRaw;
    for (char &c : in) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    if (maChuan == in) return true;

    // Khớp nếu bắt đầu bằng SC
    if (in.rfind("SC", 0) == 0 && maChuan == in) return true;

    // Khớp theo đuôi sau dấu '-' (ví dụ nhập "01" hoặc "1")
    size_t posDash = maChuan.rfind('-');
    if (posDash != std::string::npos) {
      std::string phanCa = maChuan.substr(posDash + 1);
      if (phanCa == in) return true;
      try {
        if (std::stoi(phanCa) == std::stoi(in)) return true;
      } catch (...) {}

      // Khớp theo "30-01" hoặc "1030-01"
      if (maChuan.length() >= in.length()) {
        if (maChuan.compare(maChuan.length() - in.length(), in.length(), in) == 0) {
          return true;
        }
      }
    }
    return false;
  }

  // Tính giờ kết thúc từ giờ bắt đầu (HH:MM) và thời lượng chiếu (phút)
  static std::string tinhGioKetThuc(const std::string &gioBatDau, int thoiLuongPhut) {
    if (gioBatDau.length() < 5 || thoiLuongPhut <= 0) {
      return gioBatDau;
    }
    try {
      int gio = std::stoi(gioBatDau.substr(0, 2));
      int phut = std::stoi(gioBatDau.substr(3, 2));
      int tongPhut = gio * 60 + phut + thoiLuongPhut;
      int gioKT = (tongPhut / 60) % 24;
      int phutKT = tongPhut % 60;
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%02d:%02d", gioKT, phutKT);
      return std::string(buf);
    } catch (...) {
      return gioBatDau;
    }
  }

  // Kiểm tra định dạng giờ chuẩn HH:MM (00:00 đến 23:59)
  static bool kiemTraDinhDangGio(const std::string &gioStr) {
    if (gioStr.length() != 5 || gioStr[2] != ':') return false;
    for (int i = 0; i < 5; i++) {
      if (i == 2) continue;
      if (!std::isdigit(static_cast<unsigned char>(gioStr[i]))) return false;
    }
    int g = std::stoi(gioStr.substr(0, 2));
    int p = std::stoi(gioStr.substr(3, 2));
    return (g >= 0 && g <= 23 && p >= 0 && p <= 59);
  }

  // Chuyển chuỗi HH:MM thành số phút tính từ đầu ngày
  static int chuyenGioThanhPhut(const std::string &gioStr) {
    if (!kiemTraDinhDangGio(gioStr)) return -1;
    int g = std::stoi(gioStr.substr(0, 2));
    int p = std::stoi(gioStr.substr(3, 2));
    return g * 60 + p;
  }

  // Kiểm tra hai khoảng thời gian trong ngày có bị chồng lấn (chồng giờ) không
  static bool kiemTraChongLanGio(const std::string &bd1, const std::string &kt1,
                                 const std::string &bd2, const std::string &kt2) {
    int s1 = chuyenGioThanhPhut(bd1);
    int e1 = chuyenGioThanhPhut(kt1);
    int s2 = chuyenGioThanhPhut(bd2);
    int e2 = chuyenGioThanhPhut(kt2);

    if (s1 < 0 || e1 < 0 || s2 < 0 || e2 < 0) return false;
    if (s1 >= e1 || s2 >= e2) return false;

    return (std::max(s1, s2) < std::min(e1, e2));
  }
};

#endif // TIENICHNGAY_H
