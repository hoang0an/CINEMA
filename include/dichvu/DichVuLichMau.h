#ifndef DICHVULICHMAU_H
#define DICHVULICHMAU_H

#include "../luutru/QuanLyFile.h"
#include "../models/CaTrucMau.h"
#include "DichVuCaTruc.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class DichVuLichMau {
private:
  std::vector<CaTrucMau> dsCaMau;
  std::string duongDanFile;

public:
  DichVuLichMau(const std::string &duongDan = "dulieu/catruc_mau.txt") : duongDanFile(duongDan) {}

  void datDuongDanFile(const std::string &duongDan) { duongDanFile = duongDan; }
  std::string layDuongDanFile() const { return duongDanFile; }

  const std::vector<CaTrucMau> &layDanhSach() const { return dsCaMau; }

  // Khởi tạo 21 ca mẫu mặc định cho 7 ngày trong tuần
  void khoiTaoMauMacDinh(const std::vector<std::string> &dsMaNV = {}) {
    dsCaMau.clear();
    std::string nv1 = (!dsMaNV.empty()) ? dsMaNV[0] : "NV01";
    std::string nv2 = (dsMaNV.size() > 1) ? dsMaNV[1] : nv1;

    for (int thu = 2; thu <= 8; thu++) {
      std::string nvSang, nvChieu, nvToi;
      if (thu % 2 == 0) {
        nvSang = nv1;
        nvChieu = nv2;
        nvToi = nv1;
      } else {
        nvSang = nv2;
        nvChieu = nv1;
        nvToi = nv2;
      }
      dsCaMau.emplace_back(thu, "Ca Sáng", "08:00", "13:30", nvSang);
      dsCaMau.emplace_back(thu, "Ca Chiều", "13:30", "18:30", nvChieu);
      dsCaMau.emplace_back(thu, "Ca Tối", "18:30", "23:30", nvToi);
    }
  }

  bool docDanhSach(const std::string &duongDan, const std::vector<std::string> &dsMaNV = {}) {
    duongDanFile = duongDan;
    dsCaMau.clear();
    std::ifstream file(duongDan);
    if (!file.is_open()) {
      khoiTaoMauMacDinh(dsMaNV);
      ghiDanhSach(duongDan);
      return true;
    }

    std::string dong;
    while (std::getline(file, dong)) {
      if (!dong.empty() && dong.back() == '\r') {
        dong.pop_back();
      }
      if (dong.empty()) continue;

      std::vector<std::string> tokens;
      std::string token;
      std::stringstream ss(dong);
      while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
      }
      if (tokens.size() < 5) continue;

      try {
        int thu = std::stoi(tokens[0]);
        std::string tenCa = tokens[1];
        std::string gioBD = tokens[2];
        std::string gioKT = tokens[3];
        std::string maNV = tokens[4];
        dsCaMau.emplace_back(thu, tenCa, gioBD, gioKT, maNV);
      } catch (...) {}
    }
    file.close();

    if (dsCaMau.empty()) {
      khoiTaoMauMacDinh(dsMaNV);
      ghiDanhSach(duongDan);
    }
    return true;
  }

  bool ghiDanhSach(const std::string &duongDan) const {
    std::ofstream file(duongDan);
    if (!file.is_open()) {
      return false;
    }

    for (const auto &c : dsCaMau) {
      file << c.layThu() << "|"
           << QuanLyFile::khuKyTuPhanCach(c.layTenCa()) << "|"
           << QuanLyFile::khuKyTuPhanCach(c.layGioBatDau()) << "|"
           << QuanLyFile::khuKyTuPhanCach(c.layGioKetThuc()) << "|"
           << QuanLyFile::khuKyTuPhanCach(c.layMaNV()) << "\n";
    }
    file.close();
    return true;
  }

  CaTrucMau *timCaMau(int thu, const std::string &tenCa) {
    for (auto &c : dsCaMau) {
      if (c.layThu() == thu && c.layTenCa() == tenCa) {
        return &c;
      }
    }
    return nullptr;
  }

  std::vector<CaTrucMau> layCaMauTheoThu(int thu) const {
    std::vector<CaTrucMau> kq;
    for (const auto &c : dsCaMau) {
      if (c.layThu() == thu) {
        kq.push_back(c);
      }
    }
    return kq;
  }

  // Cập nhật phân công nhân viên cho một ca trong Lịch tuần chuẩn
  bool capNhatPhanCong(int thu, const std::string &tenCa, const std::string &maNVMoi) {
    CaTrucMau *c = timCaMau(thu, tenCa);
    if (c == nullptr) return false;
    c->ganNhanVien(maNVMoi);
    return ghiDanhSach(duongDanFile);
  }

  // Áp dụng Lịch tuần chuẩn vào một tuần thực tế (ngayT2DauTuan là Thứ Hai YYYY-MM-DD)
  // Quy tắc Fill Gaps: Chỉ điền vào các ca chưa có, bỏ qua ca đã tồn tại để tránh xung đột
  int apDungChoTuan(const std::string &ngayT2DauTuan, DichVuCaTruc &dvCa, int &soCaBoQua) {
    int soCaTaoMoi = 0;
    soCaBoQua = 0;

    for (int thu = 2; thu <= 8; thu++) {
      int offsetNgay = thu - 2;
      std::string targetDate = TienIchNgay::congNgay(ngayT2DauTuan, offsetNgay);
      std::vector<CaTrucMau> dsCaTrongThu = layCaMauTheoThu(thu);

      for (const auto &cm : dsCaTrongThu) {
        if (!cm.coNhanVien()) {
          continue; // Slot trống trong mẫu -> bỏ qua
        }

        // Kiểm tra xem tại ngày đó rạp đã có ca trực trùng giờ chưa
        if (dvCa.kiemTraXungDotKhungGio(targetDate, cm.layGioBatDau(), cm.layGioKetThuc())) {
          soCaBoQua++;
          continue;
        }

        std::string maCaMoi;
        int kq = dvCa.xepCaMoiChiTiet(targetDate, cm.layTenCa(),
                                      cm.layGioBatDau(), cm.layGioKetThuc(),
                                      cm.layMaNV(), &maCaMoi);
        if (kq == 0) {
          soCaTaoMoi++;
        } else {
          soCaBoQua++;
        }
      }
    }
    return soCaTaoMoi;
  }

  // Lưu cách bố trí thực tế của một tuần cụ thể thành Lịch tuần chuẩn mới
  int luuTuanHienTaiLamMau(const std::string &ngayT2DauTuan, const DichVuCaTruc &dvCa) {
    int soCaCapNhat = 0;

    for (int thu = 2; thu <= 8; thu++) {
      int offsetNgay = thu - 2;
      std::string targetDate = TienIchNgay::congNgay(ngayT2DauTuan, offsetNgay);
      std::vector<CaTruc> dsCaThucTe = dvCa.layDanhSachCaTheoNgay(targetDate);

      for (const auto &caTT : dsCaThucTe) {
        CaTrucMau *cm = timCaMau(thu, caTT.layTenCa());
        if (cm != nullptr) {
          cm->ganNhanVien(caTT.layMaNVThucTe());
          soCaCapNhat++;
        }
      }
    }

    if (soCaCapNhat > 0) {
      ghiDanhSach(duongDanFile);
    }
    return soCaCapNhat;
  }

  // Đếm số ca còn trống (chưa phân công nhân viên) trong Lịch tuần chuẩn
  int demSoCaKhuyet() const {
    int dem = 0;
    for (const auto &cm : dsCaMau) {
      if (cm.layMaNV() == "TRONG" || cm.layMaNV().empty()) {
        dem++;
      }
    }
    return dem;
  }

  bool coCaKhuyet() const {
    return demSoCaKhuyet() > 0;
  }
};

#endif // DICHVULICHMAU_H
