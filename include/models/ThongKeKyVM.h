#ifndef THONGKEKYVM_H
#define THONGKEKYVM_H

#include <string>
#include <vector>

struct PhimDoanhThuVM {
  std::string tenPhim;
  int soVe;
  double doanhThu;
  double tyTrong; // % trên tổng doanh thu vé của kỳ

  PhimDoanhThuVM() : tenPhim(""), soVe(0), doanhThu(0.0), tyTrong(0.0) {}
  PhimDoanhThuVM(std::string ten, int ve, double dt, double tt)
      : tenPhim(ten), soVe(ve), doanhThu(dt), tyTrong(tt) {}
};

struct ThongKeKyVM {
  std::string tieuDeKy;       // Ví dụ: "THÁNG 11/2026", "HÔM NAY - 02/11/2026"
  std::string tuNgay;         // ISO YYYY-MM-DD
  std::string denNgay;        // ISO YYYY-MM-DD

  // 1. Doanh thu vé
  int tongSoVe;
  int tongSoHoaDon;
  double tongDoanhThuVe;

  // 2. Chi phí nhân sự & ca trực trong kỳ
  int tongSoCaTrongKy;
  int soCaDaTra;
  double tienLuongDaTra;
  int soCaChuaTra;
  double tienLuongChuaTra;
  double tongChiPhiLuong;

  // 3. Hiệu quả tài chính
  double loiNhuanRong;       // tongDoanhThuVe - tongChiPhiLuong
  double tySuatLoiNhuan;     // (loiNhuanRong / tongDoanhThuVe) * 100
  double dongTienThucTe;     // tongDoanhThuVe - tienLuongDaTra

  // 4. Chi tiết doanh thu theo từng phim trong kỳ
  std::vector<PhimDoanhThuVM> dsPhim;

  ThongKeKyVM()
      : tieuDeKy(""), tuNgay(""), denNgay(""),
        tongSoVe(0), tongSoHoaDon(0), tongDoanhThuVe(0.0),
        tongSoCaTrongKy(0), soCaDaTra(0), tienLuongDaTra(0.0),
        soCaChuaTra(0), tienLuongChuaTra(0.0), tongChiPhiLuong(0.0),
        loiNhuanRong(0.0), tySuatLoiNhuan(0.0), dongTienThucTe(0.0) {}
};

#endif // THONGKEKYVM_H
