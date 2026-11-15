#ifndef DICHVUDATVE_H
#define DICHVUDATVE_H

#include "../cautruc/DanhSachLienKet.h"
#include "../giaodien/TienIchConsole.h"
#include "../luutru/QuanLyFile.h"
#include "../models/HoaDon.h"
#include "../models/Phim.h"
#include "../models/SuatChieu.h"
#include "../models/ThongKeKyVM.h"
#include "../models/Ve.h"
#include "../tienich/TienIchNgay.h"
#include "DichVuCaTruc.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <limits>
#include <cmath>
#include <set>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

class DichVuDatVe {
private:
  DanhSachLienKet<SuatChieu *> dsSuatChieu;
  DanhSachLienKet<HoaDon *> dsHoaDon;
  bool khoaGhi = false;
  bool daNapHoaDon = false;
  int demVe;
  int demHD;
  std::string duongDanFile;
  std::string duongDanFileHD;
  const DanhSachLienKet<Phim> *dsPhimRef;

public:
  DichVuDatVe(const DichVuDatVe&) = delete;
  DichVuDatVe& operator=(const DichVuDatVe&) = delete;
  DichVuDatVe()
      : demVe(0), demHD(0), duongDanFile("dulieu/suatchieu.txt"),
        duongDanFileHD("dulieu/hoadon.txt"), dsPhimRef(nullptr) {}

  DanhSachLienKet<SuatChieu *> &layDanhSachSuatChieu() { return dsSuatChieu; }
  const DanhSachLienKet<SuatChieu *> &layDanhSachSuatChieu() const { return dsSuatChieu; }

  void themSuatChieu(SuatChieu *sc) { dsSuatChieu.themCuoi(sc); }

  void ganDanhSachPhim(const DanhSachLienKet<Phim> *dsP) {
    dsPhimRef = dsP;
  }

  int layThoiLuongPhim(const std::string &maPhim, const DanhSachLienKet<Phim> *dsP = nullptr) const {
    const DanhSachLienKet<Phim> *ds = (dsP != nullptr) ? dsP : dsPhimRef;
    if (ds == nullptr) return 0;
    Nut<Phim> *cur = ds->layDau();
    while (cur != nullptr) {
      if (cur->duLieu.layMaPhim() == maPhim) {
        return cur->duLieu.layThoiLuong();
      }
      cur = cur->tiepTheo;
    }
    return 0;
  }

  // Khởi tạo lịch chiếu: Đọc từ file dulieu/suatchieu.txt, nếu chưa có thì tự động sinh 7 ngày tới
  void khoiTaoLichChieu(const std::string &duongDan, DanhSachLienKet<Phim> &dsPhim) {
    duongDanFile = duongDan;
    khoaGhi = std::filesystem::exists(duongDanFile + ".blocked");
    dsPhimRef = &dsPhim;
    if (!QuanLyFile::docDanhSachSuatChieu(duongDanFile, dsSuatChieu)) khoaGhi = true;
    // Chưa ghi file khi chưa nạp và đối soát hóa đơn.
  }

  bool hoanTatKhoiTao(DanhSachLienKet<Phim> &dsPhim) {
    kiemTraDoiSoat();
    if (khoaGhi || !daNapHoaDon) return false;
    for (auto p = dsPhim.layDau(); p; p = p->tiepTheo) {
      if (p->duLieu.layTrangThai() == 0) {
        std::vector<std::string> giuLai;
        if (xuLyKhiPhimNgungChieu(p->duLieu.layMaPhim(), giuLai) < 0) return false;
      }
    }
    for (auto n = dsSuatChieu.layDau(); n; n = n->tiepTheo)
      for (auto p = dsPhim.layDau(); p; p = p->tiepTheo)
        if (n->duLieu->layMaPhim() == p->duLieu.layMaPhim()) {
          n->duLieu->setTenPhim(p->duLieu.layTenPhim()); break;
        }
    sinhLichChieuTuDong(dsPhim, soNgayPhuLich());
    return luuLichChieu();
  }

  static int soNgayPhuLich() {
    return 7; // Hôm nay và sáu ngày tiếp theo.
  }

  int demSuatTheoPhim(const std::string &maPhim, const std::string &tuNgay,
                     const std::string &denNgay) const {
    int count=0;
    for (auto n=dsSuatChieu.layDau();n;n=n->tiepTheo) {
      const auto sc=n->duLieu;
      if (sc->layMaPhim()==maPhim && sc->layNgayChieu()>=tuNgay && sc->layNgayChieu()<=denNgay) ++count;
    }
    return count;
  }

  bool coSuatSauNgay(const std::string &maPhim, const std::string &ngayCuoi) const {
    for (auto n=dsSuatChieu.layDau();n;n=n->tiepTheo)
      if (n->duLieu->layMaPhim()==maPhim && n->duLieu->layNgayChieu()>ngayCuoi) return true;
    return false;
  }

  bool coSuatTuongLai(const std::string &maPhim) const {
    for (auto n = dsSuatChieu.layDau(); n; n = n->tiepTheo)
      if (n->duLieu->layMaPhim() == maPhim &&
          (n->duLieu->layNgayChieu() > TienIchNgay::layNgayHienTaiISO() ||
           (n->duLieu->layNgayChieu() == TienIchNgay::layNgayHienTaiISO() &&
            n->duLieu->layGioChieu() > TienIchNgay::layGioHienTai()))) return true;
    return false;
  }

  // Lưu bền vững lịch chiếu và toàn bộ trạng thái ghế vào file
  bool luuLichChieu() {
    if (khoaGhi) return false;
    if (QuanLyFile::ghiDanhSachSuatChieu(duongDanFile, dsSuatChieu)) return true;
    khoaGhi = true; return false;
  }

  // Đồng bộ tên phim mới sang các suất chiếu hiện hành khi Admin sửa tên phim
  bool capNhatTenPhimSuatChieu(const std::string &maPhim, const std::string &tenMoi) {
    bool coThayDoi = false;
    for (auto n = dsSuatChieu.layDau(); n; n = n->tiepTheo) {
      if (n->duLieu->layMaPhim() == maPhim && n->duLieu->layTenPhim() != tenMoi) {
        n->duLieu->setTenPhim(tenMoi);
        coThayDoi = true;
      }
    }
    if (coThayDoi && !duongDanFile.empty()) {
      return luuLichChieu();
    }
    return true;
  }

  // Thuật toán Sinh lịch chiếu Lai (Hybrid Auto-Scheduler) cho 5 phòng chiếu trong n ngày tới
  void sinhLichChieuTuDong(DanhSachLienKet<Phim> &dsPhim, int soNgayToi = 7) {
    if (khoaGhi) return;
    dsPhimRef = &dsPhim;
    if (dsPhim.laySoLuong() == 0 || soNgayToi<=0) return;
    soNgayToi=std::min(soNgayToi,soNgayPhuLich());

    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();

    // 5 Phòng chiếu chuẩn hóa
    struct PhongMau {
      std::string maPhong;
      std::string tenPhong;
      int soHang;
      int soCot;
    };
    PhongMau danhSachPhong[5] = {
      {"P01", "Phòng Chiếu 01", 8, 6},
      {"P02", "Phòng Chiếu 02", 8, 6},
      {"P03", "Phòng Chiếu 03", 8, 6},
      {"P04", "Phòng Chiếu 04", 8, 6},
      {"P05", "Phòng VIP 05", 4, 6}
    };

    // Khung giờ chiếu chuẩn trong ngày cho phòng thường và phòng VIP
    std::string gioChieuChuan[5] = {"09:30", "13:00", "16:00", "19:00", "21:45"};
    std::string gioChieuVIP[5] = {"10:00", "13:30", "16:30", "19:30", "22:00"};

    for (int d = 0; d < soNgayToi; d++) {
      std::string ngayMucTieu = TienIchNgay::congNgay(ngayHienTai, d);

      // Thu thập danh sách phim đang chiếu và phim có suất chiếu sớm
      std::vector<Phim> dsPhimDangChieu;
      std::vector<Phim> dsPhimChieuSom;

      Nut<Phim> *curPhim = dsPhim.layDau();
      while (curPhim != nullptr) {
        int tt = curPhim->duLieu.layTrangThai() == 0 ? 0 : curPhim->duLieu.tinhTrangThaiTheoNgay(ngayMucTieu);
        if (tt == 1) {
          dsPhimDangChieu.push_back(curPhim->duLieu);
        } else if (tt == 2 && curPhim->duLieu.coTheChieuSom(ngayMucTieu)) {
          dsPhimChieuSom.push_back(curPhim->duLieu);
        }
        curPhim = curPhim->tiepTheo;
      }

      if (dsPhimDangChieu.empty() && dsPhimChieuSom.empty()) {
        continue;
      }

      int caThuTrongNgay = 1;

      // 1. Lập lịch cho Phòng VIP P05 (Ưu tiên Suất chiếu sớm Sneak Show 180k)
      for (int k = 0; k < 5; k++) {
        Phim phimChon;
        bool coPhim = false;

        if (!dsPhimChieuSom.empty()) {
          // Ưu tiên chiếu sớm các phim Sneak Show ở P05
          phimChon = dsPhimChieuSom[k % dsPhimChieuSom.size()];
          coPhim = true;
        } else if (!dsPhimDangChieu.empty()) {
          // Nếu không có phim chiếu sớm, chiếu phim đang hot ở trải nghiệm VIP
          phimChon = dsPhimDangChieu[k % dsPhimDangChieu.size()];
          coPhim = true;
        }

        if (coPhim) {
          std::string maSC = TienIchNgay::sinhMaSuatChieu(ngayMucTieu, caThuTrongNgay++);
          SuatChieu *sc = new SuatChieu(
              maSC, phimChon.layMaPhim(), phimChon.layTenPhim(),
              ngayMucTieu, gioChieuVIP[k],
              danhSachPhong[4].maPhong, danhSachPhong[4].tenPhong,
              danhSachPhong[4].soHang, danhSachPhong[4].soCot);
          themSuatNeuTrong(sc);
        }
      }

      // 2. Lập lịch cho các Phòng Tiêu Chuẩn P01 - P04 (48 ghế)
      if (!dsPhimDangChieu.empty()) {
        size_t idxPhim = 0;
        for (int p = 0; p < 4; p++) {
          for (int k = 0; k < 4; k++) { // 4 ca mỗi phòng tiêu chuẩn
            Phim phimChon = dsPhimDangChieu[(idxPhim++) % dsPhimDangChieu.size()];
            std::string maSC = TienIchNgay::sinhMaSuatChieu(ngayMucTieu, caThuTrongNgay++);
            SuatChieu *sc = new SuatChieu(
                maSC, phimChon.layMaPhim(), phimChon.layTenPhim(),
                ngayMucTieu, gioChieuChuan[k],
                danhSachPhong[p].maPhong, danhSachPhong[p].tenPhong,
                danhSachPhong[p].soHang, danhSachPhong[p].soCot);
            themSuatNeuTrong(sc);
          }
        }
      }
    }
  }

  void themSuatNeuTrong(SuatChieu *sc) {
    for (auto n = dsSuatChieu.layDau(); n; n = n->tiepTheo) {
      auto cu = n->duLieu;
      auto phut = [](const std::string &gio) { return std::stoi(gio.substr(0,2))*60 + std::stoi(gio.substr(3,2)); };
      if (cu->layNgayChieu() == sc->layNgayChieu() &&
          cu->layPhongChieu()->layMaPhong() == sc->layPhongChieu()->layMaPhong()) {
        int a=phut(cu->layGioChieu()), b=phut(sc->layGioChieu());
        if (a < b + layThoiLuongPhim(sc->layMaPhim()) &&
            b < a + layThoiLuongPhim(cu->layMaPhim())) { delete sc; return; }
      }
      if (cu->layMaSuatChieu() == sc->layMaSuatChieu() ||
          (cu->layNgayChieu() == sc->layNgayChieu() &&
           cu->layPhongChieu()->layMaPhong() == sc->layPhongChieu()->layMaPhong() &&
           cu->layGioChieu() == sc->layGioChieu())) { delete sc; return; }
    }
    dsSuatChieu.themCuoi(sc);
  }

  // Tìm suất chiếu hỗ trợ đối sánh mờ thông minh (Fuzzy match: 'SC261030-01', '30-01', '01'...)
  SuatChieu *timSuatChieu(const std::string &inputRaw, const std::string &ngayISO = "") {
    if (inputRaw.empty()) return nullptr;
    SuatChieu *ketQua = nullptr;
    for (auto n = dsSuatChieu.layDau(); n; n = n->tiepTheo) {
      auto sc = n->duLieu;
      if (!ngayISO.empty() && sc->layNgayChieu() != ngayISO) continue;
      if (sc->layMaSuatChieu() == inputRaw ||
          TienIchNgay::khopMaSuatChieu(sc->layMaSuatChieu(), inputRaw)) {
        if (ketQua) return nullptr; // Không chọn tùy tiện khi mã mơ hồ.
        ketQua = sc;
      }
    }
    return ketQua;
  }

  // Cơ chế Bảo vệ khi Ngừng chiếu đột xuất (Early Pull-out Protection):
  // Xóa các suất tương lai chưa có vé, giữ lại các suất đã có vé và báo cáo
  int xuLyKhiPhimNgungChieu(const std::string &maPhim, std::vector<std::string> &thongBaoGiuLai) {
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    if (khoaGhi) return -1;
    int soSuatDaHuy = 0;
    const auto truoc = dsSuatChieu;
    std::vector<SuatChieu *> daGo;

    Nut<SuatChieu *> *hienTai = dsSuatChieu.layDau();

    while (hienTai != nullptr) {
      SuatChieu *sc = hienTai->duLieu;
      if (sc->layMaPhim() == maPhim && (sc->layNgayChieu() > ngayHienTai || (sc->layNgayChieu() == ngayHienTai && sc->layGioChieu() > TienIchNgay::layGioHienTai()))) {
        if (sc->demSoGheDaDat() == 0) {
          // Suất chưa có ai mua vé -> Tự động hủy an toàn để giải phóng phòng
          hienTai = hienTai->tiepTheo;
          dsSuatChieu.xoaTheoGiaTri(sc);
          daGo.push_back(sc);
          soSuatDaHuy++;
          continue;
        } else {
          // Suất đã có vé -> Giữ lại để chiếu vét phục vụ khách đã mua vé
          char tb[256];
          snprintf(tb, sizeof(tb),
                   "Suất [%s] ngày %s (%s) có %d vé đã bán -> GIỮ LẠI ĐỂ CHIẾU VÉT",
                   sc->layMaSuatChieu().c_str(),
                   TienIchNgay::dinhDangHienThi(sc->layNgayChieu()).c_str(),
                   sc->layGioChieu().c_str(),
                   sc->demSoGheDaDat());
          thongBaoGiuLai.push_back(std::string(tb));
        }
      }
      hienTai = hienTai->tiepTheo;
    }

    if (soSuatDaHuy > 0) {
      if (!luuLichChieu()) { dsSuatChieu = truoc; return -1; }
    }
    for (auto sc : daGo) delete sc;
    return soSuatDaHuy;
  }

  bool laSuatChieuSom(const SuatChieu *sc, const DanhSachLienKet<Phim> *dsNgoai = nullptr) const {
    if (!sc || !sc->laPhongVIP()) return false;
    const auto ds = dsNgoai ? dsNgoai : dsPhimRef;
    if (!ds) return false;
    for (auto n=ds->layDau(); n; n=n->tiepTheo) {
      const auto &p=n->duLieu;
      if (p.layMaPhim()==sc->layMaPhim())
        return p.tinhTrangThaiTheoNgay(sc->layNgayChieu())==2 && p.coTheChieuSom(sc->layNgayChieu());
    }
    return false;
  }

  // Hiển thị lịch chiếu phân tách theo ngày chuẩn Movie-Centric (Gom nhóm theo Phim & Phòng)
  void hienThiLichChieuTheoNgay(const std::string &ngayISO,
                                const DanhSachLienKet<Phim> *dsPhimNgoai = nullptr) const {
    printf("\n");
    TienIchConsole::veKhungTren();
    std::string nhanNgay = TienIchNgay::layNhanNgayKemThu(ngayISO);
    TienIchConsole::veDongGiua("LỊCH CHIẾU - " + nhanNgay);
    TienIchConsole::veKhungGiua();

    // 1. Cấu trúc gom nhóm theo Phim và Phòng chiếu trong ngày
    struct CaChieuVM {
      std::string maSC;
      std::string gioChieu;
      std::string gioKetThuc;
      int soGheTrong;
      int tongGhe;
    };

    struct PhongChieuVM {
      std::string maPhong;
      std::string tenPhong;
      bool laVIP;
      std::vector<CaChieuVM> dsCa;
    };

    struct NhomPhimVM {
      std::string maPhim;
      std::string tenPhim;
      int thoiLuong;
      bool laSneakVIP;
      std::vector<PhongChieuVM> dsPhong;
    };

    std::vector<NhomPhimVM> dsNhom;

    // 2. Thuật toán Two-Pass Grouping gom nhóm theo từng bộ phim
    Nut<SuatChieu *> *cur = dsSuatChieu.layDau();
    while (cur != nullptr) {
      SuatChieu *sc = cur->duLieu;
      if (sc != nullptr && sc->layNgayChieu() == ngayISO && sc->layPhongChieu() != nullptr) {
        int idxPhim = -1;
        for (size_t i = 0; i < dsNhom.size(); i++) {
          if (dsNhom[i].maPhim == sc->layMaPhim()) {
            idxPhim = (int)i;
            break;
          }
        }
        if (idxPhim == -1) {
          NhomPhimVM np;
          np.maPhim = sc->layMaPhim();
          np.tenPhim = sc->layTenPhim();
          np.thoiLuong = layThoiLuongPhim(sc->layMaPhim(), dsPhimNgoai);
          np.laSneakVIP = laSuatChieuSom(sc, dsPhimNgoai);
          dsNhom.push_back(np);
          idxPhim = (int)dsNhom.size() - 1;
        } else {
          if (laSuatChieuSom(sc, dsPhimNgoai)) {
            dsNhom[idxPhim].laSneakVIP = true;
          }
        }

        std::string maP = sc->layPhongChieu()->layMaPhong();
        int idxPhong = -1;
        for (size_t j = 0; j < dsNhom[idxPhim].dsPhong.size(); j++) {
          if (dsNhom[idxPhim].dsPhong[j].maPhong == maP) {
            idxPhong = (int)j;
            break;
          }
        }
        if (idxPhong == -1) {
          PhongChieuVM pc;
          pc.maPhong = maP;
          pc.tenPhong = sc->layPhongChieu()->layTenPhong();
          pc.laVIP = sc->laPhongVIP();
          dsNhom[idxPhim].dsPhong.push_back(pc);
          idxPhong = (int)dsNhom[idxPhim].dsPhong.size() - 1;
        }

        CaChieuVM ca;
        ca.maSC = sc->layMaSuatChieu();
        ca.gioChieu = sc->layGioChieu();
        ca.gioKetThuc = TienIchNgay::tinhGioKetThuc(ca.gioChieu, dsNhom[idxPhim].thoiLuong);
        ca.soGheTrong = sc->demSoGheTrong();
        ca.tongGhe = sc->layTongSoGhe();
        dsNhom[idxPhim].dsPhong[idxPhong].dsCa.push_back(ca);
      }
      cur = cur->tiepTheo;
    }

    if (dsNhom.empty()) {
      TienIchConsole::veDongTrong();
      TienIchConsole::veDongGiua("Không có suất chiếu nào trong ngày này!");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();
      return;
    }

    // 3. Hiển thị từng khối phim tinh gọn, chuyên nghiệp
    for (size_t i = 0; i < dsNhom.size(); i++) {
      if (i > 0) {
        TienIchConsole::veDuongKe();
      }

      const NhomPhimVM &np = dsNhom[i];

      // Dòng 1: Tiêu đề phim duy nhất 1 lần (kèm thời lượng và nhãn Sneak VIP nếu có)
      std::string dongTieuDe = "▶ [" + np.maPhim + "] " + np.tenPhim;
      if (np.thoiLuong > 0) {
        dongTieuDe += " (" + std::to_string(np.thoiLuong) + " phút)";
      }
      if (np.laSneakVIP) {
        dongTieuDe += " - [SNEAK SHOW VIP]";
      }
      TienIchConsole::veDong(dongTieuDe);

      // Duyệt qua các phòng chiếu của phim
      for (size_t p = 0; p < np.dsPhong.size(); p++) {
        const PhongChieuVM &phong = np.dsPhong[p];

        if (phong.dsCa.size() == 1) {
          // Chỉ có 1 ca trong phòng này -> Gộp phòng và suất chiếu trên 1 dòng
          const CaChieuVM &ca = phong.dsCa[0];
          std::string maTat = ca.maSC;
          size_t posDash = maTat.rfind('-');
          if (posDash != std::string::npos) {
            maTat = maTat.substr(posDash + 1);
          }

          std::string gioHienThi = ca.gioChieu;
          if (!ca.gioKetThuc.empty() && ca.gioKetThuc != ca.gioChieu) {
            gioHienThi += " - " + ca.gioKetThuc;
          }

          char dongGop[128];
          snprintf(dongGop, sizeof(dongGop), "  • %s: %s [%s] (%d trống)",
                   phong.tenPhong.c_str(), gioHienThi.c_str(), maTat.c_str(), ca.soGheTrong);
          TienIchConsole::veDong(dongGop);
        } else {
          // Nhiều ca trong phòng (như phòng VIP P05)
          std::string dongPhong = "  • " + phong.tenPhong;
          if (phong.laVIP) {
            dongPhong += " (VIP Suite - 180k):";
          } else {
            dongPhong += ":";
          }
          TienIchConsole::veDong(dongPhong);

          for (size_t c = 0; c < phong.dsCa.size(); c++) {
            const CaChieuVM &ca = phong.dsCa[c];
            std::string maTat = ca.maSC;
            size_t posDash = maTat.rfind('-');
            if (posDash != std::string::npos) {
              maTat = maTat.substr(posDash + 1);
            }

            std::string gioHienThi = ca.gioChieu;
            if (!ca.gioKetThuc.empty() && ca.gioKetThuc != ca.gioChieu) {
              gioHienThi += " - " + ca.gioKetThuc;
            }

            char dongCa[128];
            snprintf(dongCa, sizeof(dongCa), "    %s [%s] (%d trống)",
                     gioHienThi.c_str(), maTat.c_str(), ca.soGheTrong);
            TienIchConsole::veDong(dongCa);
          }
        }
      }
    }

    TienIchConsole::veKhungDuoi();
  }

  // Hiển thị toàn bộ lịch chiếu (tổng hợp)
  void hienThiLichChieu() const {
    printf("\n");
    TienIchConsole::veKhungTren();
    TienIchConsole::veDongGiua("TỔNG HỢP LỊCH CHIẾU PHIM");
    TienIchConsole::veKhungGiua();

    Nut<SuatChieu *> *hienTai = dsSuatChieu.layDau();
    int dem = 0;
    while (hienTai != nullptr) {
      dem++;
      if (dem > 1)
        TienIchConsole::veDuongKe();

      SuatChieu *sc = hienTai->duLieu;
      char dong1[256], dong2[256];
      snprintf(dong1, sizeof(dong1), "[%s] %s",
               sc->layMaSuatChieu().c_str(),
               sc->layTenPhim().c_str());
      snprintf(dong2, sizeof(dong2), "  Phòng: %s | Giờ: %s | Ngày: %s (%s)",
               sc->layPhongChieu()->layTenPhong().c_str(),
               sc->layGioChieu().c_str(),
               TienIchNgay::dinhDangHienThi(sc->layNgayChieu()).c_str(),
               TienIchNgay::layChuoiThu(sc->layNgayChieu()).c_str());
      TienIchConsole::veDong(dong1);
      TienIchConsole::veDong(dong2);
      hienTai = hienTai->tiepTheo;
    }

    if (dem == 0) {
      TienIchConsole::veDongTrong();
      TienIchConsole::veDongGiua("Chưa có suất chiếu nào trong hệ thống!");
      TienIchConsole::veDongTrong();
    }

    TienIchConsole::veKhungDuoi();
  }

  int layTongSoVeDaBan() const { return demVe; }
  int layTongSoHoaDon() const { return dsHoaDon.laySoLuong(); }

  double tinhTongDoanhThu() const {
    Nut<HoaDon *> *hienTai = dsHoaDon.layDau();
    double tong = 0;
    while (hienTai != nullptr) {
      tong += hienTai->duLieu->layTongTien();
      hienTai = hienTai->tiepTheo;
    }
    return tong;
  }

  // Tìm kiếm hóa đơn theo mã đầy đủ hoặc viết tắt (vd: "HD001", "hd001", "1")
  HoaDon *timHoaDon(const std::string &maHDInput) const {
    std::string s = TienIchConsole::trim(maHDInput);
    if (s.empty()) return nullptr;

    for (char &c : s) {
      c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    std::string maChuan = s;
    if (s.find_first_not_of("0123456789") == std::string::npos) {
      try {
        int num = std::stoi(s);
        if (num < 10) maChuan = "HD00" + std::to_string(num);
        else if (num < 100) maChuan = "HD0" + std::to_string(num);
        else maChuan = "HD" + std::to_string(num);
      } catch (...) {}
    }

    Nut<HoaDon *> *cur = dsHoaDon.layDau();
    while (cur != nullptr) {
      if (cur->duLieu != nullptr) {
        std::string maHD = cur->duLieu->layMaHoaDon();
        if (maHD == s || maHD == maChuan) {
          return cur->duLieu;
        }
      }
      cur = cur->tiepTheo;
    }
    return nullptr;
  }

  void tinhDoanhThuNhanVienTrongNgay(const std::string &maNV, const std::string &ngayISO,
                                     int &soVe, int &soHD, double &tongTien,
                                     const std::string &maCa = "") const {
    soVe = soHD = 0; tongTien = 0;
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo) {
      const auto hd = n->duLieu;
      if (!maNV.empty() && hd->layMaNhanVien() == maNV &&
          TienIchNgay::chuanHoaISO(hd->layNgayLap()) == ngayISO &&
          (maCa.empty() || hd->layMaCa() == maCa)) {
        ++soHD; soVe += hd->laySoLuongVe(); tongTien += hd->layTongTien();
      }
    }
  }

  bool coHoaDonNhanVien(const std::string &maNV, const std::string &ten) const {
    if (!daNapHoaDon) return true; // Chưa đủ dữ liệu để chứng minh có thể xóa.
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo)
      if (n->duLieu->layMaNhanVien() == maNV ||
          (n->duLieu->layMaNhanVien().empty() && n->duLieu->layTenNhanVien() == ten)) return true;
    return false;
  }

  bool coHoaDonCa(const std::string &maCa) const {
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo)
      if (!maCa.empty() && n->duLieu->layMaCa() == maCa) return true;
    return false;
  }

  bool dongBoDoanhThuCa(DichVuCaTruc &dvCa) const {
    std::map<std::string, double> tong;
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo)
      if (!n->duLieu->layMaCa().empty()) tong[n->duLieu->layMaCa()] += n->duLieu->layTongTien();
    bool ok = true;
    for (const auto &p : tong) if (!dvCa.doiSoatDoanhThu(p.first, p.second)) ok = false;
    return ok;
  }

  // Nạp toàn bộ hoặc từ chối toàn bộ; không ghi đè sau khi đọc thiếu dữ liệu.
  bool docDanhSachHoaDon(const std::string &duongDan = "dulieu/hoadon.txt") {
    daNapHoaDon = false;
    duongDanFileHD = duongDan;
    std::ifstream file(duongDan);
    if (!file) { khoaGhi = true; return false; }
    std::vector<std::unique_ptr<HoaDon>> moi;
    std::set<std::string> maVeDaCo, gheDaCo;
    std::map<int, std::string> maHoaDonChuan;
    int maxHD = 0, maxVe = 0, dongSo = 0;
    auto soMa = [](const std::string &ma, const std::string &prefix) {
      if (ma.size() <= 2 || ma.substr(0, 2) != prefix ||
          ma.find_first_not_of("0123456789", 2) != std::string::npos) throw 0;
      int n = std::stoi(ma.substr(2));
      if (n < 1 || n == std::numeric_limits<int>::max()) throw 0;
      return n;
    };
    std::string dong;
    try {
      while (std::getline(file, dong)) {
        ++dongSo;
        if (!dong.empty() && dong.back() == '\r') dong.pop_back();
        if (dong.empty()) continue;
        std::vector<std::string> t;
        std::stringstream ss(dong); std::string token;
        while (std::getline(ss, token, '|')) t.push_back(token);
        if (dong.back() == '|') t.push_back("");
        if (t.size() != 11 && t.size() != 14) throw 0;
        const bool legacy = t.size() == 11;
        t.resize(14);
        size_t used = 0; double gia = std::stod(t[10], &used);
        if (used != t[10].size() || !std::isfinite(gia) || gia < 0 || gia > 1000000000) throw 0;
        std::tm date{};
        if (!TienIchNgay::phanTichISO(TienIchNgay::chuanHoaISO(t[1]), date) ||
            !TienIchNgay::phanTichISO(TienIchNgay::chuanHoaISO(t[7]), date) ||
            !TienIchNgay::kiemTraDinhDangGio(t[8]) || t[2].empty() || t[4].empty()) throw 0;
        if (!legacy && (!t[11].empty() || !t[12].empty() || !t[13].empty()) &&
            (t[11].empty() || !TienIchNgay::kiemTraDinhDangGio(t[13]))) throw 0;
        if (t[9].size() < 2 || t[9][0] < 'A' || t[9][0] > 'Z' ||
            t[9].find_first_not_of("0123456789", 1) != std::string::npos || std::stoi(t[9].substr(1)) < 1) throw 0;
        const int idHD = soMa(t[0], "HD"), idVe = soMa(t[3], "VE");
        const auto idCu = maHoaDonChuan.find(idHD);
        if (idCu != maHoaDonChuan.end() && idCu->second != t[0]) throw 0;
        maHoaDonChuan[idHD] = t[0];
        if (!maVeDaCo.insert(std::to_string(idVe)).second ||
            !gheDaCo.insert(t[4] + "|" + t[9].substr(0, 1) + std::to_string(std::stoi(t[9].substr(1)))).second) throw 0;
        maxHD = (std::max)(maxHD, idHD); maxVe = (std::max)(maxVe, idVe);
        HoaDon *hd = nullptr;
        for (auto &h : moi) if (h->layMaHoaDon() == t[0]) { hd = h.get(); break; }
        if (hd && (hd->layNgayLap() != t[1] || hd->layTenNhanVien() != t[2] ||
                   hd->layMaNhanVien() != t[11] || hd->layMaCa() != t[12] || hd->layGioLap() != t[13])) throw 0;
        if (!hd) { moi.emplace_back(new HoaDon(t[0], t[1], t[2], t[11], t[12], t[13])); hd = moi.back().get(); }
        hd->themVe(Ve(t[3], t[4], t[5], t[6], t[7], t[8], t[9], gia));
      }
      if (file.bad()) throw 0;
    } catch (...) {
      std::cerr << "Hoa don khong hop le tai dong " << dongSo << ". Da khoa ghi.\n";
      khoaGhi = true; return false;
    }
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo) delete n->duLieu;
    dsHoaDon = DanhSachLienKet<HoaDon *>();
    for (auto &h : moi) dsHoaDon.themCuoi(h.release());
    demVe = maxVe; demHD = maxHD; daNapHoaDon = true;
    return true;
  }

  // Chỉ kiểm tra; không tự suy diễn hay sửa hóa đơn lịch sử.
  std::vector<std::string> kiemTraDoiSoat() {
    std::vector<std::string> loi;
    const auto homNay = TienIchNgay::layNgayHienTaiISO();
    for (auto n = dsHoaDon.layDau(); n; n = n->tiepTheo) {
      for (auto v = n->duLieu->layDanhSachVe().layDau(); v; v = v->tiepTheo) {
        const auto &ve = v->duLieu;
        SuatChieu *sc = nullptr;
        for (auto p = dsSuatChieu.layDau(); p; p = p->tiepTheo)
          if (p->duLieu->layMaSuatChieu() == ve.layMaSuatChieu()) { sc = p->duLieu; break; }
        const int hang = ve.layMaGhe()[0] - 'A', cot = std::stoi(ve.layMaGhe().substr(1)) - 1;
        const bool khop = sc && sc->layPhongChieu() && hang < sc->layPhongChieu()->laySoHang() &&
          cot < sc->layPhongChieu()->laySoCot() && sc->layPhongChieu()->kiemTraGheDaDat(hang, cot) &&
          sc->layNgayChieu() == TienIchNgay::chuanHoaISO(ve.layNgayChieu()) && sc->layGioChieu() == ve.layGioChieu() &&
          sc->layPhongChieu()->layTenPhong() == ve.layPhongChieu();
        if (!khop) {
          loi.push_back(ve.layMaVe() + ": " + ve.layMaSuatChieu() + " / " + ve.layMaGhe());
          if (TienIchNgay::chuanHoaISO(ve.layNgayChieu()) >= homNay || (sc && sc->layNgayChieu() >= homNay)) khoaGhi = true;
        }
      }
    }
    return loi;
  }

  // Lưu toàn bộ danh sách hóa đơn hiện tại
  bool luuToanBoHoaDon(const std::string &duongDan = "") const {
    if (khoaGhi) return false;
    std::string path = duongDan.empty() ? duongDanFileHD : duongDan;
    return ghiFileAnToan(path, [&](std::ostream &file) {

    Nut<HoaDon *> *curHD = dsHoaDon.layDau();
    while (curHD != nullptr) {
      HoaDon *hd = curHD->duLieu;
      if (hd != nullptr) {
        Nut<Ve> *curVe = hd->layDanhSachVe().layDau();
        while (curVe != nullptr) {
          const Ve &v = curVe->duLieu;
          file << QuanLyFile::khuKyTuPhanCach(hd->layMaHoaDon()) << "|"
               << QuanLyFile::khuKyTuPhanCach(hd->layNgayLap()) << "|"
               << QuanLyFile::khuKyTuPhanCach(hd->layTenNhanVien()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layMaVe()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layMaSuatChieu()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layTenPhim()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layPhongChieu()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layNgayChieu()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layGioChieu()) << "|"
               << QuanLyFile::khuKyTuPhanCach(v.layMaGhe()) << "|"
               << (long long)v.layGiaVe() << "|"
               << QuanLyFile::khuKyTuPhanCach(hd->layMaNhanVien()) << "|"
               << QuanLyFile::khuKyTuPhanCach(hd->layMaCa()) << "|"
               << QuanLyFile::khuKyTuPhanCach(hd->layGioLap()) << "\n";
          curVe = curVe->tiepTheo;
        }
      }
      curHD = curHD->tiepTheo;
    }
    });
  }

  // Tính toán báo cáo kinh doanh và thống kê đa chiều theo khoảng thời gian [tuNgayISO, denNgayISO]
  ThongKeKyVM tinhThongKeTheoKy(const std::string &tuNgayISO, const std::string &denNgayISO,
                                const std::string &tieuDeKy,
                                const DichVuCaTruc &dvCaTruc) const {
    ThongKeKyVM kq;
    kq.tieuDeKy = tieuDeKy;
    kq.tuNgay = tuNgayISO;
    kq.denNgay = denNgayISO;
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();

    // 1. Quét hóa đơn và vé trong dsHoaDon
    std::map<std::string, std::pair<int, double>> mapPhim;

    Nut<HoaDon *> *curHD = dsHoaDon.layDau();
    while (curHD != nullptr) {
      HoaDon *hd = curHD->duLieu;
      if (hd != nullptr) {
        std::string ngayLapISO = TienIchNgay::chuanHoaISO(hd->layNgayLap());
        if (ngayLapISO >= tuNgayISO && ngayLapISO <= denNgayISO) {
          kq.tongSoHoaDon++;
          kq.tongSoVe += hd->laySoLuongVe();
          kq.tongDoanhThuVe += hd->layTongTien();

          Nut<Ve> *curVe = hd->layDanhSachVe().layDau();
          while (curVe != nullptr) {
            std::string tp = curVe->duLieu.layTenPhim();
            double gv = curVe->duLieu.layGiaVe();
            mapPhim[tp].first += 1;
            mapPhim[tp].second += gv;
            curVe = curVe->tiepTheo;
          }
        }
      }
      curHD = curHD->tiepTheo;
    }

    // Chuyển mapPhim sang danh sách và sắp xếp giảm dần theo doanh thu
    for (const auto &pair : mapPhim) {
      PhimDoanhThuVM pvm;
      pvm.tenPhim = pair.first;
      pvm.soVe = pair.second.first;
      pvm.doanhThu = pair.second.second;
      pvm.tyTrong = (kq.tongDoanhThuVe > 0) ? (pvm.doanhThu / kq.tongDoanhThuVe * 100.0) : 0.0;
      kq.dsPhim.push_back(pvm);
    }
    std::sort(kq.dsPhim.begin(), kq.dsPhim.end(), [](const PhimDoanhThuVM &a, const PhimDoanhThuVM &b) {
      if (a.doanhThu != b.doanhThu) return a.doanhThu > b.doanhThu;
      return a.soVe > b.soVe;
    });

    // 2. Quét ca trực và tính chi phí nhân sự trong kỳ
    // Chỉ tính các ca trong kỳ và <= ngayHienTai (ca tương lai chưa làm việc không phát sinh lương)
    double donGiaCa = dvCaTruc.layDonGiaCa();
    for (const auto &ca : dvCaTruc.layDanhSachCa()) {
      std::string ngayCa = ca.layNgay();
      if (ngayCa >= tuNgayISO && ngayCa <= denNgayISO && ngayCa <= ngayHienTai) {
        kq.tongSoCaTrongKy++;
        if (ca.layTrangThaiLuong() == 1) {
          kq.soCaDaTra++;
          kq.tienLuongDaTra += donGiaCa;
        } else {
          kq.soCaChuaTra++;
          kq.tienLuongChuaTra += donGiaCa;
        }
      }
    }
    kq.tongChiPhiLuong = kq.tienLuongDaTra + kq.tienLuongChuaTra;

    // 3. Hiệu quả tài chính
    kq.loiNhuanRong = kq.tongDoanhThuVe - kq.tongChiPhiLuong;
    kq.tySuatLoiNhuan = (kq.tongDoanhThuVe > 0) ? (kq.loiNhuanRong / kq.tongDoanhThuVe * 100.0) : 0.0;
    kq.dongTienThucTe = kq.tongDoanhThuVe - kq.tienLuongDaTra;

    return kq;
  }

  // Lấy ngày sớm nhất có phát sinh dữ liệu (hóa đơn bán vé hoặc ca trực thực tế)
  std::string layNgayDauTienCoDuLieuISO(const DichVuCaTruc &dvCaTruc) const {
    std::string minDate = "";
    Nut<HoaDon *> *curHD = dsHoaDon.layDau();
    while (curHD != nullptr) {
      if (curHD->duLieu != nullptr) {
        std::string ngayLapISO = TienIchNgay::chuanHoaISO(curHD->duLieu->layNgayLap());
        if (!ngayLapISO.empty()) {
          if (minDate.empty() || ngayLapISO < minDate) {
            minDate = ngayLapISO;
          }
        }
      }
      curHD = curHD->tiepTheo;
    }

    for (const auto &ca : dvCaTruc.layDanhSachCa()) {
      std::string ngayCa = ca.layNgay();
      if (!ngayCa.empty()) {
        if (minDate.empty() || ngayCa < minDate) {
          minDate = ngayCa;
        }
      }
    }

    if (minDate.empty()) {
      minDate = TienIchNgay::layNgayHienTaiISO();
    }
    return minDate;
  }

  // Kiểm tra chung cho UI và dịch vụ; không cho đi tới thanh toán suất đã bắt đầu.
  bool kiemTraThoiGianBan(const SuatChieu *sc) const {
    if (!sc) return false;
    const auto homNay = TienIchNgay::layNgayHienTaiISO();
    const auto gioHienTai = TienIchNgay::layGioHienTai();
    if (sc->layNgayChieu() < homNay ||
        (sc->layNgayChieu() == homNay && sc->layGioChieu() <= gioHienTai)) {
      TienIchConsole::hienThiLoi("Suất chiếu " + sc->layGioChieu() + " ngày " +
          TienIchNgay::dinhDangHienThi(sc->layNgayChieu()) +
          " đã bắt đầu hoặc đã qua. Không thể đặt vé. Vui lòng chọn suất tương lai.");
      return false;
    }
    return true;
  }

  // Nghiệp vụ đặt nhiều vé cùng lúc trong 1 giao dịch: Đảm bảo tính nguyên tử (All-or-Nothing)
  bool datNhieuVe(const std::string &maSCInput,
                  const std::vector<std::pair<int, int>> &dsGhe,
                  const std::string &tenNV, const std::string &ngayLap,
                   const std::string &maNV, const std::string &maCa, const std::string &gioLap,
                   bool inHoaDon = true) {
    if (khoaGhi || !daNapHoaDon || dsGhe.empty() || maNV.empty() ||
        !TienIchNgay::kiemTraDinhDangGio(gioLap) ||
        TienIchNgay::chuanHoaISO(ngayLap) != TienIchNgay::layNgayHienTaiISO()) return false;
    if (demHD == std::numeric_limits<int>::max() ||
        dsGhe.size() > static_cast<size_t>(std::numeric_limits<int>::max() - demVe)) return false;

    SuatChieu *sc = timSuatChieu(maSCInput);
    if (sc == nullptr) {
      TienIchConsole::hienThiLoi("Không tìm thấy suất chiếu!");
      return false;
    }
    if (!kiemTraThoiGianBan(sc)) return false;
    PhongChieu *phong = sc->layPhongChieu();
    if (phong == nullptr) {
      TienIchConsole::hienThiLoi("Lỗi phòng chiếu không khả dụng!");
      return false;
    }

    // 1. Kiểm tra tính khả dụng: Toàn bộ ghế phải còn trống
    std::set<std::pair<int,int>> duyNhat;
    for (const auto &g : dsGhe) {
      if (g.first < 0 || g.second < 0 || g.first >= phong->laySoHang() ||
          g.second >= phong->laySoCot() || !duyNhat.insert(g).second) return false;
      if (phong->kiemTraGheDaDat(g.first, g.second)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Ghế [%c%d] đã có người đặt trước!",
                 (char)('A' + g.first), g.second + 1);
        TienIchConsole::hienThiLoi(buf);
        return false;
      }
    }

    const auto gheCu = phong->xuatChuoiTrangThaiGhe();
    const int veCu = demVe, hdCu = demHD;
    // 2. Tạo 1 hóa đơn duy nhất cho cả lượt đặt
    demHD++;
    std::string tientoHD = (demHD < 10) ? "HD00" : (demHD < 100 ? "HD0" : "HD");
    std::string maHD = tientoHD + std::to_string(demHD);
    HoaDon *hd = new HoaDon(maHD, ngayLap, tenNV, maNV, maCa, gioLap);

    // 3. Đặt từng ghế và tạo các vé tương ứng
    for (const auto &g : dsGhe) {
      if (!phong->datGhe(g.first, g.second)) {
        phong->napChuoiTrangThaiGhe(gheCu); demVe = veCu; demHD = hdCu; delete hd; return false;
      }

      char chuHang = 'A' + g.first;
      std::string maGhe = std::string(1, chuHang) + std::to_string(g.second + 1);
      double giaVe = phong->layGiaGhe(g.first, g.second);

      demVe++;
      std::string tienTo = (demVe < 10) ? "VE00" : (demVe < 100 ? "VE0" : "VE");
      std::string maVe = tienTo + std::to_string(demVe);

      Ve ve(maVe, sc->layMaSuatChieu(), sc->layTenPhim(), phong->layTenPhong(),
            TienIchNgay::dinhDangHienThi(sc->layNgayChieu()), sc->layGioChieu(), maGhe, giaVe);
      hd->themVe(ve);
    }

    dsHoaDon.themCuoi(hd);

    // 4. Lưu bền vững ngay vào file dulieu/suatchieu.txt và dulieu/hoadon.txt
    if (!luuGiaoDich()) {
      phong->napChuoiTrangThaiGhe(gheCu);
      dsHoaDon.xoaTheoGiaTri(hd); delete hd;
      demVe = veCu; demHD = hdCu;
      TienIchConsole::hienThiLoi("Luu giao dich that bai. Neu phuc hoi loi, dung ban ve va kiem tra tep .bak.");
      return false;
    }

    // 5. In hóa đơn lên console
    if (inHoaDon) hd->inHoaDon();
    return true;
  }

  // Hai file: stage trước; backup trước publish; lỗi phục hồi khóa bán vé.
  // Không cam kết atomic qua mất điện/crash. Giữ .bak để đối soát thủ công.
  bool luuGiaoDich() {
    namespace fs = std::filesystem;
    if (khoaGhi) return false;
    const std::string a = duongDanFile, b = duongDanFileHD;
    if (a == b) return false;
    if (!QuanLyFile::ghiDanhSachSuatChieu(a + ".tmp", dsSuatChieu) ||
        !luuToanBoHoaDon(b + ".tmp")) return false;
    std::error_code ec;
    bool ea = fs::exists(a, ec); if (ec) return false;
    bool eb = fs::exists(b, ec); if (ec) return false;
    if (ea) { fs::copy_file(a, a + ".bak", fs::copy_options::overwrite_existing, ec); if (ec) return false; }
    if (eb) { fs::copy_file(b, b + ".bak", fs::copy_options::overwrite_existing, ec); if (ec) return false; }
    auto publish = [](const std::string &src, const std::string &dst) {
      return MoveFileExA(src.c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    };
    std::ofstream marker(a + ".blocked"); marker << "Kiem tra .bak truoc khi mo lai ghi\n"; marker.close();
    if (marker.fail()) return false;
    if (!publish(a + ".tmp", a)) { fs::remove(a + ".blocked", ec); return false; }
    if (publish(b + ".tmp", b)) { fs::remove(a + ".blocked", ec); if(ec) khoaGhi=true; return true; }
    if (!phucHoiLich(a, ea)) khoaGhi = true;
    else { fs::remove(a + ".blocked", ec); if(ec) khoaGhi=true; }
    return false;
  }

protected:
  // Điểm thay thế I/O để kiểm chứng lỗi phục hồi bằng fixture kế thừa.
  virtual bool phucHoiLich(const std::string &path, bool existed) {
    std::error_code ec;
    if (existed) std::filesystem::copy_file(path + ".bak", path, std::filesystem::copy_options::overwrite_existing, ec);
    else std::filesystem::remove(path, ec);
    return !ec;
  }
public:
  virtual ~DichVuDatVe() {
    Nut<SuatChieu *> *hienTai = dsSuatChieu.layDau();
    while (hienTai != nullptr) {
      delete hienTai->duLieu;
      hienTai = hienTai->tiepTheo;
    }
    Nut<HoaDon *> *hienTaiHD = dsHoaDon.layDau();
    while (hienTaiHD != nullptr) {
      delete hienTaiHD->duLieu;
      hienTaiHD = hienTaiHD->tiepTheo;
    }
  }
};

#endif // DICHVUDATVE_H
