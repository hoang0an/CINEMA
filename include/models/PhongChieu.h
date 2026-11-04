#ifndef PHONGCHIEU_H
#define PHONGCHIEU_H

#include "../cautruc/MaTran.h"
#include "../giaodien/TienIchConsole.h"
#include "Ghe.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

class PhongChieu {
private:
  std::string maPhong;
  std::string tenPhong;
  int soHang;
  int soCot;
  MaTran<Ghe> *soDoGhe;

  void khoiTaoGhe() {
    soDoGhe = new MaTran<Ghe>(soHang, soCot);
    bool laPhongVIP = (maPhong == "P05");

    for (int i = 0; i < soHang; i++) {
      char chuHang = 'A' + i;
      for (int j = 0; j < soCot; j++) {
        std::string maGhe = std::string(1, chuHang) + std::to_string(j + 1);
        std::string loai;
        double gia;

        if (laPhongVIP) {
          loai = "VIP Suite";
          gia = 180000.0;
        } else {
          // Phòng tiêu chuẩn P01 - P04: Hàng A-C (0-2) là thường, D-H (3-7) là VIP
          if (i <= 2) {
            loai = "Thường";
            gia = 45000.0;
          } else {
            loai = "VIP";
            gia = 90000.0;
          }
        }

        Ghe g(maGhe, i, j, loai, false, gia);
        soDoGhe->datGiaTri(i, j, g);
      }
    }
  }

public:
  PhongChieu(std::string maPhong, std::string tenPhong, int soHang, int soCot)
      : maPhong(maPhong), tenPhong(tenPhong), soHang(soHang), soCot(soCot),
        soDoGhe(nullptr) {
    khoiTaoGhe();
  }

  // Rule of Three: Copy Constructor
  PhongChieu(const PhongChieu &other)
      : maPhong(other.maPhong), tenPhong(other.tenPhong),
        soHang(other.soHang), soCot(other.soCot), soDoGhe(nullptr) {
    khoiTaoGhe();
    if (other.soDoGhe != nullptr) {
      for (int i = 0; i < soHang; i++) {
        for (int j = 0; j < soCot; j++) {
          soDoGhe->datGiaTri(i, j, other.soDoGhe->layGiaTri(i, j));
        }
      }
    }
  }

  // Rule of Three: Copy Assignment Operator
  PhongChieu &operator=(const PhongChieu &other) {
    if (this != &other) {
      delete soDoGhe;
      maPhong = other.maPhong;
      tenPhong = other.tenPhong;
      soHang = other.soHang;
      soCot = other.soCot;
      khoiTaoGhe();
      if (other.soDoGhe != nullptr) {
        for (int i = 0; i < soHang; i++) {
          for (int j = 0; j < soCot; j++) {
            soDoGhe->datGiaTri(i, j, other.soDoGhe->layGiaTri(i, j));
          }
        }
      }
    }
    return *this;
  }

  ~PhongChieu() {
    delete soDoGhe;
  }

  std::string layMaPhong() const { return maPhong; }
  std::string layTenPhong() const { return tenPhong; }
  int laySoHang() const { return soHang; }
  int laySoCot() const { return soCot; }
  bool laPhongVIP() const { return maPhong == "P05"; }

  double layGiaGhe(int h, int c) const {
    if (h >= 0 && h < soHang && c >= 0 && c < soCot && soDoGhe != nullptr) {
      return soDoGhe->layGiaTri(h, c).layGiaGhe();
    }
    return laPhongVIP() ? 180000.0 : 45000.0;
  }

  int demSoGheDaDat() const {
    if (soDoGhe == nullptr) return 0;
    int dem = 0;
    for (int i = 0; i < soHang; i++) {
      for (int j = 0; j < soCot; j++) {
        if (soDoGhe->layGiaTri(i, j).kiemTraDaDat()) {
          dem++;
        }
      }
    }
    return dem;
  }

  int demSoGheTrong() const {
    return (soHang * soCot) - demSoGheDaDat();
  }

  std::string xuatChuoiTrangThaiGhe() const {
    std::string s = "";
    if (soDoGhe == nullptr) return s;
    for (int i = 0; i < soHang; i++) {
      for (int j = 0; j < soCot; j++) {
        s += (soDoGhe->layGiaTri(i, j).kiemTraDaDat() ? '1' : '0');
      }
    }
    return s;
  }

  void napChuoiTrangThaiGhe(const std::string &trangThai) {
    if (soDoGhe == nullptr) return;
    size_t idx = 0;
    for (int i = 0; i < soHang; i++) {
      for (int j = 0; j < soCot; j++) {
        if (idx < trangThai.length()) {
          Ghe g = soDoGhe->layGiaTri(i, j);
          if (trangThai[idx] == '1') {
            g.datGhe();
          } else {
            g.huyDat();
          }
          soDoGhe->datGiaTri(i, j, g);
          idx++;
        }
      }
    }
  }

  void hienThiSoDo() const {
    std::cout << "\n";
    TienIchConsole::veKhungTren();
    std::string tieuDe = "SƠ ĐỒ GHẾ - " + tenPhong;
    if (laPhongVIP()) {
      tieuDe += " (24 GHẾ VIP)";
    } else {
      tieuDe += " (48 GHẾ)";
    }
    TienIchConsole::veDongGiua(tieuDe);
    TienIchConsole::veKhungGiua();
    TienIchConsole::veDongTrong();

    // Màn hình chiếu
    TienIchConsole::veDongGiua("=== MÀN HÌNH CHIẾU PHIM ===");
    TienIchConsole::veDongTrong();

    // Header cột căn lề chuẩn
    std::string header = "   Cột: ";
    for (int j = 0; j < soCot; j++) {
      char tmp[16];
      snprintf(tmp, sizeof(tmp), " %d   ", j + 1);
      header += tmp;
    }
    TienIchConsole::veDong(header);

    // Các hàng ghế
    for (int i = 0; i < soHang; i++) {
      char chuHang = 'A' + i;
      std::string dong = "   ";
      dong += chuHang;
      dong += " ---";
      for (int j = 0; j < soCot; j++) {
        Ghe g = soDoGhe->layGiaTri(i, j);
        char gheStr[16];
        if (g.kiemTraDaDat()) {
          snprintf(gheStr, sizeof(gheStr), " [ X]");
        } else {
          snprintf(gheStr, sizeof(gheStr), " [%s]", g.layMaGhe().c_str());
        }
        dong += gheStr;
      }
      char gia[32];
      double giaGhe = soDoGhe->layGiaTri(i, 0).layGiaGhe();
      snprintf(gia, sizeof(gia), "  (%s VNĐ)", TienIchConsole::dinhDangTien(giaGhe).c_str());
      dong += gia;
      TienIchConsole::veDong(dong);
    }

    TienIchConsole::veDongTrong();
    TienIchConsole::veKhungGiua();
    TienIchConsole::veDong("[XX] = Ghế trống       [ X] = Đã đặt");
    if (laPhongVIP()) {
      TienIchConsole::veDong("Hàng A-D: VIP Suite (Đồng giá: 180.000 VNĐ)");
    } else {
      TienIchConsole::veDong("Hàng A-C: Thường (45.000 VNĐ)   Hàng D-H: VIP (90.000 VNĐ)");
    }
    TienIchConsole::veKhungDuoi();
  }

  bool kiemTraGheDaDat(int h, int c) const {
    if (h < 0 || h >= soHang || c < 0 || c >= soCot || soDoGhe == nullptr) {
      return true;
    }
    return soDoGhe->layGiaTri(h, c).kiemTraDaDat();
  }

  bool datGhe(int h, int c) {
    if (h < 0 || h >= soHang || c < 0 || c >= soCot || soDoGhe == nullptr) {
      return false;
    }
    Ghe g = soDoGhe->layGiaTri(h, c);
    if (g.kiemTraDaDat()) {
      return false;
    }
    g.datGhe();
    soDoGhe->datGiaTri(h, c, g);
    return true;
  }
};

#endif // PHONGCHIEU_H
