#ifndef NGUOIDUNG_H
#define NGUOIDUNG_H

#include <iostream>
#include <string>

class NguoiDung {
protected:
  std::string maNguoiDung;
  std::string taiKhoan;
  std::string matKhau;
  std::string hoTen;
  std::string cccd;
  std::string soDienThoai;
  std::string diaChi;

public:
  NguoiDung(std::string ma, std::string tk, std::string mk, std::string ten,
            std::string cccdVal = "Chưa cập nhật",
            std::string sdtVal = "Chưa cập nhật",
            std::string diaChiVal = "Chưa cập nhật")
      : maNguoiDung(ma), taiKhoan(tk), matKhau(mk), hoTen(ten),
        cccd(cccdVal), soDienThoai(sdtVal), diaChi(diaChiVal) {}
  virtual ~NguoiDung() {}

  std::string layMa() const { return maNguoiDung; }

  std::string layTaiKhoan() const { return taiKhoan; }

  std::string layHoTen() const { return hoTen; }

  std::string layMatKhau() const { return matKhau; }

  std::string layCCCD() const { return cccd; }

  std::string laySoDienThoai() const { return soDienThoai; }

  std::string layDiaChi() const { return diaChi; }

  void capNhatThongTin(const std::string &c, const std::string &sdt, const std::string &dc) {
    if (!c.empty()) cccd = c;
    if (!sdt.empty()) soDienThoai = sdt;
    if (!dc.empty()) diaChi = dc;
  }

  void doiMatKhau(const std::string &mkMoi) { matKhau = mkMoi; }

  bool kiemTraDangNhap(std::string tk, std::string mk) const {
    return (taiKhoan == tk && matKhau == mk);
  }

  virtual std::string layVaiTro() const = 0;
  virtual std::string layMaVaiTro() const = 0;
};
#endif // NGUOIDUNG_H
