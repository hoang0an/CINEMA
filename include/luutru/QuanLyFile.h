#ifndef QUANLYFILE_H
#define QUANLYFILE_H

#include "../cautruc/DanhSachLienKet.h"
#include "../giaodien/TienIchConsole.h"
#include "../models/NguoiDung.h"
#include "../models/NhanVien.h"
#include "../models/Phim.h"
#include "../models/QuanTriVien.h"
#include "../models/SuatChieu.h"
#include "../tienich/TienIchNgay.h"
#include <cctype>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

class QuanLyFile {
public:
  // Khử ký tự phân cách '|' và ký tự xuống dòng để chống phá hoại cấu trúc file CSV/delimited
  static std::string khuKyTuPhanCach(const std::string &s) {
    std::string ketQua = s;
    for (size_t i = 0; i < ketQua.length(); i++) {
      if (ketQua[i] == '|' || ketQua[i] == '\r' || ketQua[i] == '\n') {
        ketQua[i] = '-';
      }
    }
    return ketQua;
  }

  // Khử ký tự điều khiển (ASCII < 32 và 127) như null, bell (\a), backspace (\b), ANSI escape
  // nhưng bảo toàn 100% byte mở rộng UTF-8 (>= 128) để tiếng Việt không bị suy hao
  static std::string khuKyTuDieuKhien(const std::string &s) {
    std::string clean = "";
    for (unsigned char c : s) {
      if (c >= 32 && c != 127) {
        clean += static_cast<char>(c);
      }
    }
    return clean;
  }

  // Kiểm tra mã phim chuẩn định dạng hệ thống: đúng 6 ký số (YYXXXX, ví dụ: 260001)
  static bool kiemTraMaPhimHopLe(const std::string &ma) {
    if (ma.length() != 6) return false;
    for (char c : ma) {
      if (!std::isdigit(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    return true;
  }

  // KIỂM TRA CHUỖI HỢP LỆ THEO BỘ 4 TIÊU CHÍ HÌNH THÁI HỌC ĐIỆN ẢNH (Dev-Team Consensus):
  // 1. Tiêu chí 1: Ngưỡng tối thiểu
  //    - choPhepSo = true (Tên phim): Cần ít nhất 2 chữ hoặc số (hỗ trợ "IT", "UP", "300", "1917"...)
  //    - choPhepSo = false (Thể loại, Đạo diễn): Bắt buộc cần ít nhất 2 chữ cái (không thể là số thuần túy)
  // 2. Tiêu chí 2: Chống đè phím spam ký tự (Anti-mash)
  //    - Không ký tự nào được lặp lại liên tiếp >= 4 lần (ví dụ: aaaa, oooo, xxxx...)
  // 3. Tiêu chí 3: Chống spam dấu câu liên tiếp
  //    - Dấu câu không được lặp lại >= 2 lần liên tiếp (,, !!, ??, --, //, __...)
  //    - Riêng dấu chấm '.' được phép tối đa 3 lần liên tiếp (...) cho dấu chấm lửng
  // 4. Tiêu chí 4: Tỷ lệ mật độ chữ cái (Alphanumeric Density >= 50%)
  //    - Số lượng chữ cái/chữ số phải chiếm ít nhất 50% tổng số ký tự hiển thị (loại trừ khoảng trắng)
  static bool kiemTraChuoiHopLe(const std::string &s, bool choPhepSo = true) {
    if (s.empty()) return false;

    int soChuCai = 0;
    int soChuSo = 0;
    int tongKyTuKhongTrang = 0;

    std::string kyTuTruoc = "";
    int soLanLapKyTu = 0;

    char dauCauTruoc = '\0';
    int soLanLapDauCau = 0;

    size_t i = 0;
    while (i < s.length()) {
      unsigned char c = static_cast<unsigned char>(s[i]);
      std::string kyTuHienTai = "";
      size_t byteLen = 1;

      if ((c & 0x80) == 0) {
        byteLen = 1;
        kyTuHienTai = s.substr(i, 1);
      } else if ((c & 0xE0) == 0xC0) {
        byteLen = 2;
        kyTuHienTai = s.substr(i, (i + 2 <= s.length()) ? 2 : 1);
      } else if ((c & 0xF0) == 0xE0) {
        byteLen = 3;
        kyTuHienTai = s.substr(i, (i + 3 <= s.length()) ? 3 : 1);
      } else if ((c & 0xF8) == 0xF0) {
        byteLen = 4;
        kyTuHienTai = s.substr(i, (i + 4 <= s.length()) ? 4 : 1);
      } else {
        byteLen = 1;
        kyTuHienTai = s.substr(i, 1);
      }
      i += byteLen;

      // Xử lý khoảng trắng
      if (byteLen == 1 && std::isspace(c)) {
        dauCauTruoc = '\0';
        soLanLapDauCau = 0;
        kyTuTruoc = "";
        soLanLapKyTu = 0;
        continue;
      }

      tongKyTuKhongTrang++;

      // Tiêu chí 2: Kiểm tra lặp ký tự liên tiếp (Anti-mash)
      if (kyTuHienTai == kyTuTruoc) {
        soLanLapKyTu++;
        if (soLanLapKyTu >= 4) {
          return false; // Spam 1 ký tự >= 4 lần (vd: aaaa, oooo)
        }
      } else {
        kyTuTruoc = kyTuHienTai;
        soLanLapKyTu = 1;
      }

      // Phân loại ký tự
      if (byteLen > 1) {
        // Ký tự đa byte UTF-8 (tiếng Việt có dấu: á, à, ệ, ơ, Đ, đ...)
        soChuCai++;
        dauCauTruoc = '\0';
        soLanLapDauCau = 0;
      } else if (std::isalpha(c)) {
        soChuCai++;
        dauCauTruoc = '\0';
        soLanLapDauCau = 0;
      } else if (std::isdigit(c)) {
        soChuSo++;
        dauCauTruoc = '\0';
        soLanLapDauCau = 0;
      } else {
        // Dấu câu hoặc ký hiệu đặc biệt
        char dauCau = static_cast<char>(c);
        if (dauCau == dauCauTruoc) {
          soLanLapDauCau++;
          if (dauCau == '.' && soLanLapDauCau > 3) {
            return false; // Quá 3 dấu chấm liên tiếp
          }
          if (dauCau != '.' && soLanLapDauCau >= 2) {
            return false; // Dấu câu khác lặp >= 2 lần (,, !!, ??, --...)
          }
        } else {
          dauCauTruoc = dauCau;
          soLanLapDauCau = 1;
        }
      }
    }

    // Tiêu chí 1: Ngưỡng tối thiểu
    int tongChuVaSo = soChuCai + soChuSo;
    if (choPhepSo) {
      if (tongChuVaSo < 2) return false; // Tên phim cần >= 2 chữ hoặc số
    } else {
      if (soChuCai < 2) return false; // Thể loại / Đạo diễn bắt buộc có >= 2 chữ cái
    }

    // Tiêu chí 4: Tỷ lệ mật độ chữ cái / chữ số >= 50%
    if (tongKyTuKhongTrang == 0) return false;
    double tyLe = static_cast<double>(tongChuVaSo) / tongKyTuKhongTrang;
    if (tyLe < 0.5) {
      return false; // Ký tự chữ số bị loãng dưới 50% (ví dụ: a,,,,,,,,,,,,,,)
    }

    return true;
  }

  static bool kiemTraTenPhimHopLe(const std::string &s) {
    return kiemTraChuoiHopLe(s, true);
  }

  static bool kiemTraTheLoaiHopLe(const std::string &s) {
    return kiemTraChuoiHopLe(s, false);
  }

  static bool kiemTraDaoDienHopLe(const std::string &s) {
    return kiemTraChuoiHopLe(s, false);
  }

  // Khái quát hoá tương thích ngược
  static bool coNoiDungCoNghia(const std::string &s) {
    return kiemTraTenPhimHopLe(s);
  }

  // Trim khoảng trắng đầu cuối
  static std::string trim(const std::string &str) {
    size_t dau = str.find_first_not_of(" \t\r\n");
    if (dau == std::string::npos) return "";
    size_t cuoi = str.find_last_not_of(" \t\r\n");
    return str.substr(dau, cuoi - dau + 1);
  }

  static bool ghiDanhSachPhim(const std::string &duongDan,
                              const DanhSachLienKet<Phim> &dsPhim) {
    std::ofstream file(duongDan);

    if (!file.is_open()) {
      return false;
    }

    Nut<Phim> *hienTai = dsPhim.layDau();
    while (hienTai != nullptr) {
      file << khuKyTuDieuKhien(khuKyTuPhanCach(hienTai->duLieu.layMaPhim())) << "|"
           << khuKyTuDieuKhien(khuKyTuPhanCach(hienTai->duLieu.layTenPhim())) << "|"
           << khuKyTuDieuKhien(khuKyTuPhanCach(hienTai->duLieu.layTheLoai())) << "|"
           << khuKyTuDieuKhien(khuKyTuPhanCach(hienTai->duLieu.layDaoDien())) << "|"
           << hienTai->duLieu.layThoiLuong() << "|"
           << hienTai->duLieu.layTrangThai() << "|"
           << hienTai->duLieu.layNgayKhoiChieu() << "|"
           << hienTai->duLieu.laySoNgayChieu() << "\n";
      hienTai = hienTai->tiepTheo;
    }

    file.close();
    return !file.fail();
  }

  // Đọc danh sách phim với Màng Lọc Phòng Thủ 3 Tầng Toàn Diện (Defensive Ingestion Pipeline):
  // 1. TẦNG CẤU TRÚC: Hỗ trợ linh hoạt cả 6 cột cũ và 8 cột mới (Vòng đời rạp phim)
  // 2. TẦNG NGỮ NGHĨA: Từ chối chuỗi vô nghĩa/ký tự rác (toàn dấu phẩy, ký hiệu lạ), thời lượng 1-600 phút
  // 3. TẦNG BIÊN AN TOÀN: Tự động kẹp biên độ dài (45 / 30 / 25) và đồng bộ trạng thái thời gian thực
  static bool docDanhSachPhim(const std::string &duongDan,
                              DanhSachLienKet<Phim> &dsPhim) {
    std::ifstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }
    std::string dong;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();

    while (std::getline(file, dong)) {
      if (!dong.empty() && dong.back() == '\r') {
        dong.pop_back();
      }
      if (dong.empty()) {
        continue;
      }

      std::stringstream ss(dong);
      std::string maPhim, tenPhim, theLoai, daoDien, chuoiThoiLuong;
      std::string chuoiTrangThai, chuoiNgayKC, chuoiSoNgayChieu;

      if (!std::getline(ss, maPhim, '|') ||
          !std::getline(ss, tenPhim, '|') ||
          !std::getline(ss, theLoai, '|') ||
          !std::getline(ss, daoDien, '|') ||
          !std::getline(ss, chuoiThoiLuong, '|')) {
        continue; // Dòng bị thiếu cột nghiêm trọng -> bỏ qua an toàn
      }
      std::getline(ss, chuoiTrangThai, '|');
      std::getline(ss, chuoiNgayKC, '|');
      std::getline(ss, chuoiSoNgayChieu);

      // TẦNG 1: Khử ký tự điều khiển và chuẩn hóa khoảng trắng thừa
      maPhim = trim(khuKyTuDieuKhien(maPhim));
      // Khử UTF-8 BOM (0xEF, 0xBB, 0xBF) nếu xuất hiện ở đầu dòng 1
      if (maPhim.length() >= 3 &&
          static_cast<unsigned char>(maPhim[0]) == 0xEF &&
          static_cast<unsigned char>(maPhim[1]) == 0xBB &&
          static_cast<unsigned char>(maPhim[2]) == 0xBF) {
        maPhim = maPhim.substr(3);
      }
      tenPhim = trim(khuKyTuDieuKhien(tenPhim));
      theLoai = trim(khuKyTuDieuKhien(theLoai));
      daoDien = trim(khuKyTuDieuKhien(daoDien));

      // Kiểm tra định dạng mã phim (bắt buộc đúng 6 ký số YYXXXX)
      if (!kiemTraMaPhimHopLe(maPhim)) {
        continue;
      }

      // TẦNG 2: Kiểm tra nội dung hợp lệ theo bộ 4 tiêu chí hình thái học điện ảnh
      if (!kiemTraTenPhimHopLe(tenPhim) || !kiemTraTheLoaiHopLe(theLoai) || !kiemTraDaoDienHopLe(daoDien)) {
        continue;
      }

      // Kiểm tra và ép kiểu thời lượng an toàn
      int thoiLuong = 0;
      try {
        thoiLuong = std::stoi(chuoiThoiLuong);
        if (thoiLuong <= 0 || thoiLuong > 600) {
          continue; // Thời lượng vô lý (phim phải từ 1 đến 600 phút)
        }
      } catch (...) {
        continue; // Dữ liệu chữ hoặc rác (abc, NaN...) -> bỏ qua an toàn, không crash
      }

      // Kiểm tra trạng thái (0: Ngừng chiếu, 1: Đang chiếu, 2: Sắp chiếu)
      int trangThai = 1;
      if (!chuoiTrangThai.empty()) {
        try {
          int parsedTt = std::stoi(chuoiTrangThai);
          if (parsedTt == 0 || parsedTt == 1 || parsedTt == 2) {
            trangThai = parsedTt;
          } else {
            trangThai = 1; // Mặc định về Đang chiếu nếu nhập số lạ
          }
        } catch (...) {
          trangThai = 1;
        }
      }

      // Ngày khởi chiếu và số ngày chiếu (Tương thích ngược 6 cột -> mặc định)
      std::string ngayKC = trim(chuoiNgayKC);
      if (ngayKC.empty()) {
        ngayKC = "2026-10-24";
      } else {
        ngayKC = TienIchNgay::chuanHoaISO(ngayKC);
      }

      int soNgayChieu = 30;
      if (!chuoiSoNgayChieu.empty()) {
        try {
          int sn = std::stoi(chuoiSoNgayChieu);
          if (sn > 0) soNgayChieu = sn;
        } catch (...) {}
      }

      // TẦNG 3: Kẹp biên an toàn (Bounds Clamping) chống tràn dữ liệu giao diện
      tenPhim = TienIchConsole::catChuoi(tenPhim, 45);
      theLoai = TienIchConsole::catChuoi(theLoai, 30);
      daoDien = TienIchConsole::catChuoi(daoDien, 25);

      // Chống trùng lặp mã phim trong file
      bool daTonTai = false;
      Nut<Phim> *duyet = dsPhim.layDau();
      while (duyet != nullptr) {
        if (duyet->duLieu.layMaPhim() == maPhim) {
          daTonTai = true;
          break;
        }
        duyet = duyet->tiepTheo;
      }
      if (daTonTai) {
        continue; // Bỏ qua bản ghi trùng mã phim
      }

      Phim p(maPhim, tenPhim, theLoai, daoDien, thoiLuong, trangThai, ngayKC, soNgayChieu);
      p.dongBoTrangThaiThoiGianThuc(ngayHienTai);
      dsPhim.themCuoi(p);
    }
    file.close();
    return !file.bad();
  }

  // Đọc danh sách suất chiếu từ file dulieu/suatchieu.txt
  static bool docDanhSachSuatChieu(const std::string &duongDan,
                                   DanhSachLienKet<SuatChieu *> &dsSuatChieu) {
    std::ifstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }
    std::string dong;
    while (std::getline(file, dong)) {
      if (!dong.empty() && dong.back() == '\r') {
        dong.pop_back();
      }
      if (dong.empty()) continue;

      std::stringstream ss(dong);
      std::string maSC, maPhim, tenPhim, ngayChieu, gioChieu, maPhong, tenPhong;
      std::string chuoiSoHang, chuoiSoCot, chuoiGhe;

      if (!std::getline(ss, maSC, '|') ||
          !std::getline(ss, maPhim, '|') ||
          !std::getline(ss, tenPhim, '|') ||
          !std::getline(ss, ngayChieu, '|') ||
          !std::getline(ss, gioChieu, '|') ||
          !std::getline(ss, maPhong, '|') ||
          !std::getline(ss, tenPhong, '|') ||
          !std::getline(ss, chuoiSoHang, '|') ||
          !std::getline(ss, chuoiSoCot, '|')) {
        continue;
      }
      std::getline(ss, chuoiGhe);

      int soHang = 8, soCot = 6;
      try {
        size_t n=0;
        soHang = std::stoi(chuoiSoHang,&n); if(n!=chuoiSoHang.size()) throw 0;
        soCot = std::stoi(chuoiSoCot,&n); if(n!=chuoiSoCot.size()) throw 0;
      } catch (...) { std::cerr << "Invalid room: " << maSC << "\n"; continue; }
      bool gioHopLe = gioChieu.size()==5 && gioChieu[2]==':' &&
        gioChieu[0]>='0' && gioChieu[0]<='2' && gioChieu[1]>='0' && gioChieu[1]<='9' &&
        gioChieu[3]>='0' && gioChieu[3]<='5' && gioChieu[4]>='0' && gioChieu[4]<='9' && gioChieu.substr(0,2)<="23";
      std::tm parsed{};
      if (!gioHopLe || soHang < 1 || soHang > 26 || soCot < 1 || soCot > 100 ||
          !TienIchNgay::phanTichISO(ngayChieu, parsed)) {
        std::cerr << "Invalid show: " << maSC << "\n"; continue;
      }

      SuatChieu *sc = new SuatChieu(maSC, maPhim, tenPhim, ngayChieu, gioChieu,
                                    maPhong, tenPhong, soHang, soCot);
      if (!chuoiGhe.empty() && sc->layPhongChieu() != nullptr) {
        sc->layPhongChieu()->napChuoiTrangThaiGhe(chuoiGhe);
      }
      dsSuatChieu.themCuoi(sc);
    }
    file.close();
    return !file.bad();
  }

  // Ghi danh sách suất chiếu và toàn bộ trạng thái ghế vào file dulieu/suatchieu.txt
  static bool ghiDanhSachSuatChieu(const std::string &duongDan,
                                   const DanhSachLienKet<SuatChieu *> &dsSuatChieu) {
    std::ofstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }
    Nut<SuatChieu *> *hienTai = dsSuatChieu.layDau();
    while (hienTai != nullptr) {
      SuatChieu *sc = hienTai->duLieu;
      if (sc != nullptr && sc->layPhongChieu() != nullptr) {
        file << sc->layMaSuatChieu() << "|"
             << sc->layMaPhim() << "|"
             << khuKyTuDieuKhien(khuKyTuPhanCach(sc->layTenPhim())) << "|"
             << sc->layNgayChieu() << "|"
             << sc->layGioChieu() << "|"
             << sc->layPhongChieu()->layMaPhong() << "|"
             << khuKyTuDieuKhien(khuKyTuPhanCach(sc->layPhongChieu()->layTenPhong())) << "|"
             << sc->layPhongChieu()->laySoHang() << "|"
             << sc->layPhongChieu()->laySoCot() << "|"
             << sc->layPhongChieu()->xuatChuoiTrangThaiGhe() << "\n";
      }
      hienTai = hienTai->tiepTheo;
    }
    file.close();
    return !file.fail();
  }

  static bool docDanhSachNguoiDung(const std::string &duongDan,
                                   DanhSachLienKet<NguoiDung *> &dsNguoiDung) {
    std::ifstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }

    std::string dong;
    while (std::getline(file, dong)) {
      if (!dong.empty() && dong.back() == '\r') {
        dong.pop_back();
      }
      if (dong.empty()) {
        continue;
      }

      std::vector<std::string> tokens;
      std::string token;
      std::stringstream ss(dong);
      while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
      }
      if (tokens.size() < 5) continue;

      std::string ma = tokens[0];
      std::string taiKhoan = tokens[1];
      std::string matKhau = tokens[2];
      std::string hoTen = tokens[3];
      std::string vaiTro = tokens[4];
      std::string cccd = (tokens.size() > 5 && !tokens[5].empty()) ? tokens[5] : "Chưa cập nhật";
      std::string sdt = (tokens.size() > 6 && !tokens[6].empty()) ? tokens[6] : "Chưa cập nhật";
      std::string diaChi = (tokens.size() > 7 && !tokens[7].empty()) ? tokens[7] : "Chưa cập nhật";

      if (vaiTro == "ADMIN") {
        dsNguoiDung.themCuoi(new QuanTriVien(ma, taiKhoan, matKhau, hoTen,
                                            (cccd == "Chưa cập nhật" ? "Hệ thống" : cccd),
                                            (sdt == "Chưa cập nhật" ? "Hệ thống" : sdt),
                                            (diaChi == "Chưa cập nhật" ? "Hệ thống" : diaChi)));
      } else if (vaiTro == "NHANVIEN") {
        dsNguoiDung.themCuoi(new NhanVien(ma, taiKhoan, matKhau, hoTen, cccd, sdt, diaChi));
      }
    }
    file.close();
    return !file.bad();
  }

  static bool ghiDanhSachNguoiDung(const std::string &duongDan,
                                   const DanhSachLienKet<NguoiDung *> &dsNguoiDung) {
    std::ofstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }

    Nut<NguoiDung *> *hienTai = dsNguoiDung.layDau();
    while (hienTai != nullptr) {
      NguoiDung *nd = hienTai->duLieu;
      if (nd != nullptr) {
        file << khuKyTuPhanCach(nd->layMa()) << "|"
             << khuKyTuPhanCach(nd->layTaiKhoan()) << "|"
             << khuKyTuPhanCach(nd->layMatKhau()) << "|"
             << khuKyTuPhanCach(nd->layHoTen()) << "|"
             << nd->layMaVaiTro() << "|"
             << khuKyTuPhanCach(nd->layCCCD()) << "|"
             << khuKyTuPhanCach(nd->laySoDienThoai()) << "|"
             << khuKyTuPhanCach(nd->layDiaChi()) << "\n";
      }
      hienTai = hienTai->tiepTheo;
    }
    file.close();
    return !file.fail();
  }
};

#endif
