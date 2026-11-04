#ifndef QUANTRIVIEN_H
#define QUANTRIVIEN_H

#include "NguoiDung.h"
#include <string>

class QuanTriVien : public NguoiDung {
public:
  QuanTriVien(std::string ma, std::string tk, std::string mk, std::string ten,
              std::string cccdVal = "Hệ thống",
              std::string sdtVal = "Hệ thống",
              std::string diaChiVal = "Hệ thống")
      : NguoiDung(ma, tk, mk, ten, cccdVal, sdtVal, diaChiVal) {}
  std::string layVaiTro() const override { return "Quản trị viên"; }
  std::string layMaVaiTro() const override { return "ADMIN"; }
};

#endif // QUANTRIVIEN_H
