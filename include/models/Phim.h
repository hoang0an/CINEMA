#ifndef PHIM_H
#define PHIM_H

#include "../tienich/TienIchNgay.h"
#include <iostream>
#include <string>

class Phim {
private:
  std::string maPhim;
  std::string tenPhim;
  std::string theLoai;
  std::string daoDien;
  int thoiLuong;
  int trangThai; // 1: Đang chiếu, 2: Sắp chiếu, 0: Ngừng chiếu
  std::string ngayKhoiChieu; // YYYY-MM-DD
  int soNgayChieu;           // Số ngày trình chiếu tại rạp

public:
  Phim()
      : thoiLuong(0), trangThai(1), ngayKhoiChieu("2026-10-30"),
        soNgayChieu(30) {}

  // Constructor tương thích ngược 6 tham số cũ
  Phim(std::string maPhim, std::string tenPhim, std::string theLoai,
       std::string daoDien, int thoiLuong, int trangThai = 1)
      : maPhim(maPhim), tenPhim(tenPhim), theLoai(theLoai), daoDien(daoDien),
        thoiLuong(thoiLuong), trangThai(trangThai),
        ngayKhoiChieu("2026-10-24"), soNgayChieu(30) {}

  // Constructor đầy đủ 8 tham số mới hỗ trợ Vòng đời rạp phim (Theatrical Lifecycle)
  Phim(std::string maPhim, std::string tenPhim, std::string theLoai,
       std::string daoDien, int thoiLuong, int trangThai,
       std::string ngayKhoiChieu, int soNgayChieu = 30)
      : maPhim(maPhim), tenPhim(tenPhim), theLoai(theLoai), daoDien(daoDien),
        thoiLuong(thoiLuong), trangThai(trangThai),
        ngayKhoiChieu(TienIchNgay::chuanHoaISO(ngayKhoiChieu)),
        soNgayChieu(soNgayChieu > 0 ? soNgayChieu : 30) {}

  std::string layMaPhim() const { return maPhim; }
  std::string layTenPhim() const { return tenPhim; }
  std::string layTheLoai() const { return theLoai; }
  std::string layDaoDien() const { return daoDien; }
  int layThoiLuong() const { return thoiLuong; }
  int layTrangThai() const { return trangThai; }
  std::string layNgayKhoiChieu() const { return ngayKhoiChieu; }
  int laySoNgayChieu() const { return soNgayChieu; }

  std::string layNgayKetThuc() const {
    return TienIchNgay::congNgay(ngayKhoiChieu, soNgayChieu);
  }

  std::string layChuoiTrangThai() const {
    switch (trangThai) {
    case 1:
      return "Đang chiếu";
    case 2:
      return "Sắp chiếu";
    case 0:
      return "Ngừng chiếu";
    default:
      return "Không xác định";
    }
  }

  void setTrangThai(int tt) { trangThai = tt; }

  void setNgayKhoiChieu(const std::string &ngay) {
    ngayKhoiChieu = TienIchNgay::chuanHoaISO(ngay);
  }

  void setSoNgayChieu(int sn) {
    if (sn > 0) soNgayChieu = sn;
  }

  bool setNgayKetThuc(const std::string &ngay) {
    const auto iso=TienIchNgay::chuanHoaISO(ngay);
    std::tm a{},b{};
    if (!TienIchNgay::phanTichISO(ngayKhoiChieu,a) || !TienIchNgay::phanTichISO(iso,b)) return false;
    const int span=TienIchNgay::khoangCachNgay(ngayKhoiChieu,iso);
    if (span<1 || span>365) return false;
    soNgayChieu=span; return true; // Giữ định dạng file hiện tại: start + span = ngày cuối.
  }

  bool setThoiLuong(int tl) {
    if (tl > 0) {
      thoiLuong = tl;
      return true;
    }
    return false;
  }

  // Kiểm tra ngày có thỏa mãn suất chiếu sớm (Sneak Show) tại P05:
  // Đúng 1 hoặc 2 ngày trước khi phim công chiếu chính thức
  bool coTheChieuSom(const std::string &ngayKiemTraISO) const {
    int kc = TienIchNgay::khoangCachNgay(ngayKiemTraISO, ngayKhoiChieu);
    return (kc >= 1 && kc <= 2);
  }

  // Tự động tính trạng thái theo ngày kiểm tra
  int tinhTrangThaiTheoNgay(const std::string &ngayKiemTraISO) const {
    int kc = TienIchNgay::khoangCachNgay(ngayKhoiChieu, ngayKiemTraISO);
    if (kc < 0) {
      return 2; // Sắp chiếu
    }
    if (kc >= 0 && kc <= soNgayChieu) {
      return 1; // Đang chiếu
    }
    return 0; // Ngừng chiếu (quá hạn)
  }

  // Đồng bộ trạng thái tự động với thời gian thực
  void dongBoTrangThaiThoiGianThuc(const std::string &ngayHienTaiISO) {
    // Nếu phim bị ép ngừng chiếu thủ công (Early pull-out), giữ nguyên 0
    if (trangThai == 0) return;
    trangThai = tinhTrangThaiTheoNgay(ngayHienTaiISO);
  }

  void hienThi() const {
    std::cout << "[" << maPhim << "] " << tenPhim << " | Thể loại: " << theLoai
              << " | Thời lượng: " << thoiLuong << " phút"
              << " | KC: " << TienIchNgay::dinhDangHienThi(ngayKhoiChieu)
              << " | Hết đợt: " << TienIchNgay::dinhDangHienThi(layNgayKetThuc())
              << " | Trạng thái: " << layChuoiTrangThai() << std::endl;
  }

  bool operator<(const Phim &other) const { return maPhim < other.maPhim; }
  bool operator>(const Phim &other) const { return maPhim > other.maPhim; }
  bool operator==(const Phim &other) const { return maPhim == other.maPhim; }
  bool operator!=(const Phim &other) const { return maPhim != other.maPhim; }

  friend std::ostream &operator<<(std::ostream &os, const Phim &p) {
    os << "[" << p.maPhim << "]" << p.tenPhim << "(" << p.thoiLuong << "p)";
    return os;
  }
};

#endif // PHIM_H
