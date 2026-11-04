#ifndef GHE_H
#define GHE_H

#include <iostream>
#include <ostream>
#include <string>

class Ghe {
private:
  std::string maGhe;
  int hang;
  int cot;
  std::string loaiGhe;
  bool daDat;
  double giaGhe;

public:
  Ghe() : hang(0), cot(0), loaiGhe("Thường"), daDat(false), giaGhe(45000) {}
  Ghe(std::string ma, int h, int c, std::string loai, bool daDat, double gia)
      : maGhe(ma), hang(h), cot(c), loaiGhe(loai), daDat(daDat), giaGhe(gia) {}

  std::string layMaGhe() const { return maGhe; }
  int layHang() const { return hang; }
  int layCot() const { return cot; }
  std::string layLoaiGhe() const { return loaiGhe; }
  bool kiemTraDaDat() const { return daDat; }
  double layGiaGhe() const { return giaGhe; }

  void datGhe() { daDat = true; }
  void huyDat() { daDat = false; }

  friend std::ostream &operator<<(std::ostream &os, const Ghe &g) {
    if (g.daDat) {
      os << "[ X ]";
    } else {
      os << "[" << g.maGhe << " ] ";
    }
    return os;
  }
};

#endif