#ifndef DICHVUPHIM_H
#define DICHVUPHIM_H

#include "../cautruc/DanhSachLienKet.h"
#include "../giaodien/TienIchConsole.h"
#include "../luutru/QuanLyFile.h"
#include "../models/Phim.h"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <functional>
#include <iostream>

class DichVuPhim {
public:
  static const int GIOI_HAN_DANG_CHIEU = 5;
  static const int GIOI_HAN_SAP_CHIEU = 5;

private:
  DanhSachLienKet<Phim> dsPhim;

  void hoanDoi(Phim &a, Phim &b) {
    Phim temp = a;
    a = b;
    b = temp;
  }
  
  int phanHoach(Phim arr[], int thap, int cao) {
    int chot = arr[cao].layThoiLuong();
    int i = thap - 1;
    for (int j = thap; j < cao; j++) {
      if (arr[j].layThoiLuong() < chot) {
        i++;
        hoanDoi(arr[j], arr[i]);
      }
    }
    hoanDoi(arr[i + 1], arr[cao]);
    return i + 1;
  }

  void quickSort(Phim arr[], int thap, int cao) {
    if (thap < cao) {
      int pi = phanHoach(arr, thap, cao);
      quickSort(arr, thap, pi - 1);
      quickSort(arr, pi + 1, cao);
    }
  }

public:
  // Thuat toan tu dong sinh ma phim 6 chu so theo chuan YYXXXX (2 so nam + 4 so thu tu)
  // Vi du: nam 2026 -> bat dau tu 260001 den 269999 (dung luong 10.000 phim/nam)
  std::string sinhMaPhimTuDong() const {
    time_t bayGio = time(nullptr);
    tm *thoiGian = localtime(&bayGio);
    int namHienTai = (thoiGian != nullptr) ? ((thoiGian->tm_year + 1900) % 100) : 26;

    char prefix[4];
    snprintf(prefix, sizeof(prefix), "%02d", namHienTai);
    std::string strPrefix = prefix;

    int maxSeq = 0;
    Nut<Phim> *hienTai = dsPhim.layDau();
    while (hienTai != nullptr) {
      std::string ma = hienTai->duLieu.layMaPhim();
      // Kiem tra ma co dung 6 ky tu va bat dau bang nam hien tai (vd "260001")
      if (ma.length() == 6 && ma.substr(0, 2) == strPrefix) {
        try {
          int seq = std::stoi(ma.substr(2));
          if (seq > maxSeq) {
            maxSeq = seq;
          }
        } catch (...) {}
      }
      hienTai = hienTai->tiepTheo;
    }

    int nextSeq = maxSeq + 1;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d%04d", namHienTai, nextSeq);
    return std::string(buf);
  }

  Phim *timPhimTheoMa(const std::string &maPhim) {
    Nut<Phim> *hienTai = dsPhim.layDau();

    while (hienTai != nullptr) {
      if (hienTai->duLieu.layMaPhim() == maPhim) {
        return &(hienTai->duLieu);
      }
      hienTai = hienTai->tiepTheo;
    }
    return nullptr;
  }

  // Đếm số lượng phim theo từng trạng thái (1: Đang chiếu, 2: Sắp chiếu, 0: Ngừng chiếu)
  int demSoPhimTheoTrangThai(int tt) const {
    int dem = 0;
    Nut<Phim> *cur = dsPhim.layDau();
    while (cur != nullptr) {
      if (cur->duLieu.layTrangThai() == tt) {
        dem++;
      }
      cur = cur->tiepTheo;
    }
    return dem;
  }

  std::string layCanhBaoQuota() const {
    const int dang=demSoPhimTheoTrangThai(1), sap=demSoPhimTheoTrangThai(2);
    if (dang<=GIOI_HAN_DANG_CHIEU && sap<=GIOI_HAN_SAP_CHIEU) return "";
    return "CẢNH BÁO QUOTA: Đang chiếu " + std::to_string(dang) + "/" +
           std::to_string(GIOI_HAN_DANG_CHIEU) + "; Sắp chiếu " + std::to_string(sap) + "/" +
           std::to_string(GIOI_HAN_SAP_CHIEU);
  }

  int demPhimDangChieuDuKien(const Phim &phimMoi) const {
    const auto ngay=phimMoi.layNgayKhoiChieu();
    int count=1; // Phim đang lập đợt sẽ khởi chiếu vào ngày này.
    for (auto n=dsPhim.layDau();n;n=n->tiepTheo) {
      const auto &p=n->duLieu;
      if (p.layMaPhim()!=phimMoi.layMaPhim() && p.layTrangThai()!=0 &&
          p.tinhTrangThaiTheoNgay(ngay)==1) ++count;
    }
    return count;
  }

  // Kiểm soát hạn ngạch tối đa số lượng phim đang chiếu và sắp chiếu tại cụm rạp
  bool kiemTraGioiHanTrangThai(int trangThaiMoi, std::string &thongBaoLoi,
                               const std::string &maPhimHienTai = "") const {
    if (trangThaiMoi == 0) {
      return true; // Luôn cho phép chuyển về Ngừng chiếu để giải phóng hạn ngạch
    }

    if (!maPhimHienTai.empty()) {
      Nut<Phim> *cur = dsPhim.layDau();
      while (cur != nullptr) {
        if (cur->duLieu.layMaPhim() == maPhimHienTai) {
          if (cur->duLieu.layTrangThai() == trangThaiMoi) {
            return true; // Giữ nguyên trạng thái hiện tại
          }
          break;
        }
        cur = cur->tiepTheo;
      }
    }

    if (trangThaiMoi == 1) {
      int soHienTai = demSoPhimTheoTrangThai(1);
      if (soHienTai >= GIOI_HAN_DANG_CHIEU) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "Đã đạt giới hạn tối đa %d phim ĐANG CHIẾU! Vui lòng chuyển bớt phim sang 'Ngừng chiếu' trước.",
                 GIOI_HAN_DANG_CHIEU);
        thongBaoLoi = std::string(buf);
        return false;
      }
    } else if (trangThaiMoi == 2) {
      int soHienTai = demSoPhimTheoTrangThai(2);
      if (soHienTai >= GIOI_HAN_SAP_CHIEU) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "Đã đạt giới hạn tối đa %d phim SẮP CHIẾU! Vui lòng chuyển bớt phim cũ trước.",
                 GIOI_HAN_SAP_CHIEU);
        thongBaoLoi = std::string(buf);
        return false;
      }
    }
    return true;
  }

  bool themPhim(const Phim &p) {
    if (timPhimTheoMa(p.layMaPhim()) != nullptr) {
      return false;
    }
    // Lớp bảo vệ thứ 2: Xác thực tính có nghĩa theo bộ 4 tiêu chí hình thái học điện ảnh
    if (!QuanLyFile::kiemTraTenPhimHopLe(p.layTenPhim()) ||
        !QuanLyFile::kiemTraTheLoaiHopLe(p.layTheLoai()) ||
        !QuanLyFile::kiemTraDaoDienHopLe(p.layDaoDien())) {
      return false;
    }
    // Lớp bảo vệ thứ 3: Kiểm soát trần hạn ngạch số lượng phim Đang chiếu / Sắp chiếu
    std::string loiGioiHan;
    if (!kiemTraGioiHanTrangThai(p.layTrangThai(), loiGioiHan)) {
      return false;
    }
    dsPhim.themCuoi(p);
    return true;
  }

  std::function<bool(const std::string&)> coLichTuongLai;
  std::function<bool(const std::string&, const std::string&)> coLichSauNgay;
  std::function<void(const std::string&, const std::string&)> onDoiTenPhim;

  bool xoaPhim(const std::string &maPhim) {
    if (!coLichTuongLai || coLichTuongLai(maPhim)) return false;
    Phim mau(maPhim, "", "", "", 0);
    if (timPhimTheoMa(maPhim) == nullptr) {
      return false;
    } else {
      return dsPhim.xoaTheoGiaTri(mau);
    }
  }

  bool suaThongTinPhim(const std::string &ma, const std::string &ten,
                        const std::string &theLoai, const std::string &daoDien, int thoiLuong) {
    Phim *p = timPhimTheoMa(ma);
    if (!p || !QuanLyFile::kiemTraTenPhimHopLe(ten) ||
        !QuanLyFile::kiemTraTheLoaiHopLe(theLoai) || !QuanLyFile::kiemTraDaoDienHopLe(daoDien) ||
        thoiLuong < 1 || thoiLuong > 600 || ten.size()>180 || theLoai.size()>120 || daoDien.size()>100) return false;
    if (thoiLuong != p->layThoiLuong() && (!coLichTuongLai || coLichTuongLai(ma))) return false;
    std::string tenCu = p->layTenPhim();
    *p = Phim(ma,ten,theLoai,daoDien,thoiLuong,p->layTrangThai(),p->layNgayKhoiChieu(),p->laySoNgayChieu());
    if (ten != tenCu && onDoiTenPhim) {
      onDoiTenPhim(ma, ten);
    }
    return true;
  }

  bool suaMotPhanThongTinPhim(const std::string &ma, const std::string &ten,
                             const std::string &loai, const std::string &dd,
                             const std::string &chuoiPhut, const std::string &ngayHetDot = "") {
    Phim *p = timPhimTheoMa(ma); if (!p) return false;
    const auto t = QuanLyFile::trim(ten), l = QuanLyFile::trim(loai);
    const auto d = QuanLyFile::trim(dd), ph = QuanLyFile::trim(chuoiPhut);
    int phut = p->layThoiLuong();
    if (!ph.empty()) {
      try { size_t n=0; phut=std::stoi(ph,&n); if(n!=ph.size()) return false; }
      catch (...) { return false; }
    }
    Phim duKien=*p;
    const auto ngay=QuanLyFile::trim(ngayHetDot);
    if (!ngay.empty()) {
      if (!duKien.setNgayKetThuc(ngay)) return false;
      if (duKien.layTrangThai()!=0 && duKien.layNgayKetThuc()<TienIchNgay::layNgayHienTaiISO()) return false;
      if (duKien.layNgayKetThuc()<p->layNgayKetThuc() &&
          (!coLichSauNgay || coLichSauNgay(ma,duKien.layNgayKetThuc()))) return false;
    }
    if (!suaThongTinPhim(ma,t.empty()?p->layTenPhim():t,
                        l.empty()?p->layTheLoai():l,d.empty()?p->layDaoDien():d,phut)) return false;
    p->setSoNgayChieu(duKien.laySoNgayChieu());
    return true;
  }

  static bool lapDotChieuMoi(const Phim &cu, int trangThai, const std::string &ngay,
                            int soNgay, Phim &duKien) {
    if (trangThai!=2 || soNgay<1 || soNgay>365) return false;
    const auto homNay=TienIchNgay::layNgayHienTaiISO();
    const auto batDau=QuanLyFile::trim(ngay);
    std::tm t{};
    if (!TienIchNgay::phanTichISO(batDau,t) || (trangThai==2 && batDau<=homNay)) return false;
    Phim moi=cu; moi.setNgayKhoiChieu(batDau); moi.setSoNgayChieu(soNgay); moi.setTrangThai(trangThai);
    duKien=moi; return true;
  }

  // Thay doi trang thai cua phim: 1 = Dang chieu, 2 = Sap chieu, 0 = Ngung chieu
  bool doiTrangThaiPhim(const std::string &maPhim, int trangThaiMoi) {
    Phim *p = timPhimTheoMa(maPhim);
    if (p == nullptr) {
      return false;
    }
    if (trangThaiMoi!=0 && trangThaiMoi!=2) return false; // Đang chiếu chỉ chuyển tự động theo ngày.
    if (trangThaiMoi != 0 && p->tinhTrangThaiTheoNgay(TienIchNgay::layNgayHienTaiISO()) != trangThaiMoi) return false;
    std::string loi;
    if (!kiemTraGioiHanTrangThai(trangThaiMoi, loi, maPhim)) return false;
    p->setTrangThai(trangThaiMoi);
    return true;
  }

  // Chuyen chuoi sang chu thuong de tim kiem khong phan biet hoa thuong
  static std::string chuyenChuThuong(const std::string &s) {
    std::string res = s;
    for (size_t i = 0; i < res.length(); i++) {
      res[i] = (char)tolower((unsigned char)res[i]);
    }
    return res;
  }

  // Tim kiem phim theo tu khoa tren ca Ma, Ten, The loai va Dao dien
  DanhSachLienKet<Phim> timKiemPhim(const std::string &tuKhoa) const {
    DanhSachLienKet<Phim> ketQua;
    std::string tuKhoaLower = chuyenChuThuong(tuKhoa);

    Nut<Phim> *hienTai = dsPhim.layDau();
    while (hienTai != nullptr) {
      std::string maLower = chuyenChuThuong(hienTai->duLieu.layMaPhim());
      std::string tenLower = chuyenChuThuong(hienTai->duLieu.layTenPhim());
      std::string theLoaiLower = chuyenChuThuong(hienTai->duLieu.layTheLoai());
      std::string daoDienLower = chuyenChuThuong(hienTai->duLieu.layDaoDien());
      std::string ttLower = chuyenChuThuong(hienTai->duLieu.layChuoiTrangThai());

      if (maLower.find(tuKhoaLower) != std::string::npos ||
          tenLower.find(tuKhoaLower) != std::string::npos ||
          theLoaiLower.find(tuKhoaLower) != std::string::npos ||
          daoDienLower.find(tuKhoaLower) != std::string::npos ||
          ttLower.find(tuKhoaLower) != std::string::npos) {
        ketQua.themCuoi(hienTai->duLieu);
      }
      hienTai = hienTai->tiepTheo;
    }
    return ketQua;
  }

  // Hien thi danh sach phim co phan trang tuong tac (Pagination), Tim kiem (Search),
  // Doi trang thai phim (Status Change) va Xoa phim (Delete) tich hop 1 cua
  void hienThiDanhSach(int pageSize = 5, const std::string &tuKhoaKhoiTao = "", bool cheDoAdmin = true,
                       std::function<void(const std::string &, int)> onDoiTrangThai = nullptr) {
    if (dsPhim.laySoLuong() == 0) {
      TienIchConsole::xoaManHinh();
      std::cout << "\n";
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("DANH SÁCH PHIM HIỆN CÓ");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("Hiện tại hệ thống chưa có bộ phim nào!");
      TienIchConsole::veKhungDuoi();
      TienIchConsole::choPhimBatKy();
      return;
    }

    bool dangTimKiem = false;
    std::string tuKhoa = "";
    DanhSachLienKet<Phim> dsKetQua;
    int trangHienTai = 1;

    if (!tuKhoaKhoiTao.empty()) {
      dsKetQua = timKiemPhim(tuKhoaKhoiTao);
      if (dsKetQua.laySoLuong() == 0) {
        TienIchConsole::hienThiLoi("Không tìm thấy phim nào khớp với: '" + tuKhoaKhoiTao + "'!");
        Sleep(1500);
      } else {
        dangTimKiem = true;
        tuKhoa = tuKhoaKhoiTao;
      }
    }

    while (true) {
      const DanhSachLienKet<Phim> &dsHienThi = dangTimKiem ? dsKetQua : dsPhim;
      int tongPhim = dsHienThi.laySoLuong();
      int tongTrang = (tongPhim + pageSize - 1) / pageSize;
      if (tongTrang == 0) tongTrang = 1;
      if (trangHienTai > tongTrang) trangHienTai = tongTrang;

      TienIchConsole::xoaManHinh();
      std::cout << "\n";
      TienIchConsole::veKhungTren();
      char tieuDe[128];
      if (dangTimKiem) {
        snprintf(tieuDe, sizeof(tieuDe), "KẾT QUẢ TÌM: \"%s\" [%d/%d - Khớp: %d]",
                 TienIchConsole::catChuoi(tuKhoa, 15).c_str(),
                 trangHienTai, tongTrang, tongPhim);
      } else {
        snprintf(tieuDe, sizeof(tieuDe), "QUẢN LÝ DANH SÁCH PHIM [Trang %d/%d - Tổng: %d]",
                 trangHienTai, tongTrang, tongPhim);
      }
      TienIchConsole::veDongGiua(tieuDe);
      TienIchConsole::veKhungGiua();
      if (cheDoAdmin && !layCanhBaoQuota().empty()) {
        TienIchConsole::veDong(layCanhBaoQuota());
        TienIchConsole::veDong("Phim đến ngày khởi chiếu vẫn chuyển đúng trạng thái.");
        TienIchConsole::veDong("Hãy điều chỉnh trước khi thêm hoặc mở thêm phim.");
        TienIchConsole::veKhungGiua();
      }

      // Di chuyen con tro den dau trang hien tai
      Nut<Phim> *hienTai = dsHienThi.layDau();
      int boQua = (trangHienTai - 1) * pageSize;
      for (int i = 0; i < boQua && hienTai != nullptr; i++) {
        hienTai = hienTai->tiepTheo;
      }

      int demTrongTrang = 0;
      while (hienTai != nullptr && demTrongTrang < pageSize) {
        if (demTrongTrang > 0)
          TienIchConsole::veDuongKe();

        // Dong 1: Ma phim to mau Cyan, Ten phim to mau trang sang (Tu dong xuong dong 2 thong minh neu dai)
        TienIchConsole::veDongPhimTieuDe(hienTai->duLieu.layMaPhim(), hienTai->duLieu.layTenPhim());

        // Dong 2: The loai va thoi luong (cat chuoi theo ranh gioi tu, luon bao toan du thoi luong | X phut)
        int maxInside = TienIchConsole::DO_RONG - 4;
        std::string suffixThoiLuong = " | " + std::to_string(hienTai->duLieu.layThoiLuong()) + " phút";
        std::string prefixTl = "  Thể loại: ";
        int maxTl = maxInside - (TienIchConsole::doDaiHienThi(prefixTl) + TienIchConsole::doDaiHienThi(suffixThoiLuong));
        if (maxTl < 5) maxTl = 5;
        std::string tlIn = hienTai->duLieu.layTheLoai();
        if (TienIchConsole::doDaiHienThi(tlIn) > maxTl) {
          tlIn = TienIchConsole::catChuoiTheoTu(tlIn, maxTl);
        }
        TienIchConsole::veDong(prefixTl + tlIn + suffixThoiLuong);

        // Dong 3: Dao dien va badge trang thai to mau (Xanh: Dang chieu / Vang: Sap chieu / Do: Ngung chieu)
        TienIchConsole::veDongPhimDaoDienVaTrangThai(hienTai->duLieu.layDaoDien(),
                                                    hienTai->duLieu.layTrangThai(),
                                                    hienTai->duLieu.layChuoiTrangThai());
        TienIchConsole::veDong("  Khởi chiếu: " + TienIchNgay::dinhDangHienThi(hienTai->duLieu.layNgayKhoiChieu()) +
                              " | Hết đợt: " + TienIchNgay::dinhDangHienThi(hienTai->duLieu.layNgayKetThuc()));

        demTrongTrang++;
        hienTai = hienTai->tiepTheo;
      }

      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("[N]: Trang sau           [P]: Trang trước");
      if (cheDoAdmin) {
        TienIchConsole::veDong("[F]: Tìm kiếm phim       [G]: Nhảy đến trang");
        TienIchConsole::veDong("[S]: Đổi trạng thái      [D]: Xóa vĩnh viễn");
        TienIchConsole::veDong("[E]: Sửa thông tin phim");
        if (dangTimKiem) {
          TienIchConsole::veDong("[R]: Xem tất cả phim     [0]: Quay lại Menu chính");
        } else {
          TienIchConsole::veDong("[0]: Quay lại Menu chính");
        }
      } else {
        TienIchConsole::veDong("[F]: Tìm kiếm phim       [G]: Nhảy đến trang");
        if (dangTimKiem) {
          TienIchConsole::veDong("[R]: Xem tất cả phim     [0]: Quay lại Menu chính");
        } else {
          TienIchConsole::veDong("[0]: Quay lại Menu chính");
        }
      }
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để thao tác: ");
      int ch = _getch();

      // Bo qua ma phim mo rong 2 byte
      if (ch == 0 || ch == 224) {
        int extra = _getch();
        if (extra == 77) { // Mui ten phai -> Trang sau
          if (trangHienTai < tongTrang) trangHienTai++;
        } else if (extra == 75) { // Mui ten trai -> Trang truoc
          if (trangHienTai > 1) trangHienTai--;
        }
        continue;
      }

      if (ch == 'n' || ch == 'N') {
        if (trangHienTai < tongTrang) {
          trangHienTai++;
        }
      } else if (ch == 'p' || ch == 'P') {
        if (trangHienTai > 1) {
          trangHienTai--;
        }
      } else if (ch == 'f' || ch == 'F') {
        printf("\n\t\tNhập từ khóa tìm kiếm hoặc [0] / Enter để hủy: ");
        std::string kw;
        std::getline(std::cin, kw);
        size_t start = kw.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
          size_t end = kw.find_last_not_of(" \t\r\n");
          std::string trimmedKw = kw.substr(start, end - start + 1);
          if (trimmedKw == "0" || trimmedKw == "q" || trimmedKw == "Q" || trimmedKw == "huy") {
            continue;
          }

          DanhSachLienKet<Phim> kq = timKiemPhim(trimmedKw);
          if (kq.laySoLuong() == 0) {
            TienIchConsole::hienThiLoi("Không tìm thấy phim nào khớp với: '" + trimmedKw + "'!");
            Sleep(1500);
          } else {
            dsKetQua = kq;
            dangTimKiem = true;
            tuKhoa = trimmedKw;
            trangHienTai = 1;
          }
        }
      } else if ((ch == 'e' || ch == 'E') && cheDoAdmin) {
        std::string ma,ten,loai,dd,tl,hetDot;
        std::cout << "Mã phim [Enter: hủy]: "; std::getline(std::cin,ma);
        Phim *p = timPhimTheoMa(ma); if (!p) continue;
        Phim cu = *p;
        std::cout << "Nhấn Enter để giữ nguyên từng mục.\n";
        std::cout << "Tên phim [" << p->layTenPhim() << "]: "; std::getline(std::cin,ten);
        std::cout << "Thể loại [" << p->layTheLoai() << "]: "; std::getline(std::cin,loai);
        std::cout << "Đạo diễn [" << p->layDaoDien() << "]: "; std::getline(std::cin,dd);
        std::cout << "Thời lượng phút [" << p->layThoiLuong() << "]: "; std::getline(std::cin,tl);
        std::cout << "Ngày hết đợt YYYY-MM-DD hoặc DD/MM/YYYY ["
                  << TienIchNgay::dinhDangHienThi(p->layNgayKetThuc()) << "]: ";
        std::getline(std::cin,hetDot);
        if (QuanLyFile::trim(ten).empty() && QuanLyFile::trim(loai).empty() &&
            QuanLyFile::trim(dd).empty() && QuanLyFile::trim(tl).empty() && QuanLyFile::trim(hetDot).empty()) {
          TienIchConsole::hienThiThanhCong("Đã giữ nguyên thông tin phim."); Sleep(1000); continue;
        }
        if (!suaMotPhanThongTinPhim(ma,ten,loai,dd,tl,hetDot)) {
          TienIchConsole::hienThiLoi("Dữ liệu không hợp lệ hoặc thay đổi xung đột lịch đã xếp.");
          TienIchConsole::hienThiLoi("Hết đợt phải sau khởi chiếu (tối đa 365 ngày); không rút ngắn qua suất còn tồn tại.");
          TienIchConsole::choPhimBatKy(); continue;
        }
        if (!QuanLyFile::ghiDanhSachPhim("dulieu/phim.txt",dsPhim)) {
          *p=cu; TienIchConsole::hienThiLoi("Lưu thất bại!"); Sleep(1200); continue;
        }
        if (dangTimKiem) dsKetQua=timKiemPhim(tuKhoa);
        TienIchConsole::hienThiThanhCong("Đã sửa thông tin phim!"); Sleep(1000);
      } else if (ch == 's' || ch == 'S') {
        if (!cheDoAdmin) {
          TienIchConsole::hienThiLoi("Chức năng chỉ dành cho Quản trị viên!");
          Sleep(1000);
          continue;
        }
        printf("\n\t\t>> Nhập mã phim cần đổi trạng thái hoặc [0] / Enter để hủy - ví dụ 260001: ");
        std::string maP;
        std::getline(std::cin, maP);
        size_t start = maP.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
          size_t end = maP.find_last_not_of(" \t\r\n");
          std::string trimmedMa = maP.substr(start, end - start + 1);
          if (trimmedMa == "0") {
            continue; // Huy thao tac doi trang thai em ai
          }

          Phim *p = timPhimTheoMa(trimmedMa);
          if (p == nullptr) {
            TienIchConsole::hienThiLoi("Không tìm thấy mã phim '" + trimmedMa + "'!");
            Sleep(1200);
          } else {
            std::cout << "\n\t\tPhim: ";
            TienIchConsole::inChuCoMau("[" + p->layMaPhim() + "] ", TienIchConsole::MAU_MA_PHIM);
            std::cout << p->layTenPhim() << " | Trạng thái hiện tại: ";
            TienIchConsole::inChuCoMau("[" + p->layChuoiTrangThai() + "]\n",
                                       TienIchConsole::layMauTheoTrangThai(p->layTrangThai()));

            std::cout << "\t\tChọn trạng thái mới:\n";
            std::cout << "\t\tĐang chiếu: tự động khi đến ngày khởi chiếu.\n";
            std::cout << "\t\t  1: ";
            TienIchConsole::inChuCoMau("Sắp chiếu\n", TienIchConsole::MAU_SAP_CHIEU);
            std::cout << "\t\t  0: ";
            TienIchConsole::inChuCoMau("Ngừng chiếu\n", TienIchConsole::MAU_NGUNG_CHIEU);
            std::cout << "\t\t  [Enter] hoặc [9]: Giữ nguyên và hủy bỏ\n";
            std::cout << "\t\t>> Lựa chọn của bạn: ";

            std::string sChonTt;
            std::getline(std::cin, sChonTt);
            sChonTt = TienIchConsole::catKhoangTrang(sChonTt);
            if (sChonTt.empty() || sChonTt == "9" || sChonTt == "q" || sChonTt == "Q" || sChonTt == "huy") {
              TienIchConsole::hienThiThanhCong("Đã giữ nguyên trạng thái cũ của phim.");
              Sleep(800);
              continue;
            }

            int chonTt = -1;
            if (sChonTt=="1") chonTt=2; // Số menu khác mã trạng thái nội bộ.
            else if (sChonTt=="0") chonTt=0;
            if (chonTt == 0 || chonTt == 2) {
              std::string loiGioiHan;
              if (!kiemTraGioiHanTrangThai(chonTt, loiGioiHan, p->layMaPhim())) {
                TienIchConsole::hienThiLoi(loiGioiHan);
                Sleep(2000);
              } else {
                Phim truoc = *p;
                Phim duKien = *p;
                if (chonTt==2) {
                  std::string ngayMoi,soNgay;
                  std::cout << "LÊN LỊCH SẮP CHIẾU — Ngày khởi chiếu phải sau hôm nay.\n";
                  std::cout << "Ngày khởi chiếu YYYY-MM-DD [Enter: hủy]: ";
                  std::getline(std::cin,ngayMoi);ngayMoi=QuanLyFile::trim(ngayMoi);
                  if (ngayMoi.empty()) {
                    TienIchConsole::hienThiThanhCong("Đã hủy đổi trạng thái."); Sleep(800); continue;
                  }
                  std::cout << "Thời hạn chiếu 1-365 ngày [Enter: hủy]: ";std::getline(std::cin,soNgay);
                  soNgay=QuanLyFile::trim(soNgay);
                  if (soNgay.empty()) {
                    TienIchConsole::hienThiThanhCong("Đã hủy đổi trạng thái."); Sleep(800); continue;
                  }
                  int sn=0;try {size_t n=0;sn=std::stoi(soNgay,&n);if(n!=soNgay.size()) sn=0;}catch(...){}
                  if (!lapDotChieuMoi(*p,chonTt,ngayMoi,sn,duKien)) {
                    TienIchConsole::hienThiLoi("Ngày khởi chiếu phải hợp lệ và sau hôm nay; thời hạn 1-365 ngày.");
                    Sleep(1500);continue;
                  }
                }
                if (chonTt != 0 && duKien.tinhTrangThaiTheoNgay(TienIchNgay::layNgayHienTaiISO()) != chonTt) {
                  TienIchConsole::hienThiLoi("Ngày và thời hạn không khớp trạng thái đã chọn."); Sleep(1500); continue;
                }
                duKien.setTrangThai(chonTt);
                if (duKien.layTrangThai()==truoc.layTrangThai() &&
                    duKien.layNgayKhoiChieu()==truoc.layNgayKhoiChieu() &&
                    duKien.laySoNgayChieu()==truoc.laySoNgayChieu()) {
                  TienIchConsole::hienThiThanhCong("Đã giữ nguyên trạng thái phim."); Sleep(800); continue;
                }
                std::cout << "\nXÁC NHẬN ĐỔI TRẠNG THÁI: " << p->layTenPhim()
                          << "\n  " << truoc.layChuoiTrangThai() << " -> " << duKien.layChuoiTrangThai()
                          << "\n  Khởi chiếu: " << duKien.layNgayKhoiChieu()
                          << " | Hết đợt: " << TienIchNgay::dinhDangHienThi(duKien.layNgayKetThuc())
                          << " | Số ngày: " << duKien.laySoNgayChieu() << "\n";
                if (chonTt==0) std::cout << "Suất tương lai chưa bán vé sẽ hủy; giữ suất đã bán và lịch sử.\n";
                else std::cout << "Ngày khởi chiếu chưa bảo đảm có suất. Lịch đã xếp được giữ nguyên;\n"
                                  "phim mới chỉ được bổ sung vào khung giờ trống phù hợp.\n";
                if (chonTt==2) {
                  const int duKienDangChieu=demPhimDangChieuDuKien(duKien);
                  std::cout << "Dự kiến ngày " << duKien.layNgayKhoiChieu() << ": " << duKienDangChieu
                            << "/" << GIOI_HAN_DANG_CHIEU << " phim đang chiếu.\n";
                  if (duKienDangChieu>GIOI_HAN_DANG_CHIEU)
                    std::cout << "CẢNH BÁO: Vượt quota. Hãy đổi ngày hoặc chủ động ngừng phim khác;\n"
                                 "chương trình không tự ngừng phim để đủ quota.\n";
                }
                std::cout << "[1/Y]: Xác nhận | [Enter/Khác]: Hủy: ";
                std::string xacNhan; std::getline(std::cin,xacNhan);
                xacNhan=QuanLyFile::trim(xacNhan);
                if (xacNhan!="1" && xacNhan!="y" && xacNhan!="Y") {
                  TienIchConsole::hienThiThanhCong("Đã hủy đổi trạng thái. Dữ liệu được giữ nguyên."); Sleep(1000); continue;
                }
                *p = duKien;
                // Khôi phục trạng thái cũ để kiểm tra quota không coi là giữ nguyên.
                p->setTrangThai(truoc.layTrangThai());
                if (!doiTrangThaiPhim(p->layMaPhim(), chonTt)) {
                  *p = truoc;
                  TienIchConsole::hienThiLoi("Đợt chiếu không hợp lệ. Cập nhật ngày khởi chiếu và thời hạn trước khi mở lại.");
                  Sleep(1500); continue;
                }
                if (!QuanLyFile::ghiDanhSachPhim("dulieu/phim.txt", dsPhim)) {
                  *p = truoc; TienIchConsole::hienThiLoi("Lưu phim thất bại!"); continue;
                }
                if (dangTimKiem) {
                  dsKetQua = timKiemPhim(tuKhoa);
                }
                if (onDoiTrangThai != nullptr) {
                  onDoiTrangThai(p->layMaPhim(), chonTt);
                } else {
                  TienIchConsole::hienThiThanhCong("Đã cập nhật trạng thái phim thành công!");
                  Sleep(1000);
                }
              }
            } else {
              TienIchConsole::hienThiLoi("Lựa chọn trạng thái không hợp lệ!");
              Sleep(1000);
            }
          }
        }
      } else if (ch == 'd' || ch == 'D') {
        if (!cheDoAdmin) {
          TienIchConsole::hienThiLoi("Chức năng chỉ dành cho Quản trị viên!");
          Sleep(1000);
          continue;
        }
        printf("\n\t\t>> Nhập mã phim cần xóa vĩnh viễn hoặc [0] / Enter để hủy - ví dụ 260001: ");
        std::string maP;
        std::getline(std::cin, maP);
        size_t start = maP.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
          size_t end = maP.find_last_not_of(" \t\r\n");
          std::string trimmedMa = maP.substr(start, end - start + 1);
          if (trimmedMa == "0") {
            continue; // Huy thao tac xoa em ai
          }

          Phim *p = timPhimTheoMa(trimmedMa);
          if (p == nullptr) {
            TienIchConsole::hienThiLoi("Không tìm thấy mã phim '" + trimmedMa + "'!");
            Sleep(1200);
          } else {
            std::cout << "\n\t\t!! CẢNH BÁO: Xóa vĩnh viễn phim ";
            TienIchConsole::inChuCoMau("[" + p->layMaPhim() + "] ", TienIchConsole::MAU_MA_PHIM);
            std::cout << p->layTenPhim() << "\n";
            printf("\t\t>> Bạn có chắc chắn muốn xóa không? [y/Y: Đồng ý | Phím khác/Enter: Hủy]: ");
            std::string xacNhan;
            std::getline(std::cin, xacNhan);
            if (xacNhan == "y" || xacNhan == "Y") {
              if (!xoaPhim(trimmedMa)) {
                TienIchConsole::hienThiLoi("Không thể xóa phim còn suất tương lai!"); Sleep(1500); continue;
              }
              QuanLyFile::ghiDanhSachPhim("dulieu/phim.txt", dsPhim);
              if (dangTimKiem) {
                dsKetQua = timKiemPhim(tuKhoa);
              }
              TienIchConsole::hienThiThanhCong("Đã xóa vĩnh viễn bộ phim khỏi hệ thống!");
              Sleep(1200);
            } else {
              TienIchConsole::hienThiLoi("Đã hủy thao tác xóa!");
              Sleep(800);
            }
          }
        }
      } else if (ch == 'r' || ch == 'R') {
        if (dangTimKiem) {
          dangTimKiem = false;
          tuKhoa = "";
          trangHienTai = 1;
        }
      } else if (ch == 'g' || ch == 'G') {
        printf("\n\t\tNhập số trang muốn đến [1-%d] hoặc [0] / Enter để hủy: ", tongTrang);
        std::string sTrang;
        std::getline(std::cin, sTrang);
        sTrang = TienIchConsole::catKhoangTrang(sTrang);
        if (sTrang == "0" || sTrang.empty() || sTrang == "q" || sTrang == "Q" || sTrang == "huy") {
          continue;
        }
        int trangNhap = 0;
        try { trangNhap = std::stoi(sTrang); } catch (...) { trangNhap = -1; }
        if (trangNhap >= 1 && trangNhap <= tongTrang) {
          trangHienTai = trangNhap;
        } else {
          TienIchConsole::hienThiLoi("Số trang không hợp lệ!");
          Sleep(1200);
        }
      } else if (ch == '0' || ch == 27) { // 0 hoac phim ESC de thoat
        break;
      }
    }
  }

  int laySoLuong() const { return dsPhim.laySoLuong(); }

  // Hien thi danh sach phim sap xep theo thoi luong tang dan (QuickSort)
  // Co phan trang tuong tac (Pagination), chong tran cuon man hinh va tuong thich Win32 Console
  void sapXepTheoThoiLuong(int pageSize = 5) {
    int n = dsPhim.laySoLuong();
    if (n == 0) {
      TienIchConsole::xoaManHinh();
      std::cout << "\n";
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("SẮP XẾP PHIM THEO THỜI LƯỢNG");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("Hiện tại hệ thống chưa có bộ phim nào!");
      TienIchConsole::veKhungDuoi();
      TienIchConsole::choPhimBatKy();
      return;
    }

    Phim *mang = new Phim[n];
    Nut<Phim> *hienTai = dsPhim.layDau();
    for (int i = 0; i < n; i++) {
      mang[i] = hienTai->duLieu;
      hienTai = hienTai->tiepTheo;
    }

    // Ap dung thuat toan QuickSort O(N log N) sap xep tang dan theo thoi luong
    quickSort(mang, 0, n - 1);

    int tongTrang = (n + pageSize - 1) / pageSize;
    if (tongTrang == 0) tongTrang = 1;
    int trangHienTai = 1;

    while (true) {
      if (trangHienTai > tongTrang) trangHienTai = tongTrang;
      if (trangHienTai < 1) trangHienTai = 1;

      TienIchConsole::xoaManHinh();
      std::cout << "\n";
      TienIchConsole::veKhungTren();
      char tieuDe[128];
      snprintf(tieuDe, sizeof(tieuDe), "PHIM THEO THỜI LƯỢNG (Trang %d/%d – Tăng dần)",
               trangHienTai, tongTrang);
      TienIchConsole::veDongGiua(tieuDe);
      TienIchConsole::veKhungGiua();

      int batDau = (trangHienTai - 1) * pageSize;
      int ketThuc = batDau + pageSize;
      if (ketThuc > n) ketThuc = n;

      int demTrongTrang = 0;
      for (int i = batDau; i < ketThuc; i++) {
        if (demTrongTrang > 0)
          TienIchConsole::veDuongKe();

        // Dong 1: Ma phim to mau Cyan, Ten phim to mau trang sang
        TienIchConsole::veDongPhimTieuDe(mang[i].layMaPhim(), mang[i].layTenPhim());

        // Dong 2: The loai va thoi luong (luon bao toan thoi luong hien thi ro rang)
        int maxInside = TienIchConsole::DO_RONG - 4;
        std::string suffixThoiLuong = " | " + std::to_string(mang[i].layThoiLuong()) + " phút";
        std::string prefixTl = "  Thể loại: ";
        int maxTl = maxInside - (TienIchConsole::doDaiHienThi(prefixTl) + TienIchConsole::doDaiHienThi(suffixThoiLuong));
        if (maxTl < 5) maxTl = 5;
        std::string tlIn = mang[i].layTheLoai();
        if (TienIchConsole::doDaiHienThi(tlIn) > maxTl) {
          tlIn = TienIchConsole::catChuoiTheoTu(tlIn, maxTl);
        }
        TienIchConsole::veDong(prefixTl + tlIn + suffixThoiLuong);

        // Dong 3: Dao dien va badge trang thai to mau
        TienIchConsole::veDongPhimDaoDienVaTrangThai(mang[i].layDaoDien(),
                                                    mang[i].layTrangThai(),
                                                    mang[i].layChuoiTrangThai());
        TienIchConsole::veDong("  Khởi chiếu: " + TienIchNgay::dinhDangHienThi(mang[i].layNgayKhoiChieu()) +
                              " | Hết đợt: " + TienIchNgay::dinhDangHienThi(mang[i].layNgayKetThuc()));
        demTrongTrang++;
      }

      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("[N]: Trang sau           [P]: Trang trước");
      TienIchConsole::veDong("[G]: Nhảy đến trang      [0]: Quay lại Menu chính");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để thao tác hoặc nhấn Esc để thoát: ");
      int ch = _getch();

      // Bo qua ma phim mo rong 2 byte (phim mui ten)
      if (ch == 0 || ch == 224) {
        int extra = _getch();
        if (extra == 77) { // Mui ten phai -> Trang sau
          if (trangHienTai < tongTrang) trangHienTai++;
        } else if (extra == 75) { // Mui ten trai -> Trang truoc
          if (trangHienTai > 1) trangHienTai--;
        }
        continue;
      }

      if (ch == 'n' || ch == 'N') {
        if (trangHienTai < tongTrang) {
          trangHienTai++;
        }
      } else if (ch == 'p' || ch == 'P') {
        if (trangHienTai > 1) {
          trangHienTai--;
        }
      } else if (ch == 'g' || ch == 'G') {
        printf("\n\t\tNhập số trang muốn đến [1-%d] hoặc [0] / Enter để hủy: ", tongTrang);
        std::string sTrang;
        std::getline(std::cin, sTrang);
        sTrang = TienIchConsole::catKhoangTrang(sTrang);
        if (sTrang == "0" || sTrang.empty() || sTrang == "q" || sTrang == "Q" || sTrang == "huy") {
          continue;
        }
        int trangNhap = 0;
        try { trangNhap = std::stoi(sTrang); } catch (...) { trangNhap = -1; }
        if (trangNhap >= 1 && trangNhap <= tongTrang) {
          trangHienTai = trangNhap;
        } else {
          TienIchConsole::hienThiLoi("Số trang không hợp lệ!");
          Sleep(1200);
        }
      } else if (ch == '0' || ch == 27) { // 0 hoac Esc de thoat
        break;
      }
    }

    delete[] mang;
  }

  DanhSachLienKet<Phim> &layDanhSach() { return dsPhim; }
};

#endif
