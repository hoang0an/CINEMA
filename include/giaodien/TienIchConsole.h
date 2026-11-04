#ifndef TIENICHCONSOLE_H
#define TIENICHCONSOLE_H

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>
#include <conio.h>

// Lop tien ich ve giao dien Console voi box-drawing Unicode
// Xu ly chuan UTF-8, tu dong cat chuoi chong tran khung tuyet doi
class TienIchConsole {
public:
  static const int DO_RONG = 60;

  // === HẰNG SỐ MÀU SẮC CONSOLE (WIN32 API) ===
  enum MauSac {
    MAU_MAC_DINH    = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,                          // 0x07 Trắng mặc định
    MAU_TRANG_SANG  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,   // 0x0F Trắng sáng
    MAU_MA_PHIM     = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,                    // 0x0B Xanh lam sáng (Cyan) cho mã phim [260001]
    MAU_DANG_CHIEU  = FOREGROUND_GREEN | FOREGROUND_INTENSITY,                                      // 0x0A Xanh lá sáng (Green) cho [Đang chiếu]
    MAU_SAP_CHIEU   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,                     // 0x0E Vàng sáng (Yellow) cho [Sắp chiếu]
    MAU_NGUNG_CHIEU = FOREGROUND_RED | FOREGROUND_INTENSITY,                                      // 0x0C Đỏ sáng (Red) cho [Ngừng chiếu]
    MAU_DO          = FOREGROUND_RED | FOREGROUND_INTENSITY,                                      // 0x0C Đỏ sáng (Red) thông báo lỗi
    MAU_XAM         = FOREGROUND_INTENSITY                                                          // 0x08 Xám (Dark Gray)
  };

  // Cấu trúc một đoạn văn bản có màu sắc riêng biệt
  struct DoanMau {
    std::string text;
    WORD mau;
    DoanMau(const std::string &t = "", WORD m = MAU_MAC_DINH) : text(t), mau(m) {}
  };

  // Thiết lập màu chữ hiện tại của console
  static void datMau(WORD mau) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), mau);
  }

  // Khôi phục màu chữ mặc định của console
  static void resetMau() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), MAU_MAC_DINH);
  }

  // Lấy mã màu tương ứng với trạng thái phim (1: Đang chiếu, 2: Sắp chiếu, 0: Ngừng chiếu)
  static WORD layMauTheoTrangThai(int trangThai) {
    if (trangThai == 1) return MAU_DANG_CHIEU;
    if (trangThai == 2) return MAU_SAP_CHIEU;
    return MAU_NGUNG_CHIEU;
  }

  // In một chuỗi văn bản với màu chỉ định và tự động reset về màu mặc định
  static void inChuCoMau(const std::string &s, WORD mau) {
    datMau(mau);
    std::cout << s;
    resetMau();
  }

  // Đọc một phím đơn tức thì (Zero-Enter), tự động lọc bỏ các mã phím mở rộng 2-byte (mũi tên, F1-F12...)
  static int docPhimDon() {
    int ch = _getch();
    if (ch == 0 || ch == 224) {
      _getch(); // Bỏ qua byte thứ hai của phím mở rộng
      return -1;
    }
    return ch;
  }

  // Xoa toan bo man hinh console
  static void xoaManHinh() { system("cls"); }

  // Định dạng số tiền có dấu chấm phân cách hàng nghìn (ví dụ: 1440000 -> "1.440.000")
  static std::string dinhDangTien(long long soTien) {
    if (soTien == 0) return "0";
    bool am = (soTien < 0);
    if (am) soTien = -soTien;

    std::string s = std::to_string(soTien);
    int n = (int)s.length();
    std::string ketQua = "";
    int dem = 0;
    for (int i = n - 1; i >= 0; i--) {
      ketQua = s[i] + ketQua;
      dem++;
      if (dem % 3 == 0 && i > 0) {
        ketQua = "." + ketQua;
      }
    }
    if (am) ketQua = "-" + ketQua;
    return ketQua;
  }

  static std::string dinhDangTien(int soTien) {
    return dinhDangTien((long long)soTien);
  }

  static std::string dinhDangTien(double soTien) {
    return dinhDangTien((long long)soTien);
  }

  // Tinh so luong ky tu hien thi thuc te (visual columns) theo chuan UTF-8
  static int doDaiHienThi(const char *s) {
    int len = 0;
    while (*s) {
      unsigned char c = (unsigned char)*s;
      if ((c & 0x80) == 0) s += 1;
      else if ((c & 0xE0) == 0xC0) s += 2;
      else if ((c & 0xF0) == 0xE0) s += 3;
      else if ((c & 0xF8) == 0xF0) s += 4;
      else s += 1;
      len++;
    }
    return len;
  }

  static int doDaiHienThi(const std::string &s) {
    return doDaiHienThi(s.c_str());
  }

  // Cat chuoi an toan theo so ky tu hien thi (khong lam hong multi-byte UTF-8)
  static std::string catChuoi(const std::string &s, int maxVisual) {
    int vLen = 0;
    size_t byteIdx = 0;
    while (byteIdx < s.length() && vLen < maxVisual) {
      unsigned char c = (unsigned char)s[byteIdx];
      size_t charBytes = 1;
      if ((c & 0x80) == 0) charBytes = 1;
      else if ((c & 0xE0) == 0xC0) charBytes = 2;
      else if ((c & 0xF0) == 0xE0) charBytes = 3;
      else if ((c & 0xF8) == 0xF0) charBytes = 4;

      if (byteIdx + charBytes > s.length()) break;
      byteIdx += charBytes;
      vLen++;
    }
    return s.substr(0, byteIdx);
  }

  // Cat chuoi an toan theo so ky tu hien thi va luon cat tai ranh gioi tu (khong chet dut ngang mot tu)
  static std::string catChuoiTheoTu(const std::string &s, int maxVisual) {
    if (doDaiHienThi(s) <= maxVisual) return s;
    std::string cut = catChuoi(s, maxVisual - 3);
    size_t lastSpace = cut.find_last_of(" ");
    if (lastSpace != std::string::npos && lastSpace > (size_t)(maxVisual / 3)) {
      return cut.substr(0, lastSpace) + "...";
    }
    return cut + "...";
  }

  // Tách văn bản thành các dòng vừa vặn với độ rộng hiển thị tối đa, ngắt theo từ để không bao giờ mất chữ
  static std::vector<std::string> tachDongTuDong(const std::string &text, int maxVisual) {
    std::vector<std::string> ketQua;
    if (text.empty() || maxVisual <= 0) return ketQua;

    std::vector<std::string> tuList;
    std::string tuHienTai = "";
    for (size_t i = 0; i < text.length(); i++) {
      if (text[i] == ' ' || text[i] == '\t' || text[i] == '\n') {
        if (!tuHienTai.empty()) {
          tuList.push_back(tuHienTai);
          tuHienTai.clear();
        }
      } else {
        tuHienTai += text[i];
      }
    }
    if (!tuHienTai.empty()) {
      tuList.push_back(tuHienTai);
    }

    std::string dongHienTai = "";
    int visualHienTai = 0;

    for (const auto &w : tuList) {
      int wLen = doDaiHienThi(w);
      if (dongHienTai.empty()) {
        if (wLen <= maxVisual) {
          dongHienTai = w;
          visualHienTai = wLen;
        } else {
          std::string wCon = w;
          while (doDaiHienThi(wCon) > maxVisual) {
            std::string doanCat = catChuoi(wCon, maxVisual);
            ketQua.push_back(doanCat);
            wCon = wCon.substr(doanCat.length());
          }
          dongHienTai = wCon;
          visualHienTai = doDaiHienThi(wCon);
        }
      } else {
        if (visualHienTai + 1 + wLen <= maxVisual) {
          dongHienTai += " " + w;
          visualHienTai += 1 + wLen;
        } else {
          ketQua.push_back(dongHienTai);
          if (wLen <= maxVisual) {
            dongHienTai = w;
            visualHienTai = wLen;
          } else {
            std::string wCon = w;
            while (doDaiHienThi(wCon) > maxVisual) {
              std::string doanCat = catChuoi(wCon, maxVisual);
              ketQua.push_back(doanCat);
              wCon = wCon.substr(doanCat.length());
            }
            dongHienTai = wCon;
            visualHienTai = doDaiHienThi(wCon);
          }
        }
      }
    }

    if (!dongHienTai.empty()) {
      ketQua.push_back(dongHienTai);
    }

    return ketQua;
  }

  static std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
  }

  // Phân tích mã ghế: "B3", "b3", "A1", "H6" -> hang, cot (0-indexed)
  // Trả về:
  //   1: Hợp lệ
  //   0: Hủy bỏ (người dùng nhập 0, q, Q, huy, exit)
  //  -1: Không hợp lệ
  static int phanTichMaGhe(const std::string &inputRaw, int &hang, int &cot, int maxHang, int maxCot) {
    std::string s = trim(inputRaw);
    if (s.empty()) return -1;

    for (char &c : s) {
      c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    if (s == "0" || s == "Q" || s == "HUY" || s == "EXIT" || s == "CANCEL") {
      return 0; // Hủy bỏ
    }

    std::string clean = "";
    for (char c : s) {
      if (c != ' ') clean += c;
    }

    if (clean.length() >= 2 && clean[0] >= 'A' && clean[0] <= 'Z') {
      int h = clean[0] - 'A';
      std::string phanSo = clean.substr(1);
      try {
        int c = std::stoi(phanSo) - 1;
        if (h >= 0 && h < maxHang && c >= 0 && c < maxCot) {
          hang = h;
          cot = c;
          return 1; // Hợp lệ
        }
      } catch (...) {}
    }

    // Dự phòng cách nhập 2 số cũ: "hang cot" ví dụ "1 2" hoặc "1,2"
    int hOld = -1, cOld = -1;
    if (sscanf(clean.c_str(), "%d,%d", &hOld, &cOld) == 2 ||
        sscanf(s.c_str(), "%d %d", &hOld, &cOld) == 2) {
      if (hOld >= 0 && hOld < maxHang && cOld >= 0 && cOld < maxCot) {
        hang = hOld;
        cot = cOld;
        return 1;
      }
    }

    return -1;
  }

  // Số vé tối đa cho phép trong một lần đặt bán lẻ (Chuẩn CGV / Lotte Cinema: 8 vé)
  static const int MAX_VE_MAC_DINH = 8;

  // Phân tích danh sách mã ghế (vd: "b3 b4 b5", "B3, B4, B5") & thực thi quy tắc ghế liền kề
  // Trả về:
  //   1: Hợp lệ (dsGhe chứa các tọa độ hàng/cột đã sắp xếp)
  //   0: Người dùng hủy bỏ (nhập 0, q, huy, exit)
  //  -1: Không hợp lệ (thongBaoLoi chứa lý do chi tiết)
  static int phanTichDanhSachMaGhe(const std::string &inputRaw,
                                   std::vector<std::pair<int, int>> &dsGhe,
                                   std::string &thongBaoLoi,
                                   int maxHang, int maxCot, int maxVe = MAX_VE_MAC_DINH) {
    dsGhe.clear();
    thongBaoLoi = "";
    std::string s = trim(inputRaw);
    if (s.empty()) {
      thongBaoLoi = "Vui lòng nhập mã ghế!";
      return -1;
    }

    for (char &c : s) {
      c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    if (s == "0" || s == "Q" || s == "HUY" || s == "EXIT" || s == "CANCEL") {
      return 0; // Hủy
    }

    // Thay thế dấu phẩy ',' thành khoảng trắng để tách token dễ dàng
    for (char &c : s) {
      if (c == ',') c = ' ';
    }

    std::stringstream ss(s);
    std::string token;
    std::vector<std::string> dsToken;
    while (ss >> token) {
      dsToken.push_back(token);
    }

    if (dsToken.empty()) {
      thongBaoLoi = "Vui lòng nhập mã ghế!";
      return -1;
    }

    if ((int)dsToken.size() > maxVe) {
      thongBaoLoi = "Chỉ được đặt tối đa " + std::to_string(maxVe) + " vé trong một lượt đặt!";
      return -1;
    }

    for (const auto &t : dsToken) {
      if (t.length() < 2 || t[0] < 'A' || t[0] > 'Z') {
        thongBaoLoi = "Mã ghế [" + t + "] không đúng định dạng (Ví dụ đúng: A1, B3)!";
        return -1;
      }
      int h = t[0] - 'A';
      std::string phanSo = t.substr(1);
      int c = -1;
      try {
        c = std::stoi(phanSo) - 1;
      } catch (...) {
        thongBaoLoi = "Mã ghế [" + t + "] có số cột không hợp lệ!";
        return -1;
      }

      if (h < 0 || h >= maxHang || c < 0 || c >= maxCot) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Ghế [%s] vượt quá giới hạn phòng (Hàng A-%c, Cột 1-%d)!",
                 t.c_str(), (char)('A' + maxHang - 1), maxCot);
        thongBaoLoi = std::string(buf);
        return -1;
      }

      for (const auto &daChon : dsGhe) {
        if (daChon.first == h && daChon.second == c) {
          thongBaoLoi = "Mã ghế [" + t + "] bị nhập trùng lặp nhiều lần!";
          return -1;
        }
      }

      dsGhe.push_back({h, c});
    }

    // QUY TẮC CỤM GHẾ LIỀN KỀ 2D (2D 4-CONNECTED COMPONENT ENFORCEMENT)
    if (dsGhe.size() > 1) {
      // Hai ghế được gọi là kề nhau nếu chung cạnh ngang (cùng hàng, cột kề) hoặc chung cạnh dọc (cùng cột, hàng kề)
      auto laKeNhau = [](const std::pair<int, int> &p1, const std::pair<int, int> &p2) {
        int dHang = std::abs(p1.first - p2.first);
        int dCot = std::abs(p1.second - p2.second);
        return (dHang + dCot == 1);
      };

      // Duyệt BFS kiểm tra tính liên thông trực giao của toàn bộ cụm ghế đã chọn
      std::vector<bool> visited(dsGhe.size(), false);
      std::vector<int> q;
      q.push_back(0);
      visited[0] = true;
      int countConnected = 1;

      size_t head = 0;
      while (head < q.size()) {
        int u = q[head++];
        for (size_t v = 0; v < dsGhe.size(); v++) {
          if (!visited[v] && laKeNhau(dsGhe[u], dsGhe[v])) {
            visited[v] = true;
            countConnected++;
            q.push_back(v);
          }
        }
      }

      if (countConnected != (int)dsGhe.size()) {
        thongBaoLoi = "Quy tắc rạp: Các ghế chọn cùng lúc phải LIỀN KỀ NHAU (chung cạnh ngang hoặc dọc)!";
        return -1;
      }

      // Sắp xếp lại danh sách ghế để hiển thị đẹp mắt: Hàng tăng dần, sau đó Cột tăng dần
      std::sort(dsGhe.begin(), dsGhe.end(),
                [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second;
                });
    }

    return 1;
  }

  // === CAC HAM VE KHUNG ===

  static void veKhungTren(int doRong = DO_RONG) {
    std::cout << "\t\t╔";
    for (int i = 0; i < doRong; i++) std::cout << "═";
    std::cout << "╗\n";
  }

  static void veKhungGiua(int doRong = DO_RONG) {
    std::cout << "\t\t╠";
    for (int i = 0; i < doRong; i++) std::cout << "═";
    std::cout << "╣\n";
  }

  static void veKhungDuoi(int doRong = DO_RONG) {
    std::cout << "\t\t╚";
    for (int i = 0; i < doRong; i++) std::cout << "═";
    std::cout << "╝\n";
  }

  static void veDongTrong(int doRong = DO_RONG) {
    std::cout << "\t\t║" << std::string(doRong, ' ') << "║\n";
  }

  // Ve dong noi dung can trai, tu dong cat chuoi neu dai hon khung de chong tran
  static void veDong(const std::string &raw, int doRong = DO_RONG) {
    int maxInside = doRong - 4; // 2 spaces trai, 2 spaces phai
    std::string noiDung = raw;
    int vLen = doDaiHienThi(noiDung);

    if (vLen > maxInside) {
      noiDung = catChuoi(noiDung, maxInside - 3) + "...";
      vLen = maxInside;
    }

    int padding = doRong - 2 - vLen;
    if (padding < 0) padding = 0;

    std::cout << "\t\t║  " << noiDung << std::string(padding, ' ') << "║\n";
  }

  static void veDong(const char *raw, int doRong = DO_RONG) {
    veDong(std::string(raw), doRong);
  }

  // Ve dong noi dung can giua khung
  static void veDongGiua(const std::string &raw, int doRong = DO_RONG) {
    int maxInside = doRong - 2;
    std::string noiDung = raw;
    int vLen = doDaiHienThi(noiDung);

    if (vLen > maxInside) {
      noiDung = catChuoi(noiDung, maxInside - 3) + "...";
      vLen = maxInside;
    }

    int khoangTrong = doRong - vLen;
    int trai = khoangTrong / 2;
    int phai = khoangTrong - trai;

    std::cout << "\t\t║" << std::string(trai, ' ') << noiDung << std::string(phai, ' ') << "║\n";
  }

  static void veDongGiua(const char *raw, int doRong = DO_RONG) {
    veDongGiua(std::string(raw), doRong);
  }

  // Ve duong ke ngang phan cach
  static void veDuongKe(int doRong = DO_RONG) {
    std::cout << "\t\t║  ";
    for (int i = 0; i < doRong - 4; i++) std::cout << "─";
    std::cout << "  ║\n";
  }

  // Ve dong voi nhieu doan van ban co mau sac khac nhau, can chinh chong meo lech khung 100%
  static void veDongNhieuMau(const std::vector<DoanMau> &cacDoan, int doRong = DO_RONG) {
    int maxInside = doRong - 4; // 2 spaces trai, 2 spaces phai
    int tongVLen = 0;
    for (size_t i = 0; i < cacDoan.size(); i++) {
      tongVLen += doDaiHienThi(cacDoan[i].text);
    }

    std::vector<DoanMau> doanIn = cacDoan;
    if (tongVLen > maxInside) {
      int daDung = 0;
      for (size_t i = 0; i < doanIn.size(); i++) {
        int vLen = doDaiHienThi(doanIn[i].text);
        if (daDung + vLen > maxInside - 3) {
          int conLai = (maxInside - 3) - daDung;
          if (conLai > 0) {
            doanIn[i].text = catChuoi(doanIn[i].text, conLai) + "...";
          } else {
            doanIn[i].text = "...";
          }
          doanIn.resize(i + 1);
          break;
        }
        daDung += vLen;
      }
      tongVLen = 0;
      for (size_t i = 0; i < doanIn.size(); i++) {
        tongVLen += doDaiHienThi(doanIn[i].text);
      }
    }

    int padding = doRong - 2 - tongVLen;
    if (padding < 0) padding = 0;

    std::cout << "\t\t║  ";
    for (size_t i = 0; i < doanIn.size(); i++) {
      if (!doanIn[i].text.empty()) {
        datMau(doanIn[i].mau);
        std::cout << doanIn[i].text;
      }
    }
    resetMau();
    std::cout << std::string(padding, ' ') << "║\n";
  }

  // Ve dong tieu de phim voi Ma phim to mau Cyan ruc ro va Ten phim trang sang
  static void veDongPhimTieuDe(const std::string &maP, const std::string &tenP, int doRong = DO_RONG) {
    int maxInside = doRong - 4;
    std::string badgeMa = "[" + maP + "] ";
    int lenMa = doDaiHienThi(badgeMa);
    int maxTen = maxInside - lenMa;
    if (maxTen < 5) maxTen = 5;

    std::string tenIn = tenP;
    if (doDaiHienThi(tenIn) > maxTen) {
      tenIn = catChuoiTheoTu(tenIn, maxTen);
    }

    std::vector<DoanMau> cacDoan;
    cacDoan.push_back(DoanMau(badgeMa, MAU_MA_PHIM));
    cacDoan.push_back(DoanMau(tenIn, MAU_TRANG_SANG));
    veDongNhieuMau(cacDoan, doRong);
  }

  // Ve dong dao dien kem badge trang thai phim to mau (Xanh: Dang chieu / Vang: Sap chieu / Do: Ngung chieu)
  // Tu dong cat theo ranh gioi tu (khong bao gio chet dut doi ten nguoi)
  static void veDongPhimDaoDienVaTrangThai(const std::string &daoDien, int trangThai, const std::string &chuoiTrangThai, int doRong = DO_RONG) {
    int maxInside = doRong - 4;
    std::string prefix = "  Đạo diễn: ";
    std::string sep = " | ";
    std::string badgeTT = "[" + chuoiTrangThai + "]";

    int lenPrefix = doDaiHienThi(prefix);
    int lenSep = doDaiHienThi(sep);
    int lenBadge = doDaiHienThi(badgeTT);

    int maxDaoDien = maxInside - (lenPrefix + lenSep + lenBadge);
    if (maxDaoDien < 5) maxDaoDien = 5;

    std::string ddIn = daoDien;
    if (doDaiHienThi(ddIn) > maxDaoDien) {
      ddIn = catChuoiTheoTu(ddIn, maxDaoDien);
    }

    std::vector<DoanMau> cacDoan;
    cacDoan.push_back(DoanMau(prefix + ddIn + sep, MAU_MAC_DINH));
    cacDoan.push_back(DoanMau(badgeTT, layMauTheoTrangThai(trangThai)));
    veDongNhieuMau(cacDoan, doRong);
  }

  // === MAN HINH BANNER & THONG BAO ===

  // In banner khoi dong voi ASCII Art "CINEMA"
  static void inBanner() {
    veKhungTren();
    veDongTrong();

    const char *lines[] = {
        "  ██████╗ ██╗███╗   ██╗███████╗███╗   ███╗ █████╗   ",
        " ██╔════╝ ██║████╗  ██║██╔════╝████╗ ████║██╔══██╗  ",
        " ██║      ██║██╔██╗ ██║█████╗  ██╔████╔██║███████║  ",
        " ██║      ██║██║╚██╗██║██╔══╝  ██║╚██╔╝██║██╔══██║  ",
        " ╚██████╗ ██║██║ ╚████║███████╗██║ ╚═╝ ██║██║  ██║  ",
        "  ╚═════╝ ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝  "
    };

    for (int i = 0; i < 6; i++) {
      veDongGiua(lines[i]);
    }

    veDongTrong();
    veDongGiua("HỆ THỐNG QUẢN LÝ RẠP CHIẾU PHIM");
    veDongGiua("PBL2 - LẬP TRÌNH HƯỚNG ĐỐI TƯỢNG 2026");
    veDongTrong();
    veKhungDuoi();
  }

  // Cho nhan phim bat ky de tiep tuc
  static void choPhimBatKy() {
    std::cout << "\n\t\t>> Nhấn phím bất kỳ để tiếp tục...";
    _getch();
    std::cout << "\n";
  }

  // Hien thi thong bao thanh cong (vien va chu mau xanh la sang, tu dong xuong dong khong bao gio mat chu)
  static void hienThiThanhCong(const std::string &msg, int doRong = DO_RONG) {
    std::cout << "\n";
    datMau(MAU_DANG_CHIEU);
    veKhungTren(doRong);

    int maxInside = doRong - 4; // 2 spaces trai, 2 spaces phai
    int maxTextLen = maxInside - 3; // Tru 3 ky tu cho prefix ">> " hoac "   "
    if (maxTextLen < 10) maxTextLen = 10;

    std::vector<std::string> dsDong = tachDongTuDong(msg, maxTextLen);
    if (dsDong.empty()) {
      veDong(">> ", doRong);
    } else {
      for (size_t i = 0; i < dsDong.size(); i++) {
        std::string prefix = (i == 0) ? ">> " : "   ";
        std::string dongFull = prefix + dsDong[i];
        int vLen = doDaiHienThi(dongFull);
        int padding = doRong - 2 - vLen;
        if (padding < 0) padding = 0;
        std::cout << "\t\t║  " << dongFull << std::string(padding, ' ') << "║\n";
      }
    }

    veKhungDuoi(doRong);
    resetMau();
  }

  // Hien thi thong bao loi (vien va chu mau do sang, tu dong xuong dong khong bao gio mat chu)
  static void hienThiLoi(const std::string &msg, int doRong = DO_RONG) {
    std::cout << "\n";
    datMau(MAU_DO);
    veKhungTren(doRong);

    int maxInside = doRong - 4; // 2 spaces trai, 2 spaces phai
    int maxTextLen = maxInside - 3; // Tru 3 ky tu cho prefix "!! " hoac "   "
    if (maxTextLen < 10) maxTextLen = 10;

    std::vector<std::string> dsDong = tachDongTuDong(msg, maxTextLen);
    if (dsDong.empty()) {
      veDong("!! ", doRong);
    } else {
      for (size_t i = 0; i < dsDong.size(); i++) {
        std::string prefix = (i == 0) ? "!! " : "   ";
        std::string dongFull = prefix + dsDong[i];
        int vLen = doDaiHienThi(dongFull);
        int padding = doRong - 2 - vLen;
        if (padding < 0) padding = 0;
        std::cout << "\t\t║  " << dongFull << std::string(padding, ' ') << "║\n";
      }
    }

    veKhungDuoi(doRong);
    resetMau();
  }

  // Hien thi thong bao loi tam thoi trong khoang thoi gian ms (mac dinh 1500ms = 1.5s)
  static void hienThiLoiTamThoi(const std::string &msg, int ms = 1500) {
    hienThiLoi(msg);
    Sleep(ms);
  }

  // Hien thi thong bao thanh cong tam thoi trong khoang thoi gian ms (mac dinh 1000ms = 1s)
  static void hienThiThanhCongTamThoi(const std::string &msg, int ms = 1000) {
    hienThiThanhCong(msg);
    Sleep(ms);
  }

  // Doc mot so nguyen tu ban phim mot cach an toan, chong troi lenh, chong loi "1asdads"
  // Chi tra ve true khi toan bo dong nhap vao la mot so nguyen hop le duy nhat
  static bool docSoNguyen(int &ketQua) {
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cin.clear();
      return false;
    }
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
      return false;
    }
    size_t end = line.find_last_not_of(" \t\r\n");
    std::string trimmed = line.substr(start, end - start + 1);

    size_t i = 0;
    if (trimmed[0] == '-' || trimmed[0] == '+') {
      if (trimmed.length() == 1) return false;
      i = 1;
    }
    for (; i < trimmed.length(); i++) {
      if (!isdigit((unsigned char)trimmed[i])) {
        return false;
      }
    }

    try {
      ketQua = std::stoi(trimmed);
      return true;
    } catch (...) {
      return false;
    }
  }

  // Form dang nhap nam gon hoan toan trong khung
  // - Chan khong cho phep nhan Enter khi tai khoan hoac mat khau dang rong
  // - Nhan phim [Tab] de bat/tat an/hien mat khau (khong bi xung dot bo go Unikey, van nhap duoc chu 'o' binh thuong)
  // - Dieu huong muot ma, khong giat lag, khong chop nhay man hinh
  static bool nhapDangNhapTrongKhung(std::string &taiKhoan, std::string &matKhau) {
    taiKhoan = "";
    matKhau = "";
    bool hienMatKhau = false;
    int buoc = 0; // 0: dang nhap tai khoan, 1: dang nhap mat khau

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD goc = csbi.dwCursorPosition;

    // An con tro console tam thoi de ve khung khong bi lech
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    BOOL cursorGoc = cursorInfo.bVisible;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    auto veForm = [&]() {
      SetConsoleCursorPosition(hConsole, goc);

      veKhungTren();
      veDongGiua("ĐĂNG NHẬP HỆ THỐNG RẠP PHIM");
      veKhungGiua();

      // Dong tai khoan
      std::string hienThiTk = taiKhoan + (buoc == 0 ? "_" : "");
      veDong("Tài khoản: " + hienThiTk);

      // Dong mat khau
      std::string passStr = "";
      if (hienMatKhau) {
        passStr = matKhau;
      } else {
        passStr = std::string(matKhau.length(), '*');
      }
      if (buoc == 1) {
        passStr += "_";
      }
      std::string tagTrangThai = (buoc == 1) ? (hienMatKhau ? "  [ĐANG HIỆN]" : "  [ĐANG ẨN]") : "";
      veDong("Mật khẩu:  " + passStr + tagTrangThai);

      veKhungGiua();
      if (buoc == 0) {
        veDong(">> Nhập TÀI KHOẢN - Nhấn Enter để sang Mật khẩu");
      } else {
        std::string cheDo = hienMatKhau ? "[ĐANG HIỆN]" : "[ĐANG ẨN]";
        veDong(">> Trạng thái: " + cheDo + " - Bấm [Tab] để đổi");
      }
      veDong("Ghi chú: Enter: Xác nhận | Esc: Hủy đăng nhập");
      veKhungDuoi();
    };

    veForm();

    while (true) {
      int ch = _getch();

      // Xu ly cac phim mui ten hoac phim chuc nang dac biet
      if (ch == 0 || ch == 224) {
        _getch();
        continue;
      }

      if (ch == 27) {
        return false; // Phím Esc: Hủy đăng nhập an toàn
      }

      if (buoc == 0) {
        // --- DANG NHAP TAI KHOAN ---
        if (ch == '\r' || ch == '\n') {
          // Khong cho phep nhan Enter khi chua nhap gi
          if (!taiKhoan.empty()) {
            buoc = 1;
            veForm();
          }
        } else if (ch == '\b' || ch == 127) {
          if (!taiKhoan.empty()) {
            taiKhoan.pop_back();
            veForm();
          }
        } else if (ch >= 32 && ch <= 126) {
          if (taiKhoan.length() < 30) {
            taiKhoan += (char)ch;
            veForm();
          }
        }
      } else if (buoc == 1) {
        // --- DANG NHAP MAT KHAU ---
        if (ch == '\t' || ch == 9 || ch == 15) {
          // Phim [Tab] (ma 9) hoac [Ctrl + O] (ma 15): Bat/tat an hien mat khau
          // Dung Tab giup tranh 100% loi bo go tieng Viet (Unikey/EVKey Telex go o+o thanh o bi gui Backspace xoa ky tu)
          hienMatKhau = !hienMatKhau;
          veForm();
        } else if (ch == '\r' || ch == '\n') {
          // Khong cho phep nhan Enter khi mat khau rong
          if (!matKhau.empty()) {
            break; // Hoan thanh
          }
        } else if (ch == '\b' || ch == 127) {
          if (!matKhau.empty()) {
            matKhau.pop_back();
            veForm();
          } else {
            // Neu mat khau rong ma nhan Backspace -> quay lai sua tai khoan
            buoc = 0;
            veForm();
          }
        } else if (ch >= 32 && ch <= 126) {
          if (matKhau.length() < 30) {
            matKhau += (char)ch;
            veForm();
          }
        }
      }
    }

    // Khoi phuc con tro console
    cursorInfo.bVisible = cursorGoc;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    std::cout << "\n";
    return true;
  }

  // Nhap mat khau an (hien thi * thay vi ky tu that)
  static std::string nhapMatKhauAn() {
    std::string mk;
    char ch;
    while (true) {
      ch = _getch();
      if (ch == '\r' || ch == '\n') {
        std::cout << "\n";
        break;
      } else if (ch == '\b' || ch == 127) {
        if (!mk.empty()) {
          mk.pop_back();
          std::cout << "\b \b";
        }
      } else if (ch >= 32) {
        mk += ch;
        std::cout << "*";
      }
    }
    return mk;
  }

  // Cat tia khoang trang dau va cuoi chuoi
  static std::string catKhoangTrang(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
  }

  // Chuyen doi std::wstring sang std::string theo chuan UTF-8
  static std::string wstringSangUtf8(const std::wstring &wstr) {
    if (wstr.empty()) return std::string();
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0) return std::string();
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], sizeNeeded, NULL, NULL);
    return result;
  }

  // Nhap chuoi ky tu tuong tac voi khoa phim vat ly theo thoi gian thuc (Keystroke MaxLength Input Mask):
  // - Ho tro go tieng Viet (Unikey / EVKey Telex/VNI) 100% qua _getwch() va phan biet thong minh _kbhit()
  // - Chan vat ly khong cho phep go qua maxLen (mac dinh len den 100 ky tu), phat am thanh bip \a
  // - Hien thi bo dem thoi gian thuc (len/maxLen) va canh bao ruc ro o dong Trang thai ben tren
  // - Khong cho phep bo trong chuoi khi nhan Enter (bao loi tai cho)
  // - Khong bao gio tu y cat xen chu cua nguoi dung, bao toan 100% du lieu go vao
  // - Nhap "0" hoac nhan phim Esc de huy nhap (tra ve false)
  // - Ho tro Validator Callback kiem tra tinh hop le cua chuoi, giu nguyen chu cho sua khi loi (Phuong an A)
  static bool nhapChuoiGioiHan(std::string &ketQua, int maxLen, const std::string &nhan,
                               bool (*hamKiemTra)(const std::string&) = nullptr,
                               const std::string &thongBaoLoiTuyChinh = "") {
    std::cout << nhan << " (tối đa " << maxLen << " ký tự, nhập 0 hoặc Esc để hủy):\n";
    std::cout << "\t\t   ";
    datMau(MAU_XAM);
    std::cout << "Trạng thái: (0/" << maxLen << " ký tự)\n";
    resetMau();
    std::cout << "\t\t>> ";

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
      // Fallback cho moi truong khong phai console tuong tac truc tiep
      std::string s;
      if (!std::getline(std::cin, s)) return false;
      s = catKhoangTrang(s);
      if (s == "0" || s.empty()) return false;
      if (hamKiemTra != nullptr && !hamKiemTra(s)) return false;
      ketQua = s;
      return true;
    }

    COORD gocNhap = csbi.dwCursorPosition; // Vi tri ngay sau "\t\t>> "
    COORD gocThongBao = { (SHORT)gocNhap.X, (SHORT)(gocNhap.Y - 1) }; // Dong "Trang thai:" ngay phia tren

    std::wstring buffer = L"";

    auto capNhatManHinh = [&](int loaiThongBao) {
      CONSOLE_CURSOR_INFO cinfo;
      GetConsoleCursorInfo(hConsole, &cinfo);
      BOOL curVis = cinfo.bVisible;
      cinfo.bVisible = FALSE;
      SetConsoleCursorInfo(hConsole, &cinfo);

      // 1. Cap nhat dong Trang thai (o hang phia tren)
      SetConsoleCursorPosition(hConsole, gocThongBao);
      std::cout << "                                                                                     ";
      SetConsoleCursorPosition(hConsole, gocThongBao);

      if (loaiThongBao == 1 || (int)buffer.length() >= maxLen) {
        datMau(MAU_SAP_CHIEU);
        std::cout << "Trạng thái: [ĐÃ ĐẠT TỐI ĐA " << maxLen << " KÝ TỰ - ĐÃ KHÓA PHÍM!]";
        resetMau();
      } else if (loaiThongBao == 2) {
        datMau(MAU_DO);
        std::cout << "Trạng thái: [LỖI: Nội dung không được để trống! Vui lòng nhập]";
        resetMau();
      } else if (loaiThongBao == 3) {
        datMau(MAU_SAP_CHIEU);
        std::cout << "Trạng thái: [CẢNH BÁO: Ký tự '|' bị cấm để bảo vệ dữ liệu!]";
        resetMau();
      } else if (loaiThongBao == 4) {
        datMau(MAU_DO);
        if (!thongBaoLoiTuyChinh.empty()) {
          std::cout << thongBaoLoiTuyChinh;
        } else {
          std::cout << "Trạng thái: [LỖI: Dữ liệu không hợp lệ! Vui lòng sửa lại]";
        }
        resetMau();
      } else {
        datMau(MAU_XAM);
        std::cout << "Trạng thái: (" << buffer.length() << "/" << maxLen << " ký tự)";
        resetMau();
      }

      // 2. Ve lai chuoi nguoi dung dang go (o hang phia duoi)
      SetConsoleCursorPosition(hConsole, gocNhap);
      std::string utf8Text = wstringSangUtf8(buffer);
      std::cout << utf8Text;

      CONSOLE_SCREEN_BUFFER_INFO csbiText;
      GetConsoleScreenBufferInfo(hConsole, &csbiText);
      COORD viTriConTro = csbiText.dwCursorPosition;

      // Xoa sach cac ky tu du thua neu vua nhan Backspace
      std::cout << "               ";

      // 3. Tra con tro nhap nhay ve ngay sau ky tu vua go
      SetConsoleCursorPosition(hConsole, viTriConTro);
      cinfo.bVisible = curVis;
      SetConsoleCursorInfo(hConsole, &cinfo);
    };

    while (true) {
      wint_t wch = _getwch();

      // Xu ly phan biet phim mo rong (F1-F12, mui ten...)
      if (wch == 0) {
        _getwch(); // Phim mo rong co ma dau la 0
        continue;
      }
      if (wch == 0xE0) {
        // PHAN BIET THONG MINH GIUA PHIM MUI TEN VA CHU 'à' (Unicode 0x00E0 = 224):
        // Neu la phim mui ten, he dieu hanh phat 2 ma lien tiep => _kbhit() lap tuc tra ve true!
        // Neu la chu 'à' do Unikey gui den => _kbhit() tra ve false!
        if (_kbhit()) {
          _getwch(); // Bo qua ma thu 2 cua phim mui ten
          continue;
        }
        // _kbhit() == false: Day chinh la chu 'à'! Tiep tuc de dua vao buffer binh thuong!
      }

      // Thoat bang phim Esc (ma 27)
      if (wch == 27) {
        std::cout << "\n\n";
        return false;
      }

      // Nhan Enter
      if (wch == L'\r' || wch == L'\n') {
        std::string s = catKhoangTrang(wstringSangUtf8(buffer));
        if (s == "0") {
          std::cout << "\n\n";
          return false;
        }
        if (s.empty()) {
          capNhatManHinh(2); // Bao loi khong duoc de trong
          continue;
        }
        if (hamKiemTra != nullptr && !hamKiemTra(s)) {
          capNhatManHinh(4); // Phuong an A: Giu nguyen chu, hien loi do cho nguoi dung dung Backspace sua
          continue;
        }
        ketQua = s;
        std::cout << "\n\n";
        return true;
      }

      // Nhan Backspace (ma 8 hoac 127)
      if (wch == L'\b' || wch == 127) {
        if (!buffer.empty()) {
          buffer.pop_back();
          capNhatManHinh(0);
        }
        continue;
      }

      // Chan ky tu phan cach '|'
      if (wch == L'|') {
        capNhatManHinh(3);
        continue;
      }

      // Cac ky tu go thong thuong
      if (wch >= 32) {
        if ((int)buffer.length() >= maxLen) {
          // KHOA PHIM VAT LY KHI CHAM NGUONG GIOI HAN (Hoan toan im lang, chi hien thi canh bao thi giac)
          capNhatManHinh(1); // Dat gioi han toi da
        } else {
          buffer += (wchar_t)wch;
          capNhatManHinh(0);
        }
      }
    }
  }
};

#endif // TIENICHCONSOLE_H
