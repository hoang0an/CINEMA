#ifndef SUATCHIEU_H
#define SUATCHIEU_H

#include "../cautruc/DanhSachLienKet.h"
#include "../tienich/TienIchNgay.h"
#include "PhongChieu.h"
#include <iostream>
#include <string>

class SuatChieu {
private:
  std::string maSuatChieu;
  std::string maPhim;
  std::string tenPhim;
  std::string ngayChieu; // YYYY-MM-DD
  std::string gioChieu;  // HH:MM
  PhongChieu *phongChieu;

public:
  SuatChieu(std::string maSC, std::string maP, std::string tenP,
            std::string ngay, std::string gio, std::string maPhong,
            std::string tenPhong, int soHang, int soCot)
      : maSuatChieu(maSC), maPhim(maP), tenPhim(tenP),
        ngayChieu(TienIchNgay::chuanHoaISO(ngay)), gioChieu(gio),
        phongChieu(nullptr) {
    phongChieu = new PhongChieu(maPhong, tenPhong, soHang, soCot);
  }

  // Rule of Three: Copy Constructor
  SuatChieu(const SuatChieu &other)
      : maSuatChieu(other.maSuatChieu), maPhim(other.maPhim),
        tenPhim(other.tenPhim), ngayChieu(other.ngayChieu),
        gioChieu(other.gioChieu), phongChieu(nullptr) {
    if (other.phongChieu != nullptr) {
      phongChieu = new PhongChieu(*(other.phongChieu));
    }
  }

  // Rule of Three: Copy Assignment Operator
  SuatChieu &operator=(const SuatChieu &other) {
    if (this != &other) {
      delete phongChieu;
      maSuatChieu = other.maSuatChieu;
      maPhim = other.maPhim;
      tenPhim = other.tenPhim;
      ngayChieu = other.ngayChieu;
      gioChieu = other.gioChieu;
      phongChieu = nullptr;
      if (other.phongChieu != nullptr) {
        phongChieu = new PhongChieu(*(other.phongChieu));
      }
    }
    return *this;
  }

  ~SuatChieu() {
    delete phongChieu;
  }

  std::string layMaSuatChieu() const { return maSuatChieu; }
  std::string layMaPhim() const { return maPhim; }
  std::string layTenPhim() const { return tenPhim; }
  void setTenPhim(const std::string &ten) { tenPhim = ten; }
  std::string layNgayChieu() const { return ngayChieu; }
  std::string layGioChieu() const { return gioChieu; }
  PhongChieu *layPhongChieu() const { return phongChieu; }

  int demSoGheDaDat() const {
    return (phongChieu != nullptr) ? phongChieu->demSoGheDaDat() : 0;
  }

  int demSoGheTrong() const {
    return (phongChieu != nullptr) ? phongChieu->demSoGheTrong() : 0;
  }

  int layTongSoGhe() const {
    return (phongChieu != nullptr)
               ? (phongChieu->laySoHang() * phongChieu->laySoCot())
               : 0;
  }

  bool laPhongVIP() const {
    return (phongChieu != nullptr) && phongChieu->laPhongVIP();
  }

  double layGiaVe(int h, int c) const {
    return (phongChieu != nullptr) ? phongChieu->layGiaGhe(h, c) : 45000.0;
  }

  bool kiemTraGheDaDat(int h, int c) const {
    return (phongChieu != nullptr) && phongChieu->kiemTraGheDaDat(h, c);
  }

  void hienThi() const {
    std::cout << "[" << maSuatChieu << "] " << tenPhim
              << " | Phòng: " << (phongChieu ? phongChieu->layTenPhong() : "N/A")
              << " | Ngày: " << TienIchNgay::dinhDangHienThi(ngayChieu)
              << " | Giờ: " << gioChieu
              << " | Đã đặt: " << demSoGheDaDat() << "/" << layTongSoGhe()
              << std::endl;
  }

  bool operator<(const SuatChieu &other) const {
    return maSuatChieu < other.maSuatChieu;
  }
  bool operator>(const SuatChieu &other) const {
    return maSuatChieu > other.maSuatChieu;
  }
  bool operator==(const SuatChieu &other) const {
    return maSuatChieu == other.maSuatChieu;
  }
  bool operator!=(const SuatChieu &other) const {
    return maSuatChieu != other.maSuatChieu;
  }
};

#endif // SUATCHIEU_H
