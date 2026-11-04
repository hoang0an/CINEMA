#ifndef NHANVIEN_H
#define NHANVIEN_H

#include "NguoiDung.h"

class NhanVien : public NguoiDung {
public:
  NhanVien(std::string ma, std::string tk, std::string mk, std::string ten,
           std::string cccdVal = "Chưa cập nhật",
           std::string sdtVal = "Chưa cập nhật",
           std::string diaChiVal = "Chưa cập nhật")
      : NguoiDung(ma, tk, mk, ten, cccdVal, sdtVal, diaChiVal) {}
  std::string layVaiTro() const override { return "Nhân viên"; }
  std::string layMaVaiTro() const override { return "NHANVIEN"; }
};

#endif // NHANVIEN_H
