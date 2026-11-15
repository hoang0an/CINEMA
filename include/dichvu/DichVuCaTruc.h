#ifndef DICHVUCATRUC_H
#define DICHVUCATRUC_H

#include "../cautruc/DanhSachLienKet.h"
#include "../luutru/QuanLyFile.h"
#include "../models/CaTruc.h"
#include "../models/NguoiDung.h"
#include "../tienich/TienIchNgay.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class DichVuCaTruc {
private:
  std::vector<CaTruc> dsCaTruc;
  std::string duongDanFile;
  bool duLieuHopLe = true;
  const double DON_GIA_CA_MAC_DINH = 200000.0; // 200.000 VNĐ một ca trực 5 tiếng

public:
  DichVuCaTruc() : duongDanFile("dulieu/catruc.txt") {}

  double layDonGiaCa() const { return DON_GIA_CA_MAC_DINH; }
  const std::vector<CaTruc>& layDanhSachCa() const { return dsCaTruc; }

  bool docDanhSach(const std::string &duongDan) {
    duongDanFile = duongDan;
    const auto truoc = dsCaTruc;
    dsCaTruc.clear();
    duLieuHopLe = false;
    std::ifstream file(duongDan);
    if (!file.is_open()) {
      dsCaTruc = truoc; return false;
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
      if (tokens.size() < 7 || tokens.size() > 9) { dsCaTruc = truoc; return false; }

      std::string maCa = tokens[0];
      std::string ngay = tokens[1];
      std::string tenCa = tokens[2];
      std::string gioBD = tokens[3];
      std::string gioKT = tokens[4];
      std::string nvPhanCong = tokens[5];
      std::string nvThucTe = tokens[6];
      double doanhThu = 0; int ttLuong = 0;
      try {
        size_t n = 0;
        if (tokens.size() > 7) { doanhThu = std::stod(tokens[7], &n); if (n != tokens[7].size()) throw 0; }
        if (tokens.size() > 8) { ttLuong = std::stoi(tokens[8], &n); if (n != tokens[8].size()) throw 0; }
        if (!TienIchNgay::kiemTraDinhDangGio(gioBD) || !TienIchNgay::kiemTraDinhDangGio(gioKT) || gioBD >= gioKT) throw 0;
        std::tm t{};
        if (!std::isfinite(doanhThu) || doanhThu < 0 || ttLuong < 0 || ttLuong > 1 || !TienIchNgay::phanTichISO(ngay,t)) throw 0;
        if (maCa.empty() || layCaTheoMa(maCa)) throw 0;
      } catch (...) { std::cerr << "Invalid shift: " << maCa << "\n"; dsCaTruc = truoc; return false; }

      // Chuẩn hóa và bảo vệ tài chính: Ca trực trong tương lai chưa diễn ra nên không thể thanh toán trước
      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      if (ngay > ngayHienTai && ttLuong != 0) {
        ttLuong = 0;
      }

      dsCaTruc.emplace_back(maCa, ngay, tenCa, gioBD, gioKT, nvPhanCong, nvThucTe, doanhThu, ttLuong);
    }
    file.close();
    if (file.bad()) { dsCaTruc = truoc; return false; }
    duLieuHopLe = true;
    return true;
  }

  // Vệ sinh trạng thái tài chính của các ca tương lai (đảm bảo không ca tương lai nào bị đánh dấu đã thanh toán)
  int veSinhTrangThaiCaTuongLai() {
    const auto truoc = dsCaTruc;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    int dem = 0;
    for (auto &ca : dsCaTruc) {
      if (ca.layNgay() > ngayHienTai && ca.layTrangThaiLuong() != 0) {
        ca.datTrangThaiLuong(0);
        dem++;
      }
    }
    if (dem > 0) {
      if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return -1; }
    }
    return dem;
  }

  bool ghiDanhSach(const std::string &duongDan) const {
    if (!duLieuHopLe) return false;
    return ghiFileAnToan(duongDan, [&](std::ostream &file) {

    for (const auto &ca : dsCaTruc) {
      file << QuanLyFile::khuKyTuPhanCach(ca.layMaCa()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layNgay()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layTenCa()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layGioBatDau()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layGioKetThuc()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layMaNVPhanCong()) << "|"
           << QuanLyFile::khuKyTuPhanCach(ca.layMaNVThucTe()) << "|"
           << (long long)ca.layDoanhThu() << "|"
           << ca.layTrangThaiLuong() << "\n";
    }
    });
  }

  // Tìm ca trực theo mã ca
  const CaTruc *layCaTheoMa(const std::string &maCa) const {
    for (const auto &ca : dsCaTruc) {
      if (ca.layMaCa() == maCa) return &ca;
    }
    return nullptr;
  }

  // Kiểm tra xem trong ngày đã có ca trực nào của rạp bị chồng lấn khung giờ hay chưa (Tránh trùng ca tại rạp)
  bool kiemTraXungDotKhungGio(const std::string &ngay, const std::string &gioBD, const std::string &gioKT,
                              const std::string &maCaBoQua = "") const {
    for (const auto &ca : dsCaTruc) {
      if (ca.layNgay() == ngay) {
        if (!maCaBoQua.empty() && ca.layMaCa() == maCaBoQua) {
          continue;
        }
        if (TienIchNgay::kiemTraChongLanGio(gioBD, gioKT, ca.layGioBatDau(), ca.layGioKetThuc())) {
          return true;
        }
      }
    }
    return false;
  }

  // Kiểm tra xem nhân viên maNV có bị trùng giờ với bất kỳ ca nào khác trong ngày hay không
  bool kiemTraXungDotCa(const std::string &ngay, const std::string &gioBD, const std::string &gioKT,
                        const std::string &maNV, const std::string &maCaBoQua = "") const {
    for (const auto &ca : dsCaTruc) {
      if (ca.layNgay() == ngay) {
        if (!maCaBoQua.empty() && ca.layMaCa() == maCaBoQua) {
          continue;
        }
        // Kiểm tra xem nhân viên này có đang đứng quầy thực tế ở ca kia không
        if (ca.layMaNVThucTe() == maNV) {
          if (TienIchNgay::kiemTraChongLanGio(gioBD, gioKT, ca.layGioBatDau(), ca.layGioKetThuc())) {
            return true;
          }
        }
      }
    }
    return false;
  }

  // Đổi người trực thay (Thế ca) với kiểm tra an toàn:
  // Mã kết quả: 0: Thành công, 1: Không tìm thấy ca, 2: Ca đã thanh toán lương, 3: Nhân viên bị trùng ca
  int doiNguoiLamThayChiTiet(const std::string &maCa, const std::string &maNVThucTe) {
    const auto truoc = dsCaTruc;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    for (auto &ca : dsCaTruc) {
      if (ca.layMaCa() == maCa) {
        // Chỉ áp dụng khóa tài chính cho ca đã hoặc đang làm việc trong quá khứ/hôm nay
        if (ca.layNgay() <= ngayHienTai && ca.daThanhToan()) {
          return 2; // Ca đã thanh toán lương, cấm sửa đổi
        }
        if (kiemTraXungDotCa(ca.layNgay(), ca.layGioBatDau(), ca.layGioKetThuc(), maNVThucTe, maCa)) {
          return 3; // Nhân viên làm thay đã có ca trùng giờ trong ngày
        }
        ca.ganNguoiLamThay(maNVThucTe);
        if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return -1; }
        return 0; // Thành công
      }
    }
    return 1; // Không tìm thấy ca
  }

  // Xóa ca trực khỏi hệ thống:
  // Mã kết quả: 0: Thành công, 1: Không tìm thấy ca, 2: Ca đã thanh toán lương (cấm xóa), 3: Ca đã có doanh thu bán vé
  int xoaCaTruc(const std::string &maCa, bool buocXoaNeuCoDoanhThu = false) {
    const auto truoc = dsCaTruc;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    for (auto it = dsCaTruc.begin(); it != dsCaTruc.end(); ++it) {
      if (it->layMaCa() == maCa) {
        // Chỉ áp dụng khóa tài chính đối với các ca đã hoặc đang diễn ra (<= ngày hiện tại)
        // Ca trong tương lai là ca dự kiến, nhân viên chưa làm việc nên được phép xóa tự do
        if (it->layNgay() <= ngayHienTai) {
          if (it->daThanhToan()) {
            return 2; // Không được phép xóa ca đã thanh toán lương
          }
          if (it->layDoanhThu() > 0 && !buocXoaNeuCoDoanhThu) {
            return 3; // Ca đã có doanh thu bán vé thực tế
          }
        }
        dsCaTruc.erase(it);
        if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return -1; }
        return 0; // Thành công
      }
    }
    return 1; // Không tìm thấy ca
  }

  const CaTruc *timCaDangLam(const std::string &maNV, const std::string &ngay,
                            const std::string &gio) const {
    for (const auto &ca : dsCaTruc)
      if (ca.layNgay() == ngay && ca.layMaNVThucTe() == maNV &&
          ca.layGioBatDau() <= gio && gio < ca.layGioKetThuc()) return &ca;
    return nullptr;
  }

  bool coThamChieuNhanVien(const std::string &ma) const {
    if (!duLieuHopLe) return true;
    for (const auto &ca : dsCaTruc)
      if (ca.layMaNVPhanCong() == ma || ca.layMaNVThucTe() == ma) return true;
    return false;
  }

  bool doiSoatDoanhThu(const std::string &maCa, double tongTien) {
    if (!std::isfinite(tongTien) || tongTien < 0) return false;
    for (auto &ca : dsCaTruc) if (ca.layMaCa() == maCa) {
      const auto cu = ca;
      if (ca.layDoanhThu() == tongTien) return true;
      ca.capNhatDoanhThu(tongTien);
      if (ghiDanhSach(duongDanFile)) return true;
      ca = cu; return false;
    }
    return false;
  }

  // Tính lương cho một nhân viên cụ thể (mặc định chỉ tính đến ngày hiện tại)
  void tinhLuongNhanVien(const std::string &maNV,
                         int &soCaChuaTra, double &tienLuongChuaTra,
                         int &soCaDaTra, double &tienLuongDaTra,
                         const std::string &ngayMoc = "") const {
    std::string moc = ngayMoc.empty() ? TienIchNgay::layNgayHienTaiISO() : ngayMoc;
    soCaChuaTra = 0;
    soCaDaTra = 0;

    for (const auto &ca : dsCaTruc) {
      if (ca.layMaNVThucTe() == maNV) {
        // Nguyên tắc tài chính: Chỉ tính công lương cho ca đã hoặc đang làm (<= ngày mốc)
        // Ca trong tương lai là ca dự kiến, chưa thể phát sinh lương
        if (ca.layNgay() > moc) {
          continue;
        }
        if (ca.layTrangThaiLuong() == 0) {
          soCaChuaTra++;
        } else {
          soCaDaTra++;
        }
      }
    }
    tienLuongChuaTra = soCaChuaTra * DON_GIA_CA_MAC_DINH;
    tienLuongDaTra = soCaDaTra * DON_GIA_CA_MAC_DINH;
  }

  // Thanh toán lương cho nhân viên: chuyển các ca chưa trả đến ngày mốc thành đã trả
  int thanhToanLuongNhanVien(const std::string &maNV, const std::string &ngayMoc = "") {
    const auto truoc = dsCaTruc;
    std::string moc = ngayMoc.empty() ? TienIchNgay::layNgayHienTaiISO() : ngayMoc;
    int dem = 0;
    for (auto &ca : dsCaTruc) {
      // Chỉ quyết toán các ca đến ngày mốc (quá khứ và hiện tại)
      if (ca.layMaNVThucTe() == maNV && ca.layNgay() <= moc && ca.layTrangThaiLuong() == 0) {
        ca.danhDauDaThanhToan();
        dem++;
      }
    }
    if (dem > 0) {
      if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return -1; }
    }
    return dem;
  }

  // Lấy lịch sử ca trực của một nhân viên (sắp xếp từ mới nhất đến cũ nhất, chỉ đến ngày mốc)
  std::vector<CaTruc> layLichSuCaNhanVien(const std::string &maNV, bool chiChuaTra = false, const std::string &ngayMoc = "") const {
    std::string moc = ngayMoc.empty() ? TienIchNgay::layNgayHienTaiISO() : ngayMoc;
    std::vector<CaTruc> ketQua;
    for (auto it = dsCaTruc.rbegin(); it != dsCaTruc.rend(); ++it) {
      if (it->layMaNVThucTe() == maNV) {
        // Lịch sử ca trực và đối soát lương chỉ xét đến ngày mốc
        if (it->layNgay() > moc) {
          continue;
        }
        if (!chiChuaTra || it->layTrangThaiLuong() == 0) {
          ketQua.push_back(*it);
        }
      }
    }
    return ketQua;
  }

  // Lấy danh sách ca trực theo ngày
  std::vector<CaTruc> layDanhSachCaTheoNgay(const std::string &ngay) const {
    std::vector<CaTruc> ketQua;
    for (const auto &ca : dsCaTruc) {
      if (ca.layNgay() == ngay) {
        ketQua.push_back(ca);
      }
    }
    return ketQua;
  }

  // Xếp ca mới với kiểm tra xung đột:
  // Mã kết quả: 0: Thành công, 1: Nhân viên đã có ca trùng giờ, 2: Giờ không hợp lệ, 3: Khung giờ của rạp đã có ca trực
  int xepCaMoiChiTiet(const std::string &ngay, const std::string &tenCa,
                      const std::string &gioBD, const std::string &gioKT,
                      const std::string &maNV, std::string *maCaTaoMoi = nullptr) {
    const auto truoc = dsCaTruc;
    if (!TienIchNgay::kiemTraDinhDangGio(gioBD) || !TienIchNgay::kiemTraDinhDangGio(gioKT)) {
      return 2;
    }
    if (TienIchNgay::chuyenGioThanhPhut(gioBD) >= TienIchNgay::chuyenGioThanhPhut(gioKT)) {
      return 2;
    }
    if (kiemTraXungDotKhungGio(ngay, gioBD, gioKT)) {
      return 3; // Rạp đã có ca trực trong khung giờ này
    }
    if (kiemTraXungDotCa(ngay, gioBD, gioKT, maNV)) {
      return 1; // Xung đột giờ của nhân viên
    }

    std::tm parsed{};
    if (!TienIchNgay::phanTichISO(ngay, parsed) || maNV.empty() || maNV == "TRONG") return 2;
    std::string prefix = "CA" + ngay.substr(2, 2) + ngay.substr(5, 2) + ngay.substr(8, 2);
    int demCa = 1;
    for (const auto &c : dsCaTruc) {
      if (c.layNgay() == ngay) demCa++;
    }
    std::string stt = (demCa < 10) ? ("0" + std::to_string(demCa)) : std::to_string(demCa);
    std::string maCa = prefix + "-" + stt;

    // Đảm bảo không trùng mã ca nếu có ca trước đó bị xóa
    bool daTonTai = true;
    while (daTonTai) {
      daTonTai = false;
      for (const auto &c : dsCaTruc) {
        if (c.layMaCa() == maCa) {
          demCa++;
          stt = (demCa < 10) ? ("0" + std::to_string(demCa)) : std::to_string(demCa);
          maCa = prefix + "-" + stt;
          daTonTai = true;
          break;
        }
      }
    }

    dsCaTruc.emplace_back(maCa, ngay, tenCa, gioBD, gioKT, maNV, maNV, 0.0, 0);
    if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return -1; }
    if (maCaTaoMoi != nullptr) {
      *maCaTaoMoi = maCa;
    }
    return 0;
  }

  // Xếp ca mới
  bool xepCaMoi(const std::string &ngay, const std::string &tenCa,
                const std::string &gioBD, const std::string &gioKT,
                const std::string &maNV) {
    return (xepCaMoiChiTiet(ngay, tenCa, gioBD, gioKT, maNV) == 0);
  }

  // Đồng bộ một ca trực cụ thể theo lịch chuẩn
  bool dongBoCaTheoLichMau(const std::string &ngay, const std::string &tenCa,
                           const std::string &gioBD, const std::string &gioKT,
                           const std::string &maNV) {
    const auto truoc = dsCaTruc;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    for (auto &ca : dsCaTruc) {
      if (ca.layNgay() == ngay && (ca.layTenCa() == tenCa || ca.layGioBatDau() == gioBD)) {
        // Không đổi nhầm một ca tùy chỉnh chỉ vì trùng tên hoặc giờ bắt đầu.
        if (ca.layTenCa() != tenCa || ca.layGioBatDau() != gioBD || ca.layGioKetThuc() != gioKT) return false;
        if (ca.layNgay() <= ngayHienTai && ca.daThanhToan()) {
          return false;
        }
        if (ca.coLamThay()) {
          ca.ganNguoiPhanCong(maNV);
        } else {
          ca.capNhatNhanVienMoi(maNV);
        }
        if (!ghiDanhSach(duongDanFile)) { dsCaTruc = truoc; return false; }
        return true;
      }
    }

    if (maNV != "TRONG" && !maNV.empty()) {
      return xepCaMoiChiTiet(ngay, tenCa, gioBD, gioKT, maNV) == 0;
    }
    return true;
  }

  // Đếm số ca khuyết thực tế (thiếu ca hoặc chưa phân công) từ ngayBatDau đến hết tuần hiện tại / tuần kế tiếp
  int demSoCaKhuyetThucTe(const std::string &ngayBatDau) const {
    std::string t2Nay = TienIchNgay::layNgayThuHaiCuaTuan(ngayBatDau);
    std::string cnNay = TienIchNgay::congNgay(t2Nay, 6);
    std::string t2Toi = TienIchNgay::congNgay(t2Nay, 7);
    std::string cnToi = TienIchNgay::congNgay(t2Toi, 6);

    bool tuanSauCoCa = false;
    for (const auto &ca : dsCaTruc) {
      if (ca.layNgay() >= t2Toi && ca.layNgay() <= cnToi) {
        tuanSauCoCa = true;
        break;
      }
    }
    std::string ngayKetThucQuet = tuanSauCoCa ? cnToi : cnNay;

    int demKhuyet = 0;
    const char *gioCa[3] = {"08:00", "13:30", "18:30"};
    const char *tenCa[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};

    std::string curr = ngayBatDau;
    while (curr <= ngayKetThucQuet) {
      std::vector<CaTruc> dsNgay = layDanhSachCaTheoNgay(curr);
      for (int c = 0; c < 3; c++) {
        bool timThay = false;
        for (const auto &ca : dsNgay) {
          if (ca.layTenCa() == tenCa[c] || ca.layGioBatDau() == gioCa[c]) {
            timThay = true;
            if (ca.layMaNVThucTe() == "TRONG" || ca.layMaNVThucTe().empty()) {
              demKhuyet++;
            }
            break;
          }
        }
        if (!timThay) {
          demKhuyet++;
        }
      }
      curr = TienIchNgay::congNgay(curr, 1);
    }
    return demKhuyet;
  }

  bool coCaKhuyetThucTe(const std::string &ngayBatDau) const {
    return demSoCaKhuyetThucTe(ngayBatDau) > 0;
  }

  // Tìm ngày sớm nhất bị khuyết ca để hỗ trợ tính năng nhảy nhanh [F]
  std::string timNgayKhuyetDauTien(const std::string &ngayBatDau) const {
    std::string t2Nay = TienIchNgay::layNgayThuHaiCuaTuan(ngayBatDau);
    std::string cnNay = TienIchNgay::congNgay(t2Nay, 6);
    std::string t2Toi = TienIchNgay::congNgay(t2Nay, 7);
    std::string cnToi = TienIchNgay::congNgay(t2Toi, 6);

    bool tuanSauCoCa = false;
    for (const auto &ca : dsCaTruc) {
      if (ca.layNgay() >= t2Toi && ca.layNgay() <= cnToi) {
        tuanSauCoCa = true;
        break;
      }
    }
    std::string ngayKetThucQuet = tuanSauCoCa ? cnToi : cnNay;

    const char *gioCa[3] = {"08:00", "13:30", "18:30"};
    const char *tenCa[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};

    std::string curr = ngayBatDau;
    while (curr <= ngayKetThucQuet) {
      std::vector<CaTruc> dsNgay = layDanhSachCaTheoNgay(curr);
      for (int c = 0; c < 3; c++) {
        bool timThay = false;
        for (const auto &ca : dsNgay) {
          if (ca.layTenCa() == tenCa[c] || ca.layGioBatDau() == gioCa[c]) {
            timThay = true;
            if (ca.layMaNVThucTe() == "TRONG" || ca.layMaNVThucTe().empty()) {
              return curr;
            }
            break;
          }
        }
        if (!timThay) {
          return curr;
        }
      }
      curr = TienIchNgay::congNgay(curr, 1);
    }
    return "";
  }
};

#endif // DICHVUCATRUC_H
