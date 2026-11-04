#ifndef CATRUCMAU_H
#define CATRUCMAU_H

#include <string>

// Đại diện cho một ô phân công cố định trong Lịch Tuần Chuẩn (Weekly Shift Template)
// Thứ: 2 (Thứ Hai), 3 (Thứ Ba), ..., 7 (Thứ Bảy), 8 (Chủ Nhật)
class CaTrucMau {
private:
  int thu;                 // 2 đến 8
  std::string tenCa;       // "Ca Sáng", "Ca Chiều", "Ca Tối"
  std::string gioBatDau;   // "08:00", "13:30", "18:30"
  std::string gioKetThuc;  // "13:30", "18:30", "23:30"
  std::string maNV;        // "NV01", "NV02", "NV03", hoặc "TRONG"

public:
  CaTrucMau()
      : thu(2), tenCa(""), gioBatDau(""), gioKetThuc(""), maNV("TRONG") {}

  CaTrucMau(int t, const std::string &ten, const std::string &gBD,
            const std::string &gKT, const std::string &nv = "TRONG")
      : thu(t), tenCa(ten), gioBatDau(gBD), gioKetThuc(gKT),
        maNV(nv.empty() ? "TRONG" : nv) {}

  int layThu() const { return thu; }
  std::string layTenCa() const { return tenCa; }
  std::string layGioBatDau() const { return gioBatDau; }
  std::string layGioKetThuc() const { return gioKetThuc; }
  std::string layMaNV() const { return maNV; }

  bool coNhanVien() const { return (!maNV.empty() && maNV != "TRONG"); }

  void ganNhanVien(const std::string &nvMoi) {
    maNV = nvMoi.empty() ? "TRONG" : nvMoi;
  }

  void xoaNhanVien() {
    maNV = "TRONG";
  }
};

#endif // CATRUCMAU_H
