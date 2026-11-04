#ifndef VE_H
#define VE_H

#include "../giaodien/TienIchConsole.h"
#include "PhongChieu.h"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>

class Ve {
private:
  std::string maVe;
  std::string maSuatChieu;
  std::string tenPhim;
  std::string phongChieu;
  std::string ngayChieu;
  std::string gioChieu;
  std::string maGhe;
  double giaVe;

public:
  Ve() : giaVe(0) {}
  Ve(std::string maVe, std::string maSuatChieu, std::string tenPhim,
     std::string phongChieu, std::string ngayChieu, std::string gioChieu,
     std::string maGhe, double giaVe)
      : maVe(maVe), maSuatChieu(maSuatChieu), tenPhim(tenPhim),
        phongChieu(phongChieu), ngayChieu(ngayChieu), gioChieu(gioChieu),
        maGhe(maGhe), giaVe(giaVe) {}

  std::string layMaVe() const { return maVe; }
  std::string layMaSuatChieu() const { return maSuatChieu; }
  std::string layTenPhim() const { return tenPhim; }
  std::string layPhongChieu() const { return phongChieu; }
  std::string layNgayChieu() const { return ngayChieu; }
  std::string layGioChieu() const { return gioChieu; }
  std::string layMaGhe() const { return maGhe; }
  double layGiaVe() const { return giaVe; }

  void hienThiVe() const {
    char dong[256];
    TienIchConsole::veDuongKe();
    snprintf(dong, sizeof(dong), "Mã vé : %s", maVe.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Phim  : %s", tenPhim.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Phòng : %s  |  Ghế: %s",
             phongChieu.c_str(), maGhe.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Suất  : %s - %s", gioChieu.c_str(),
             ngayChieu.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Giá   : %s VNĐ",
             TienIchConsole::dinhDangTien(giaVe).c_str());
    TienIchConsole::veDong(dong);
  }

  bool operator==(const Ve &other) const { return maVe == other.maVe; }
  friend std::ostream &operator<<(std::ostream &os, const Ve &v) {
    os << "[ " << v.maVe << " | " << v.tenPhim << " | Ghế: " << v.maGhe
       << " ]";
    return os;
  }
};

#endif
