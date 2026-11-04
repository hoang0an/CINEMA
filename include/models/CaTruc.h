#ifndef CATRUC_H
#define CATRUC_H

#include <string>

class CaTruc {
private:
  std::string maCa;
  std::string ngay;
  std::string tenCa;
  std::string gioBatDau;
  std::string gioKetThuc;
  std::string maNVPhanCong;
  std::string maNVThucTe;
  double doanhThu;
  int trangThaiLuong; // 0: Chưa thanh toán, 1: Đã thanh toán

public:
  CaTruc()
      : maCa(""), ngay(""), tenCa(""), gioBatDau(""), gioKetThuc(""),
        maNVPhanCong(""), maNVThucTe(""), doanhThu(0.0), trangThaiLuong(0) {}

  CaTruc(std::string ma, std::string ng, std::string ten,
         std::string gioBD, std::string gioKT,
         std::string nvPhanCong, std::string nvThucTe = "",
         double dt = 0.0, int ttLuong = 0)
      : maCa(ma), ngay(ng), tenCa(ten), gioBatDau(gioBD), gioKetThuc(gioKT),
        maNVPhanCong(nvPhanCong),
        maNVThucTe(nvThucTe.empty() ? nvPhanCong : nvThucTe),
        doanhThu(dt), trangThaiLuong(ttLuong) {}

  std::string layMaCa() const { return maCa; }
  std::string layNgay() const { return ngay; }
  std::string layTenCa() const { return tenCa; }
  std::string layGioBatDau() const { return gioBatDau; }
  std::string layGioKetThuc() const { return gioKetThuc; }
  std::string layMaNVPhanCong() const { return maNVPhanCong; }
  std::string layMaNVThucTe() const { return maNVThucTe; }
  double layDoanhThu() const { return doanhThu; }
  int layTrangThaiLuong() const { return trangThaiLuong; }

  bool daThanhToan() const { return trangThaiLuong == 1; }
  bool coLamThay() const {
    return (!maNVPhanCong.empty() && !maNVThucTe.empty() && maNVPhanCong != maNVThucTe);
  }

  void ganNguoiLamThay(const std::string &maNVMoi) {
    maNVThucTe = maNVMoi;
  }

  void ganNguoiPhanCong(const std::string &maNVMoi) {
    maNVPhanCong = maNVMoi;
  }

  void capNhatNhanVienMoi(const std::string &maNVMoi) {
    maNVPhanCong = maNVMoi;
    maNVThucTe = maNVMoi;
  }

  void capNhatDoanhThu(double dtMoi) {
    doanhThu = dtMoi;
  }

  void danhDauDaThanhToan() {
    trangThaiLuong = 1;
  }

  void datTrangThaiLuong(int tt) {
    trangThaiLuong = tt;
  }
};

#endif // CATRUC_H
