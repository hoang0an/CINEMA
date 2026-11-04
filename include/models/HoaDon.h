#ifndef HOADOAN_H
#define HOADOAN_H

#include "../cautruc/DanhSachLienKet.h"
#include "../giaodien/TienIchConsole.h"
#include "Ve.h"
#include <cstdio>
#include <iostream>
#include <string>

class HoaDon {
private:
  std::string maHoaDon;
  std::string ngayLap;
  std::string tenNhanVien;
  DanhSachLienKet<Ve> dsVe;
  double tongTien;

public:
  HoaDon(std::string maHoaDon, std::string ngayLap, std::string tenNhanVien)
      : maHoaDon(maHoaDon), ngayLap(ngayLap), tenNhanVien(tenNhanVien),
        tongTien(0) {}

  void themVe(const Ve &v) {
    dsVe.themCuoi(v);
    tongTien += v.layGiaVe();
  }

  std::string layMaHoaDon() const { return maHoaDon; }
  std::string layNgayLap() const { return ngayLap; }
  std::string layTenNhanVien() const { return tenNhanVien; }
  double layTongTien() const { return tongTien; }
  int laySoLuongVe() const { return dsVe.laySoLuong(); }
  const DanhSachLienKet<Ve>& layDanhSachVe() const { return dsVe; }

  void inHoaDon() {
    char dong[256];
    printf("\n");
    TienIchConsole::veKhungTren();
    TienIchConsole::veDongGiua("HÓA ĐƠN THANH TOÁN");
    TienIchConsole::veKhungGiua();
    TienIchConsole::veDongTrong();

    snprintf(dong, sizeof(dong), "Mã hóa đơn: %s", maHoaDon.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Ngày lập  : %s", ngayLap.c_str());
    TienIchConsole::veDong(dong);
    snprintf(dong, sizeof(dong), "Thu ngân  : %s", tenNhanVien.c_str());
    TienIchConsole::veDong(dong);

    TienIchConsole::veKhungGiua();
    TienIchConsole::veDong("Danh sách vé:");

    // Duyet danh sach ve va in chi tiet
    Nut<Ve> *hienTai = dsVe.layDau();
    while (hienTai != nullptr) {
      hienTai->duLieu.hienThiVe();
      hienTai = hienTai->tiepTheo;
    }

    TienIchConsole::veKhungGiua();
    TienIchConsole::veDongTrong();
    snprintf(dong, sizeof(dong), "TỔNG TIỀN : %s VNĐ",
             TienIchConsole::dinhDangTien(tongTien).c_str());
    TienIchConsole::veDong(dong);
    TienIchConsole::veDongTrong();
    TienIchConsole::veKhungDuoi();

    printf("\t\t  Cảm ơn quý khách! Chúc xem phim vui vẻ!\n");
  }
};

#endif
