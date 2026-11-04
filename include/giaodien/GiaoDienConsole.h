#ifndef GIAODIENCONSOLE_H
#define GIAODIENCONSOLE_H

#include "../cautruc/DanhSachLienKet.h"
#include "../dichvu/DichVuCaTruc.h"
#include "../dichvu/DichVuDatVe.h"
#include "../dichvu/DichVuLichMau.h"
#include "../dichvu/DichVuPhim.h"
#include "../luutru/QuanLyFile.h"
#include "../models/NguoiDung.h"
#include "../tienich/TienIchNgay.h"
#include "TienIchConsole.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class GiaoDienConsole {
private:
  DichVuPhim dvPhim;
  DichVuDatVe dvDatVe;
  DichVuCaTruc dvCaTruc;
  DichVuLichMau dvLichMau;
  DanhSachLienKet<NguoiDung *> dsNguoiDung;
  NguoiDung *nguoiDungHienTai;

public:
  GiaoDienConsole(const GiaoDienConsole&) = delete;
  GiaoDienConsole& operator=(const GiaoDienConsole&) = delete;
  GiaoDienConsole() : nguoiDungHienTai(nullptr) {
    QuanLyFile::docDanhSachPhim("dulieu/phim.txt", dvPhim.layDanhSach());
    QuanLyFile::docDanhSachNguoiDung("dulieu/nguoidung.txt", dsNguoiDung);

    // Khởi tạo ca trực và đối soát doanh thu
    dvCaTruc.khoiTaoLichCa("dulieu/catruc.txt", dsNguoiDung);

    // Nạp lịch tuần chuẩn cho rạp
    std::vector<std::string> dsMaNV;
    Nut<NguoiDung *> *node = dsNguoiDung.layDau();
    while (node != nullptr) {
      if (node->duLieu->layMaVaiTro() == "NHANVIEN") {
        dsMaNV.push_back(node->duLieu->layMa());
      }
      node = node->tiepTheo;
    }
    dvLichMau.docDanhSach("dulieu/catruc_mau.txt", dsMaNV);

    // Nạp lịch chiếu bền vững từ file suatchieu.txt (nếu chưa có sẽ tự sinh 7 ngày)
    dvDatVe.khoiTaoLichChieu("dulieu/suatchieu.txt", dvPhim.layDanhSach());

    // Nạp lịch sử hóa đơn và vé bán từ file hoadon.txt
    dvDatVe.docDanhSachHoaDon("dulieu/hoadon.txt");
  }

  ~GiaoDienConsole() {
    Nut<NguoiDung *> *hienTai = dsNguoiDung.layDau();
    while (hienTai != nullptr) {
      delete hienTai->duLieu;
      hienTai = hienTai->tiepTheo;
    }
  }

  bool dangNhap() {
    std::string tk, mk;
    TienIchConsole::xoaManHinh();
    TienIchConsole::inBanner();

    printf("\n");
    if (!TienIchConsole::nhapDangNhapTrongKhung(tk, mk)) {
      return false;
    }

    Nut<NguoiDung *> *hienTai = dsNguoiDung.layDau();
    while (hienTai != nullptr) {
      if (hienTai->duLieu->kiemTraDangNhap(tk, mk)) {
        nguoiDungHienTai = hienTai->duLieu;
        TienIchConsole::hienThiThanhCong(
            std::string("Xin chào: ") + nguoiDungHienTai->layHoTen() + " (" +
            nguoiDungHienTai->layVaiTro() + ")");
        Sleep(1500);
        return true;
      }
      hienTai = hienTai->tiepTheo;
    }

    TienIchConsole::hienThiLoi("Sai tài khoản hoặc mật khẩu!");
    Sleep(1500);
    return false;
  }

  // Form nhập liệu 7 bước thêm phim mới hỗ trợ Vòng đời rạp phim (Theatrical Lifecycle)
  void formThemPhimMoi() {
    std::string tenP = "", theLoai = "", daoDien = "";
    int thoiLuong = 0;
    int trangThai = 1;
    std::string ngayKC = TienIchNgay::layNgayHienTaiISO();
    int soNgayChieu = 30;
    std::string maP = dvPhim.sinhMaPhimTuDong();

    auto veForm = [&](int buocHienTai) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("THÊM PHIM MỚI");
      TienIchConsole::veKhungGiua();
      std::vector<TienIchConsole::DoanMau> dm;
      dm.push_back(TienIchConsole::DoanMau("Mã phim tự động cấp: ", TienIchConsole::MAU_MAC_DINH));
      dm.push_back(TienIchConsole::DoanMau("[" + maP + "]", TienIchConsole::MAU_MA_PHIM));
      TienIchConsole::veDongNhieuMau(dm);
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Mã phim: ");
      TienIchConsole::inChuCoMau("[" + maP + "]", TienIchConsole::MAU_MA_PHIM);
      printf(" - Hệ thống tự động sinh\n");

      if (buocHienTai > 1) {
        printf("\t\t>> Tên phim: %s\n", tenP.c_str());
      }
      if (buocHienTai > 2) {
        printf("\t\t>> Thể loại: %s\n", theLoai.c_str());
      }
      if (buocHienTai > 3) {
        printf("\t\t>> Đạo diễn: %s\n", daoDien.c_str());
      }
      if (buocHienTai > 4) {
        printf("\t\t>> Thời lượng: %d phút\n", thoiLuong);
      }
      if (buocHienTai > 5) {
        printf("\t\t>> Trạng thái: %s\n",
               trangThai == 1 ? "Đang chiếu" : (trangThai == 2 ? "Sắp chiếu" : "Ngừng chiếu"));
      }
      if (buocHienTai > 6) {
        printf("\t\t>> Khởi chiếu: %s\n", TienIchNgay::dinhDangHienThi(ngayKC).c_str());
      }
    };

    bool huy = false;

    // Bước 1: Tên phim
    veForm(1);
    if (!TienIchConsole::nhapChuoiGioiHan(tenP, 45, "\t\tNhập tên phim",
        QuanLyFile::kiemTraTenPhimHopLe,
        "Trạng thái: [LỖI: Tên phim phải chứa chữ hoặc số hợp lệ!]")) {
      huy = true;
    } else {
      tenP = QuanLyFile::khuKyTuPhanCach(tenP);
    }
    if (huy) {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }

    // Bước 2: Thể loại
    veForm(2);
    if (!TienIchConsole::nhapChuoiGioiHan(theLoai, 30, "\t\tNhập thể loại",
        QuanLyFile::kiemTraTheLoaiHopLe,
        "Trạng thái: [LỖI: Thể loại phải chứa chữ cái, không được là số thuần túy!]")) {
      huy = true;
    } else {
      theLoai = QuanLyFile::khuKyTuPhanCach(theLoai);
    }
    if (huy) {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }

    // Bước 3: Đạo diễn
    veForm(3);
    if (!TienIchConsole::nhapChuoiGioiHan(daoDien, 25, "\t\tNhập đạo diễn",
        QuanLyFile::kiemTraDaoDienHopLe,
        "Trạng thái: [LỖI: Đạo diễn phải chứa chữ cái, không được là số thuần túy!]")) {
      huy = true;
    } else {
      daoDien = QuanLyFile::khuKyTuPhanCach(daoDien);
    }
    if (huy) {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }

    // Bước 4: Thời lượng
    while (true) {
      veForm(4);
      printf("\t\tNhập thời lượng [1 - 600 phút] hoặc [0] để hủy: ");
      if (!TienIchConsole::docSoNguyen(thoiLuong)) {
        TienIchConsole::hienThiLoiTamThoi("Thời lượng phải là số nguyên dương từ 1 đến 600 phút!", 1500);
        continue;
      }
      if (thoiLuong == 0) { huy = true; break; }
      if (thoiLuong < 1 || thoiLuong > 600) {
        TienIchConsole::hienThiLoiTamThoi("Thời lượng phải nằm trong khoảng từ 1 đến 600 phút!", 1500);
        continue;
      }
      break;
    }
    if (huy) {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }

    // Bước 5: Trạng thái & Kiểm tra hạn ngạch rạp
    veForm(5);
    int soDangChieu = dvPhim.demSoPhimTheoTrangThai(1);
    int soSapChieu = dvPhim.demSoPhimTheoTrangThai(2);
    char quotaInfo[128];
    snprintf(quotaInfo, sizeof(quotaInfo), " [Hiện có: %d/%d Đang chiếu, %d/%d Sắp chiếu]",
             soDangChieu, DichVuPhim::GIOI_HAN_DANG_CHIEU,
             soSapChieu, DichVuPhim::GIOI_HAN_SAP_CHIEU);
    std::cout << "\t\tHạn ngạch cụm rạp:" << quotaInfo << "\n";
    std::cout << "\t\tChọn trạng thái [";
    TienIchConsole::inChuCoMau("1: Đang chiếu", TienIchConsole::MAU_DANG_CHIEU);
    std::cout << ", ";
    TienIchConsole::inChuCoMau("2: Sắp chiếu", TienIchConsole::MAU_SAP_CHIEU);
    std::cout << ", ";
    TienIchConsole::inChuCoMau("0: Ngừng chiếu", TienIchConsole::MAU_NGUNG_CHIEU);
    std::cout << ", mặc định: 1, nhập 'q' hoặc [0] để hủy]: ";
    std::string strTt;
    std::getline(std::cin, strTt);
    strTt = TienIchConsole::catKhoangTrang(strTt);
    if (strTt == "q" || strTt == "Q" || strTt == "0" || strTt == "huy" || strTt == "exit") {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }
    if (!strTt.empty()) {
      try {
        int parsed = std::stoi(strTt);
        if (parsed == 0 || parsed == 1 || parsed == 2) trangThai = parsed;
      } catch (...) {}
    }

    std::string loiGioiHan;
    if (!dvPhim.kiemTraGioiHanTrangThai(trangThai, loiGioiHan)) {
      TienIchConsole::hienThiLoi(loiGioiHan);
      Sleep(2000);
      return;
    }

    // Bước 6: Ngày khởi chiếu
    veForm(6);
    std::cout << "\t\tNhập ngày khởi chiếu DD/MM/YYYY, nhấn Enter lấy hôm nay "
              << TienIchNgay::dinhDangHienThi(ngayKC) << ", hoặc [0] để hủy: ";
    std::string strNgay;
    std::getline(std::cin, strNgay);
    strNgay = TienIchConsole::catKhoangTrang(strNgay);
    if (strNgay == "0" || strNgay == "q" || strNgay == "Q" || strNgay == "huy") {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }
    if (!strNgay.empty()) {
      std::string parsedISO = TienIchNgay::chuanHoaISO(QuanLyFile::trim(strNgay));
      std::tm tmTest;
      if (TienIchNgay::phanTichISO(parsedISO, tmTest)) {
        ngayKC = parsedISO;
      }
    }

    // Bước 7: Số ngày trình chiếu
    veForm(7);
    std::cout << "\t\tNhập số ngày chiếu tại rạp [1 - 365], nhấn Enter lấy 30 ngày, hoặc [0] để hủy: ";
    std::string strSn;
    std::getline(std::cin, strSn);
    strSn = TienIchConsole::catKhoangTrang(strSn);
    if (strSn == "0" || strSn == "q" || strSn == "Q" || strSn == "huy") {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }
    if (!strSn.empty()) {
      try {
        int parsedSn = std::stoi(strSn);
        if (parsedSn > 0 && parsedSn <= 365) soNgayChieu = parsedSn;
      } catch (...) {}
    }

    printf("\n\t\tXác nhận thêm phim [%s] %s vào hệ thống? [1/Y]: Đồng ý  |  [0/Khác]: Hủy bỏ: ",
           maP.c_str(), tenP.c_str());
    int xn = TienIchConsole::docPhimDon();
    if (xn != '1' && xn != 'y' && xn != 'Y') {
      TienIchConsole::hienThiThanhCongTamThoi("Đã hủy thêm phim!", 800);
      return;
    }

    Phim phimMoi(maP, tenP, theLoai, daoDien, thoiLuong, trangThai, ngayKC, soNgayChieu);
    if (dvPhim.themPhim(phimMoi)) {
      QuanLyFile::ghiDanhSachPhim("dulieu/phim.txt", dvPhim.layDanhSach());
      dvDatVe.sinhLichChieuTuDong(dvPhim.layDanhSach(), DichVuDatVe::soNgayPhuLich());
      dvDatVe.luuLichChieu();
      TienIchConsole::hienThiThanhCong("Thêm phim thành công! Mã phim được cấp: [" + maP + "]");
    } else {
      TienIchConsole::hienThiLoi("Thêm phim thất bại - Mã đã tồn tại hoặc dữ liệu vi phạm!");
    }
    Sleep(1200);
  }

  // Đặt vé tương tác trực tiếp
  void thucHienDatVe(const std::string &maSCGoiY = "") {
    std::string maSC = maSCGoiY;
    if (maSC.empty()) {
      printf("\n\t\tNhập mã suất chiếu - ví dụ: 01, 30-01 - hoặc [0] để Quay lại: ");
      std::getline(std::cin, maSC);
      std::string sTrim = TienIchConsole::trim(maSC);
      if (sTrim == "0" || sTrim.empty() || sTrim == "q" || sTrim == "Q" || sTrim == "huy") {
        return;
      }
    }

    SuatChieu *sc = dvDatVe.timSuatChieu(maSC);
    if (sc == nullptr) {
      TienIchConsole::hienThiLoi("Không tìm thấy suất chiếu nào khớp!");
      Sleep(1000);
      return;
    }

    if (!dvDatVe.kiemTraThoiGianBan(sc)) {
      TienIchConsole::choPhimBatKy();
      return;
    }

    PhongChieu *phong = sc->layPhongChieu();
    if (phong == nullptr) {
      TienIchConsole::hienThiLoi("Lỗi phòng chiếu không khả dụng!");
      Sleep(1000);
      return;
    }

    std::vector<std::pair<int, int>> dsGheChon;
    while (true) {
      TienIchConsole::xoaManHinh();
      phong->hienThiSoDo();

      // Hộp thông tin tóm tắt suất chiếu
      printf("\n\t\t--------------------------------------------------");
      printf("\n\t\tĐANG ĐẶT VÉ : [%s] %s", sc->layMaSuatChieu().c_str(), sc->layTenPhim().c_str());
      printf("\n\t\tSuất chiếu   : %s  |  Ngày: %s",
             sc->layGioChieu().c_str(),
             TienIchNgay::dinhDangHienThi(sc->layNgayChieu()).c_str());
      printf("\n\t\tPhòng chiếu  : %s", phong->layTenPhong().c_str());
      printf("\n\t\tQuy tắc rạp  : Đặt tối đa 8 ghế LIỀN KỀ - ví dụ: B3, B3 B4, A3 A4 B4");
      printf("\n\t\t--------------------------------------------------");

      printf("\n\t\tNhập mã ghế muốn đặt - ví dụ: a2 a3 a4 b4 - hoặc [0] để HỦY BỎ: ");
      std::string inputGhe;
      std::getline(std::cin, inputGhe);

      dsGheChon.clear();
      std::string thongBaoLoi;
      int res = TienIchConsole::phanTichDanhSachMaGhe(inputGhe, dsGheChon, thongBaoLoi,
                                                     phong->laySoHang(), phong->laySoCot(),
                                                     TienIchConsole::MAX_VE_MAC_DINH);
      if (res == 0) {
        // Tín hiệu hủy bỏ từ người dùng (nhập 0, q, huy, exit)
        TienIchConsole::hienThiThanhCong("Đã hủy thao tác đặt vé! Dữ liệu được bảo toàn nguyên vẹn.");
        Sleep(1000);
        return;
      }

      if (res == -1) {
        TienIchConsole::hienThiLoiTamThoi(thongBaoLoi, 1800);
        continue;
      }

      // Kiểm tra xem có ghế nào trong danh sách đã có người đặt chưa
      bool biTrung = false;
      for (const auto &g : dsGheChon) {
        if (phong->kiemTraGheDaDat(g.first, g.second)) {
          char errBuf[128];
          snprintf(errBuf, sizeof(errBuf), "Ghế [%c%d] đã có người đặt trước! Vui lòng chọn dãy ghế khác.",
                   (char)('A' + g.first), g.second + 1);
          TienIchConsole::hienThiLoiTamThoi(errBuf, 1800);
          biTrung = true;
          break;
        }
      }
      if (biTrung) continue;

      // Tất cả ghế đều hợp lệ, liền kề và còn trống!
      break;
    }

    // Bước xác nhận đặt vé trước khi xuất hóa đơn
    std::string chuoiDanhSachGhe = "";
    double tongTienDuKien = 0.0;
    for (size_t i = 0; i < dsGheChon.size(); i++) {
      if (i > 0) chuoiDanhSachGhe += ", ";
      char chuH = (char)('A' + dsGheChon[i].first);
      chuoiDanhSachGhe += std::string(1, chuH) + std::to_string(dsGheChon[i].second + 1);
      tongTienDuKien += phong->layGiaGhe(dsGheChon[i].first, dsGheChon[i].second);
    }

    printf("\n\t\t==================================================");
    printf("\n\t\tXÁC NHẬN GIAO DỊCH:");
    printf("\n\t\t  • Phim       : %s", sc->layTenPhim().c_str());
    printf("\n\t\t  • Suất chiếu : %s - %s", sc->layGioChieu().c_str(), TienIchNgay::dinhDangHienThi(sc->layNgayChieu()).c_str());
    printf("\n\t\t  • Phòng chiếu: %s", phong->layTenPhong().c_str());
    printf("\n\t\t  • Ghế đã chọn: %s - %zu vé", chuoiDanhSachGhe.c_str(), dsGheChon.size());
    printf("\n\t\t  • TỔNG TIỀN  : %s VNĐ", TienIchConsole::dinhDangTien(tongTienDuKien).c_str());
    printf("\n\t\t==================================================");
    printf("\n\t\tXác nhận thanh toán & in hóa đơn? [1/Y]: Đồng ý  |  [0/Khác]: Hủy bỏ: ");

    int xn = TienIchConsole::docPhimDon();
    if (xn != '1' && xn != 'y' && xn != 'Y') {
      TienIchConsole::hienThiThanhCong("Đã hủy thao tác đặt vé! Ghế vẫn giữ trạng thái trống.");
      Sleep(1000);
      return;
    }

    // Xóa sạch toàn bộ màn hình cũ (sơ đồ ghế & hộp xác nhận) để in hóa đơn trên Viewport riêng biệt, sạch sẽ
    TienIchConsole::xoaManHinh();

    std::string ngayLap = TienIchNgay::dinhDangHienThi(TienIchNgay::layNgayHienTaiISO());
    std::string tenNV = (nguoiDungHienTai != nullptr) ? nguoiDungHienTai->layHoTen() : "Nhân viên quầy";

    if (dvDatVe.datNhieuVe(sc->layMaSuatChieu(), dsGheChon, tenNV, ngayLap)) {
      TienIchConsole::hienThiThanhCong("Giao dịch đặt vé hoàn tất! Hóa đơn đã được lập thành công.");
    }
    TienIchConsole::choPhimBatKy();
  }

  // Xem lịch chiếu tương tác theo tuần rạp (Cinema Week) & Thứ trong tuần
  // cheDoAdmin = true: Xem giám sát công suất phòng/ghế, không cho phép đặt vé
  // cheDoAdmin = false: Dành cho nhân viên thu ngân bán vé
  void xemLichChieuInteractive(bool cheDoAdmin = false) {
    int offsetTuan = 0; // 0: Tuần hiện tại, 1: Tuần tiếp theo

    while (true) {
      TienIchConsole::xoaManHinh();
      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      int thuHienTai = TienIchNgay::layChiSoThu(ngayHienTai); // 0=CN, 1=T2, ..., 6=T7

      std::vector<std::string> dsNgayTrongTuan;

      if (offsetTuan == 0) {
        // Tuần hiện tại: từ hôm nay đến hết Chủ Nhật của tuần
        int soNgayConLai = (thuHienTai == 0) ? 1 : (7 - thuHienTai + 1);
        for (int d = 0; d < soNgayConLai; d++) {
          dsNgayTrongTuan.push_back(TienIchNgay::congNgay(ngayHienTai, d));
        }
      } else {
        // Tuần tiếp theo: từ Thứ 2 đến Chủ Nhật tuần kế tiếp (7 ngày trọn vẹn)
        int ngayDenCN = (thuHienTai == 0) ? 0 : (7 - thuHienTai);
        std::string ngayT2TuanSau = TienIchNgay::congNgay(ngayHienTai, ngayDenCN + 1);
        for (int d = 0; d < 7; d++) {
          dsNgayTrongTuan.push_back(TienIchNgay::congNgay(ngayT2TuanSau, d));
        }
      }

      printf("\n");
      TienIchConsole::veKhungTren();
      std::string tieuDeTuan = (offsetTuan == 0) ? "LỊCH CHIẾU - TUẦN HIỆN TẠI" : "LỊCH CHIẾU - TUẦN TIẾP THEO";
      TienIchConsole::veDongGiua(tieuDeTuan);
      TienIchConsole::veDong("Tự sinh: hôm nay đến " + TienIchNgay::dinhDangHienThi(
          TienIchNgay::congNgay(ngayHienTai,DichVuDatVe::soNgayPhuLich()-1)));
      TienIchConsole::veDong("Ngoài phạm vi này chỉ xem lịch đã có.");
      if (!dsNgayTrongTuan.empty()) {
        std::string phamViTuan = "  Khoảng: " + TienIchNgay::layNhanNgayKemThu(dsNgayTrongTuan.front()) +
                                 " đến " + TienIchNgay::layNhanNgayKemThu(dsNgayTrongTuan.back());
        TienIchConsole::veDong(phamViTuan);
      }
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      for (size_t i = 0; i < dsNgayTrongTuan.size(); i++) {
        char mucNgay[128];
        snprintf(mucNgay, sizeof(mucNgay), "  [%d] Xem lịch: %s",
                 (int)(i + 1),
                 TienIchNgay::layNhanNgayKemThu(dsNgayTrongTuan[i]).c_str());
        TienIchConsole::veDong(mucNgay);
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      if (offsetTuan == 0) {
        TienIchConsole::veDong("[W]: Xem tuần tiếp theo >>");
      } else {
        TienIchConsole::veDong("[Q]: << Quay lại tuần hiện tại");
      }
      TienIchConsole::veDong("[0]: Quay lại Menu chính hoặc Esc");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn [1-%d, W, Q, 0/Esc]: ", (int)dsNgayTrongTuan.size());
      int ch = TienIchConsole::docPhimDon();

      if (ch == '0' || ch == 27) {
        break;
      } else if (ch == 'w' || ch == 'W') {
        offsetTuan = 1;
      } else if (ch == 'q' || ch == 'Q') {
        offsetTuan = 0;
      } else if (ch >= '1' && ch <= '7') {
        int idx = ch - '1';
        if (idx >= 0 && idx < (int)dsNgayTrongTuan.size()) {
          std::string ngayDuocChon = dsNgayTrongTuan[idx];
          TienIchConsole::xoaManHinh();
          dvDatVe.hienThiLichChieuTheoNgay(ngayDuocChon, &dvPhim.layDanhSach());

          if (cheDoAdmin) {
            std::cout << "\n\t\t[1]: Xem sơ đồ & công suất ghế  |  [Khác]: Quay lại: ";
            int luaChon = TienIchConsole::docPhimDon();
            if (luaChon == '1') {
              std::string maSC;
              printf("\n\t\tNhập mã suất chiếu hoặc [0] để quay lại - ví dụ 01: ");
              std::getline(std::cin, maSC);
              maSC = TienIchConsole::catKhoangTrang(maSC);
              if (maSC == "0" || maSC.empty() || maSC == "q" || maSC == "Q" || maSC == "huy") {
                continue;
              }
              SuatChieu *sc = dvDatVe.timSuatChieu(maSC, ngayDuocChon);
              if (sc != nullptr) {
                TienIchConsole::xoaManHinh();
                sc->layPhongChieu()->hienThiSoDo();
                TienIchConsole::choPhimBatKy();
              } else {
                TienIchConsole::hienThiLoi("Không tìm thấy suất chiếu!");
                Sleep(1000);
              }
            }
          } else {
            // Chế độ Nhân viên thu ngân mới có chức năng Đặt vé cho khách
            std::cout << "\n\t\t[1]: Chọn suất & Đặt vé  |  [2]: Xem sơ đồ ghế  |  [Khác]: Quay lại: ";
            int luaChon = TienIchConsole::docPhimDon();
            if (luaChon == '1') {
              std::string maSC;
              printf("\n\t\tNhập mã suất chiếu muốn đặt hoặc [0] để quay lại - ví dụ 01: ");
              std::getline(std::cin, maSC);
              maSC = TienIchConsole::catKhoangTrang(maSC);
              if (maSC == "0" || maSC.empty() || maSC == "q" || maSC == "Q" || maSC == "huy") {
                continue;
              }
              SuatChieu *sc = dvDatVe.timSuatChieu(maSC, ngayDuocChon);
              if (sc != nullptr) {
                thucHienDatVe(sc->layMaSuatChieu());
              } else {
                TienIchConsole::hienThiLoi("Không tìm thấy suất chiếu nào khớp!");
                Sleep(1000);
              }
            } else if (luaChon == '2') {
              std::string maSC;
              printf("\n\t\tNhập mã suất chiếu muốn xem sơ đồ hoặc [0] để quay lại - ví dụ 01: ");
              std::getline(std::cin, maSC);
              maSC = TienIchConsole::catKhoangTrang(maSC);
              if (maSC == "0" || maSC.empty() || maSC == "q" || maSC == "Q" || maSC == "huy") {
                continue;
              }
              SuatChieu *sc = dvDatVe.timSuatChieu(maSC, ngayDuocChon);
              if (sc != nullptr) {
                TienIchConsole::xoaManHinh();
                sc->layPhongChieu()->hienThiSoDo();
                TienIchConsole::choPhimBatKy();
              } else {
                TienIchConsole::hienThiLoi("Không tìm thấy suất chiếu!");
                Sleep(1000);
              }
            }
          }
        }
      }
    }
  }

  // ========================================================
  // CÁC HÀM TIỆN ÍCH & QUẢN LÝ TÀI KHOẢN NHÂN VIÊN (ADMIN)
  // ========================================================
  std::string sinhMaNhanVienTiepTheo() const {
    int maxIndex = 0;
    Nut<NguoiDung *> *node = dsNguoiDung.layDau();
    while (node != nullptr) {
      std::string ma = node->duLieu->layMa();
      if (ma.length() >= 3 && (ma[0] == 'N' || ma[0] == 'n') && (ma[1] == 'V' || ma[1] == 'v')) {
        try {
          int idx = std::stoi(ma.substr(2));
          if (idx > maxIndex) maxIndex = idx;
        } catch (...) {}
      }
      node = node->tiepTheo;
    }
    int nextIdx = maxIndex + 1;
    std::string suffix = (nextIdx < 10) ? ("0" + std::to_string(nextIdx)) : std::to_string(nextIdx);
    return "NV" + suffix;
  }

  bool daTonTaiTaiKhoan(const std::string &tk) const {
    Nut<NguoiDung *> *node = dsNguoiDung.layDau();
    while (node != nullptr) {
      std::string existingTk = node->duLieu->layTaiKhoan();
      if (existingTk.length() == tk.length()) {
        bool trung = true;
        for (size_t i = 0; i < tk.length(); i++) {
          if (std::tolower(static_cast<unsigned char>(existingTk[i])) !=
              std::tolower(static_cast<unsigned char>(tk[i]))) {
            trung = false;
            break;
          }
        }
        if (trung) return true;
      }
      node = node->tiepTheo;
    }
    return false;
  }

  NguoiDung *timNguoiDungTheoMa(const std::string &ma) const {
    Nut<NguoiDung *> *node = dsNguoiDung.layDau();
    while (node != nullptr) {
      std::string existingMa = node->duLieu->layMa();
      if (existingMa.length() == ma.length()) {
        bool match = true;
        for (size_t i = 0; i < ma.length(); i++) {
          if (std::tolower(static_cast<unsigned char>(existingMa[i])) !=
              std::tolower(static_cast<unsigned char>(ma[i]))) {
            match = false;
            break;
          }
        }
        if (match) return node->duLieu;
      }
      node = node->tiepTheo;
    }
    return nullptr;
  }

  std::vector<NguoiDung *> layDanhSachNhanVien() const {
    std::vector<NguoiDung *> ds;
    Nut<NguoiDung *> *node = dsNguoiDung.layDau();
    while (node != nullptr) {
      if (node->duLieu->layMaVaiTro() == "NHANVIEN") {
        ds.push_back(node->duLieu);
      }
      node = node->tiepTheo;
    }
    return ds;
  }

  // Màn hình 1.1: Đối soát và thanh toán lương cho một nhân viên cụ thể
  void manHinhDoiSoatVaThanhToanLuong(NguoiDung *nd) {
    if (nd == nullptr) return;

    int trang = 0;
    bool chiChuaTra = false;
    const int SO_CA_MOI_TRANG = 5;

    while (true) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      std::vector<CaTruc> dsCa = dvCaTruc.layLichSuCaNhanVien(nd->layMa(), chiChuaTra);
      int soCaChuaTra = 0, soCaDaTra = 0;
      double tienChuaTra = 0.0, tienDaTra = 0.0;
      dvCaTruc.tinhLuongNhanVien(nd->layMa(), soCaChuaTra, tienChuaTra, soCaDaTra, tienDaTra);

      int tongSoCa = (int)dsCa.size();
      int tongTrang = (tongSoCa <= 0) ? 1 : ((tongSoCa + SO_CA_MOI_TRANG - 1) / SO_CA_MOI_TRANG);
      if (trang >= tongTrang) trang = tongTrang - 1;
      if (trang < 0) trang = 0;

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("ĐỐI SOÁT VÀ THANH TOÁN LƯƠNG");
      std::string thongTinNV = "Nhân viên: " + nd->layHoTen() + " - Mã: " + nd->layMa();
      TienIchConsole::veDong(thongTinNV);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      if (dsCa.empty()) {
        if (chiChuaTra) {
          TienIchConsole::veDong("  Không có ca làm việc nào đang chờ thanh toán.");
        } else {
          TienIchConsole::veDong("  Chưa có dữ liệu ca làm việc của nhân viên này.");
        }
        TienIchConsole::veDongTrong();
      } else {
        int batDau = trang * SO_CA_MOI_TRANG;
        int ketThuc = std::min(batDau + SO_CA_MOI_TRANG, tongSoCa);

        for (int i = batDau; i < ketThuc; i++) {
          const auto &ca = dsCa[i];
          char b1[128], bGio[128], bThu[128];
          snprintf(b1, sizeof(b1), "  %d. %s: %s",
                   (i + 1),
                   TienIchNgay::dinhDangHienThi(ca.layNgay()).c_str(),
                   ca.layTenCa().c_str());
          TienIchConsole::veDong(b1);

          snprintf(bGio, sizeof(bGio), "     Khung giờ      : %s đến %s",
                   ca.layGioBatDau().c_str(),
                   ca.layGioKetThuc().c_str());
          TienIchConsole::veDong(bGio);

          snprintf(bThu, sizeof(bThu), "     Doanh thu ca   : %s VNĐ",
                   TienIchConsole::dinhDangTien(ca.layDoanhThu()).c_str());
          TienIchConsole::veDong(bThu);

          std::vector<TienIchConsole::DoanMau> cacDoan;
          cacDoan.push_back(TienIchConsole::DoanMau("     Trạng thái     : ", TienIchConsole::MAU_MAC_DINH));
          if (ca.layTrangThaiLuong() == 0) {
            cacDoan.push_back(TienIchConsole::DoanMau("Chưa thanh toán", TienIchConsole::MAU_DO));
          } else {
            cacDoan.push_back(TienIchConsole::DoanMau("Đã thanh toán", TienIchConsole::MAU_DANG_CHIEU));
          }
          TienIchConsole::veDongNhieuMau(cacDoan);

          if (ca.coLamThay()) {
            std::string noteThay = "     * Trực thay cho nhân viên: " + ca.layMaNVPhanCong();
            TienIchConsole::veDong(noteThay);
          }

          if (i + 1 < ketThuc) {
            TienIchConsole::veDuongKe();
          }
        }
      }

      TienIchConsole::veKhungGiua();
      char bNav[128];
      snprintf(bNav, sizeof(bNav), "Trang %d/%d  |  [N]: Trang sau  |  [P]: Trang trước",
               trang + 1, tongTrang);
      TienIchConsole::veDong(bNav);
      if (chiChuaTra) {
        TienIchConsole::veDong("[T]: Đang xem: Chỉ ca chưa thanh toán");
      } else {
        TienIchConsole::veDong("[T]: Đang xem: Toàn bộ lịch sử ca");
      }

      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      char bSummary1[128], bSummary2[128];
      snprintf(bSummary1, sizeof(bSummary1), "  Tổng số ca chưa thanh toán : %d ca", soCaChuaTra);
      snprintf(bSummary2, sizeof(bSummary2), "  Tiền lương cần thanh toán  : %s VNĐ",
               TienIchConsole::dinhDangTien(tienChuaTra).c_str());
      TienIchConsole::veDong(bSummary1);
      TienIchConsole::veDong(bSummary2);

      std::vector<TienIchConsole::DoanMau> doanTT;
      doanTT.push_back(TienIchConsole::DoanMau("  Trạng thái công nợ         : ", TienIchConsole::MAU_MAC_DINH));
      if (soCaChuaTra > 0) {
        doanTT.push_back(TienIchConsole::DoanMau("Chưa thanh toán", TienIchConsole::MAU_DO));
      } else {
        doanTT.push_back(TienIchConsole::DoanMau("Đã thanh toán", TienIchConsole::MAU_DANG_CHIEU));
      }
      TienIchConsole::veDongNhieuMau(doanTT);

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("1. Xác nhận thanh toán lương");
      TienIchConsole::veDong("0. Quay lại");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      if (ch == 'n' || ch == 'N') {
        if (trang + 1 < tongTrang) trang++;
      } else if (ch == 'p' || ch == 'P') {
        if (trang > 0) trang--;
      } else if (ch == 't' || ch == 'T') {
        chiChuaTra = !chiChuaTra;
        trang = 0;
      } else if (ch == '1') {
        if (soCaChuaTra <= 0) {
          TienIchConsole::hienThiLoi("Nhân viên hiện không có ca làm việc nào cần thanh toán!");
          TienIchConsole::choPhimBatKy();
        } else {
          printf("\n\t\tXác nhận thanh toán %s VNĐ cho nhân viên %s? [1/Y]: Đồng ý | [0/Khác]: Hủy: ",
                 TienIchConsole::dinhDangTien(tienChuaTra).c_str(), nd->layHoTen().c_str());
          int xn = TienIchConsole::docPhimDon();
          if (xn == '1' || xn == 'y' || xn == 'Y') {
            dvCaTruc.thanhToanLuongNhanVien(nd->layMa());
            TienIchConsole::hienThiThanhCong("Đã quyết toán và thanh toán lương thành công! Lương cần thanh toán chuyển về 0 VNĐ.");
            Sleep(1000);
          } else {
            TienIchConsole::hienThiThanhCong("Đã hủy thao tác thanh toán lương.");
            Sleep(800);
          }
        }
      } else if (ch == '0' || ch == 27) {
        break;
      }
    }
  }

  // Màn hình 1: Xem chi tiết nhân viên (Profile Card & Hành động)
  void xemChiTietNhanVien(NguoiDung *nd) {
    if (nd == nullptr) return;

    bool dangXem = true;
    while (dangXem) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("THÔNG TIN CHI TIẾT NHÂN VIÊN");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      TienIchConsole::veDong("  Mã nhân viên  : " + nd->layMa());
      TienIchConsole::veDong("  Họ và tên     : " + nd->layHoTen());
      TienIchConsole::veDong("  Tên đăng nhập : " + nd->layTaiKhoan());
      TienIchConsole::veDong("  Vai trò       : " + nd->layVaiTro());
      TienIchConsole::veDong("  Số CCCD       : " + nd->layCCCD());
      TienIchConsole::veDong("  Số điện thoại : " + nd->laySoDienThoai());

      std::string dc = nd->layDiaChi();
      if (dc.length() > 36) {
        std::vector<std::string> dsDong = TienIchConsole::tachDongTuDong(dc, 36);
        for (size_t d = 0; d < dsDong.size(); d++) {
          if (d == 0) {
            TienIchConsole::veDong("  Địa chỉ       : " + dsDong[d]);
          } else {
            TienIchConsole::veDong("                  " + dsDong[d]);
          }
        }
      } else {
        TienIchConsole::veDong("  Địa chỉ       : " + dc);
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("1. Tính và quyết toán lương");
      TienIchConsole::veDong("2. Đổi mật khẩu nhân viên");
      TienIchConsole::veDong("3. Xóa tài khoản nhân viên");
      TienIchConsole::veDong("0. Quay lại danh sách");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1':
        manHinhDoiSoatVaThanhToanLuong(nd);
        break;

      case '2': {
        TienIchConsole::xoaManHinh();
        printf("\n");
        TienIchConsole::veKhungTren();
        TienIchConsole::veDongGiua("ĐỔI MẬT KHẨU NHÂN VIÊN");
        TienIchConsole::veKhungGiua();
        TienIchConsole::veDongTrong();
        TienIchConsole::veDong("  Nhân viên : " + nd->layHoTen() + " [" + nd->layMa() + "]");
        TienIchConsole::veDongTrong();
        TienIchConsole::veKhungDuoi();

        std::string mkMoi;
        std::cout << "\n\t\t>> Nhập mật khẩu mới hoặc [0] để hủy: ";
        std::getline(std::cin, mkMoi);
        mkMoi = TienIchConsole::catKhoangTrang(mkMoi);
        if (mkMoi == "0" || mkMoi.empty()) {
          TienIchConsole::hienThiLoi("Đã hủy đổi mật khẩu.");
          Sleep(800);
          break;
        }

        mkMoi = QuanLyFile::khuKyTuPhanCach(mkMoi);
        nd->doiMatKhau(mkMoi);
        if (QuanLyFile::ghiDanhSachNguoiDung("dulieu/nguoidung.txt", dsNguoiDung)) {
          TienIchConsole::hienThiThanhCong("Đổi mật khẩu tài khoản thành công!");
        }
        TienIchConsole::choPhimBatKy();
        break;
      }

      case '3': {
        if (nd->layMaVaiTro() == "ADMIN" || (nguoiDungHienTai != nullptr && nd->layMa() == nguoiDungHienTai->layMa())) {
          TienIchConsole::hienThiLoi("Bảo vệ hệ thống: Không được phép xóa tài khoản Quản trị viên!");
          TienIchConsole::choPhimBatKy();
          break;
        }

        printf("\n\t\tBạn có chắc muốn xóa nhân viên [%s] %s không? [1/Y]: Đồng ý | [0/Khác]: Hủy: ",
               nd->layMa().c_str(), nd->layHoTen().c_str());
        int xn = TienIchConsole::docPhimDon();
        if (xn == '1' || xn == 'y' || xn == 'Y') {
          dsNguoiDung.xoaTheoGiaTri(nd);
          delete nd;
          QuanLyFile::ghiDanhSachNguoiDung("dulieu/nguoidung.txt", dsNguoiDung);
          TienIchConsole::hienThiThanhCong("Đã xóa tài khoản nhân viên thành công!");
          Sleep(1000);
          dangXem = false;
        } else {
          TienIchConsole::hienThiThanhCong("Đã hủy thao tác xóa nhân viên.");
          Sleep(800);
        }
        break;
      }

      case '0':
      case 27:
        dangXem = false;
        break;

      default:
        break;
      }
    }
  }

  // Màn hình 1: Danh sách nhân viên tương tác trực tiếp (Pagination 5/page)
  void danhSachNhanVienInteractive() {
    int trang = 0;
    const int SO_NV_MOI_TRANG = 5;

    while (true) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      std::vector<NguoiDung *> dsNV = layDanhSachNhanVien();
      int tongNV = (int)dsNV.size();
      int tongTrang = (tongNV <= 0) ? 1 : ((tongNV + SO_NV_MOI_TRANG - 1) / SO_NV_MOI_TRANG);
      if (trang >= tongTrang) trang = tongTrang - 1;
      if (trang < 0) trang = 0;

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("DANH SÁCH TÀI KHOẢN NHÂN VIÊN");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      if (dsNV.empty()) {
        TienIchConsole::veDong("  Hiện chưa có tài khoản nhân viên nào trong hệ thống.");
        TienIchConsole::veDongTrong();
      } else {
        int batDau = trang * SO_NV_MOI_TRANG;
        int ketThuc = std::min(batDau + SO_NV_MOI_TRANG, tongNV);

        for (int i = batDau; i < ketThuc; i++) {
          char buffer[128];
          snprintf(buffer, sizeof(buffer), "  %d. %s: %s - %s",
                   (int)(i - batDau + 1),
                   dsNV[i]->layMa().c_str(),
                   dsNV[i]->layHoTen().c_str(),
                   dsNV[i]->layTaiKhoan().c_str());
          TienIchConsole::veDong(buffer);
        }
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      char bNav[128];
      snprintf(bNav, sizeof(bNav), "Trang %d/%d  |  [N]: Trang sau  |  [P]: Trang trước",
               trang + 1, tongTrang);
      TienIchConsole::veDong(bNav);
      if (!dsNV.empty()) {
        int soHien = std::min(SO_NV_MOI_TRANG, tongNV - trang * SO_NV_MOI_TRANG);
        char bPrompt[128];
        snprintf(bPrompt, sizeof(bPrompt), "Nhấn phím 1-%d để chọn nhân viên, hoặc [0]: Quay lại", soHien);
        TienIchConsole::veDong(bPrompt);
      } else {
        TienIchConsole::veDong("Nhấn [0]: Quay lại");
      }
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      if (ch == '0' || ch == 27) {
        break;
      } else if (ch == 'n' || ch == 'N') {
        if (trang + 1 < tongTrang) trang++;
      } else if (ch == 'p' || ch == 'P') {
        if (trang > 0) trang--;
      } else if (ch >= '1' && ch <= '5') {
        int idx = (ch - '1');
        int viTriThucTe = trang * SO_NV_MOI_TRANG + idx;
        if (viTriThucTe < tongNV) {
          xemChiTietNhanVien(dsNV[viTriThucTe]);
        }
      }
    }
  }

  // Màn hình 2: Thêm nhân viên mới đầy đủ hồ sơ (CCCD, SĐT, Địa chỉ)
  void themNhanVienMoi() {
    TienIchConsole::xoaManHinh();
    printf("\n");
    TienIchConsole::veKhungTren();
    TienIchConsole::veDongGiua("THÊM TÀI KHOẢN NHÂN VIÊN MỚI");
    TienIchConsole::veKhungGiua();
    TienIchConsole::veDongTrong();

    std::string maMoi = sinhMaNhanVienTiepTheo();
    TienIchConsole::veDong("  Mã nhân viên dự kiến : " + maMoi);
    TienIchConsole::veDongTrong();
    TienIchConsole::veKhungDuoi();

    std::string taiKhoan, matKhau, hoTen, cccd, sdt, diaChi;
    std::cout << "\n\t\t>> Nhập tên đăng nhập hoặc [0] để hủy: ";
    std::getline(std::cin, taiKhoan);
    taiKhoan = TienIchConsole::catKhoangTrang(taiKhoan);
    if (taiKhoan == "0" || taiKhoan == "q" || taiKhoan == "Q" || taiKhoan == "huy" || taiKhoan.empty()) {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }

    if (daTonTaiTaiKhoan(taiKhoan)) {
      TienIchConsole::hienThiLoi("Tên đăng nhập đã tồn tại trong hệ thống!");
      TienIchConsole::choPhimBatKy();
      return;
    }

    std::cout << "\t\t>> Nhập mật khẩu ban đầu hoặc [0] để hủy: ";
    std::getline(std::cin, matKhau);
    matKhau = TienIchConsole::catKhoangTrang(matKhau);
    if (matKhau == "0" || matKhau == "q" || matKhau == "Q" || matKhau == "huy") {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }
    if (matKhau.empty()) {
      TienIchConsole::hienThiLoi("Mật khẩu không được để trống!");
      TienIchConsole::choPhimBatKy();
      return;
    }

    std::cout << "\t\t>> Nhập họ và tên nhân viên hoặc [0] để hủy: ";
    std::getline(std::cin, hoTen);
    hoTen = TienIchConsole::catKhoangTrang(hoTen);
    if (hoTen == "0" || hoTen == "q" || hoTen == "Q" || hoTen == "huy") {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }
    if (hoTen.empty()) {
      TienIchConsole::hienThiLoi("Họ tên không được để trống!");
      TienIchConsole::choPhimBatKy();
      return;
    }

    std::cout << "\t\t>> Nhập số CCCD gồm 12 số, Enter để bỏ qua, hoặc [0] để hủy: ";
    std::getline(std::cin, cccd);
    cccd = TienIchConsole::catKhoangTrang(cccd);
    if (cccd == "0" || cccd == "q" || cccd == "Q" || cccd == "huy") {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }
    if (cccd.empty()) cccd = "Chưa cập nhật";

    std::cout << "\t\t>> Nhập số điện thoại, Enter để bỏ qua, hoặc [0] để hủy: ";
    std::getline(std::cin, sdt);
    sdt = TienIchConsole::catKhoangTrang(sdt);
    if (sdt == "0" || sdt == "q" || sdt == "Q" || sdt == "huy") {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }
    if (sdt.empty()) sdt = "Chưa cập nhật";

    std::cout << "\t\t>> Nhập địa chỉ liên lạc, Enter để bỏ qua, hoặc [0] để hủy: ";
    std::getline(std::cin, diaChi);
    diaChi = TienIchConsole::catKhoangTrang(diaChi);
    if (diaChi == "0" || diaChi == "q" || diaChi == "Q" || diaChi == "huy") {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }
    if (diaChi.empty()) diaChi = "Chưa cập nhật";

    printf("\n\t\t==================================================");
    printf("\n\t\tXÁC NHẬN THÊM NHÂN VIÊN MỚI:");
    printf("\n\t\t  • Mã nhân viên : %s", maMoi.c_str());
    printf("\n\t\t  • Tên đăng nhập: %s", taiKhoan.c_str());
    printf("\n\t\t  • Họ và tên    : %s", hoTen.c_str());
    printf("\n\t\t  • Số CCCD      : %s", cccd.c_str());
    printf("\n\t\t  • Số điện thoại: %s", sdt.c_str());
    printf("\n\t\t==================================================");
    printf("\n\t\tXác nhận lưu tài khoản nhân viên? [1/Y]: Đồng ý  |  [0/Khác]: Hủy bỏ: ");
    int xn = TienIchConsole::docPhimDon();
    if (xn != '1' && xn != 'y' && xn != 'Y') {
      TienIchConsole::hienThiLoi("Đã hủy thao tác thêm nhân viên.");
      Sleep(800);
      return;
    }

    taiKhoan = QuanLyFile::khuKyTuPhanCach(taiKhoan);
    matKhau = QuanLyFile::khuKyTuPhanCach(matKhau);
    hoTen = QuanLyFile::khuKyTuPhanCach(hoTen);
    cccd = QuanLyFile::khuKyTuPhanCach(cccd);
    sdt = QuanLyFile::khuKyTuPhanCach(sdt);
    diaChi = QuanLyFile::khuKyTuPhanCach(diaChi);

    NguoiDung *nvMoi = new NhanVien(maMoi, taiKhoan, matKhau, hoTen, cccd, sdt, diaChi);
    dsNguoiDung.themCuoi(nvMoi);

    if (QuanLyFile::ghiDanhSachNguoiDung("dulieu/nguoidung.txt", dsNguoiDung)) {
      TienIchConsole::hienThiThanhCong("Thêm tài khoản nhân viên thành công!");
    } else {
      TienIchConsole::hienThiLoi("Lỗi lưu dữ liệu vào file nguoidung.txt!");
    }
    TienIchConsole::choPhimBatKy();
  }

  // Màn hình 3: Xếp lịch ca trực và đổi ca làm thay
  void xepLichCaTrucVaDoiCa() {
    std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
    int offsetNgay = 0;

    while (true) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      std::string ngayXem = TienIchNgay::congNgay(ngayHienTai, offsetNgay);
      std::vector<CaTruc> dsCa = dvCaTruc.layDanhSachCaTheoNgay(ngayXem);

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("LỊCH CA TRỰC VÀ BÀN GIAO THẾ CA");
      std::string headerNgay = "Ngày: " + TienIchNgay::layChuoiThu(ngayXem) + " - " + TienIchNgay::dinhDangHienThi(ngayXem);
      TienIchConsole::veDong(headerNgay);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      if (dsCa.empty()) {
        TienIchConsole::veDong("  Ngày này chưa có ca trực nào được xếp.");
        TienIchConsole::veDong("  Cửa sổ cuộn 1 tuần: Các tuần xa hơn là lịch trống.");
        TienIchConsole::veDong("  Chọn [4] để xem hoặc chỉnh sửa Lịch tuần chuẩn.");
      } else {
        for (size_t i = 0; i < dsCa.size(); i++) {
          const auto &ca = dsCa[i];
          char b1[128];
          snprintf(b1, sizeof(b1), "  [%d] %s: %s đến %s",
                   (int)(i + 1), ca.layTenCa().c_str(),
                   ca.layGioBatDau().c_str(), ca.layGioKetThuc().c_str());
          TienIchConsole::veDong(b1);

          std::string nvPC = "      Phân công ban đầu : " + ca.layMaNVPhanCong();
          TienIchConsole::veDong(nvPC);

          if (ca.coLamThay()) {
            std::vector<TienIchConsole::DoanMau> dm;
            dm.push_back(TienIchConsole::DoanMau("      Thực tế đứng quầy : ", TienIchConsole::MAU_MAC_DINH));
            dm.push_back(TienIchConsole::DoanMau(ca.layMaNVThucTe() + " - Làm thế ca", TienIchConsole::MAU_SAP_CHIEU));
            TienIchConsole::veDongNhieuMau(dm);
          } else {
            std::string nvTT = "      Thực tế đứng quầy : " + ca.layMaNVThucTe();
            TienIchConsole::veDong(nvTT);
          }

          if (i + 1 < dsCa.size()) {
            TienIchConsole::veDuongKe();
          }
        }
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      std::string ngayKhuyet = dvCaTruc.timNgayKhuyetDauTien(ngayHienTai);
      if (!ngayKhuyet.empty()) {
        std::vector<TienIchConsole::DoanMau> dmn;
        dmn.push_back(TienIchConsole::DoanMau("[N]: Sau | [P]: Trước | ", TienIchConsole::MAU_MAC_DINH));
        dmn.push_back(TienIchConsole::DoanMau("[F]: Nhảy tới ngày khuyết", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dmn);
      } else {
        TienIchConsole::veDong("[N]: Xem ngày kế tiếp  |  [P]: Xem ngày trước đó");
      }
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("1. Đổi nhân viên làm thế ca");
      TienIchConsole::veDong("2. Xếp ca làm việc mới");
      TienIchConsole::veDong("3. Hủy hoặc xóa ca làm việc");
      TienIchConsole::veDong("0. Quay lại");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      if (ch == '0' || ch == 27) {
        break;
      } else if (ch == 'n' || ch == 'N') {
        offsetNgay++;
      } else if (ch == 'p' || ch == 'P') {
        offsetNgay--;
      } else if ((ch == 'f' || ch == 'F') && !ngayKhuyet.empty()) {
        int d = 0;
        std::string tmp = ngayHienTai;
        if (tmp <= ngayKhuyet) {
          while (tmp < ngayKhuyet && d < 60) {
            tmp = TienIchNgay::congNgay(tmp, 1);
            d++;
          }
        } else {
          while (tmp > ngayKhuyet && d > -60) {
            tmp = TienIchNgay::congNgay(tmp, -1);
            d--;
          }
        }
        offsetNgay = d;
      } else if (ch == '1') {
        if (dsCa.empty()) {
          TienIchConsole::hienThiLoi("Ngày này không có ca trực nào để đổi!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::cout << "\n\t\tNhập số thứ tự ca muốn đổi người làm thay [1-" << dsCa.size() << "] hoặc [0] để hủy: ";
        std::string sChon;
        std::getline(std::cin, sChon);
        int idx = 0;
        try { idx = std::stoi(sChon) - 1; } catch (...) { idx = -1; }
        if (sChon == "0" || sChon == "q" || sChon == "Q" || sChon == "huy" || idx < 0 || idx >= (int)dsCa.size()) {
          TienIchConsole::hienThiLoi("Đã hủy thao tác.");
          Sleep(800);
          continue;
        }

        std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
        if (dsCa[idx].layNgay() <= ngayHienTai && dsCa[idx].daThanhToan()) {
          TienIchConsole::hienThiLoi("Ca trực này đã thanh toán lương! Không thể đổi người làm thay.");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::vector<NguoiDung *> dsNV = layDanhSachNhanVien();
        if (dsNV.empty()) {
          TienIchConsole::hienThiLoi("Không có nhân viên thu ngân nào trong hệ thống!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::cout << "\n\t\tChọn nhân viên làm thế ca:\n";
        for (size_t k = 0; k < dsNV.size(); k++) {
          bool biTrung = dvCaTruc.kiemTraXungDotCa(dsCa[idx].layNgay(), dsCa[idx].layGioBatDau(),
                                                  dsCa[idx].layGioKetThuc(), dsNV[k]->layMa(),
                                                  dsCa[idx].layMaCa());
          std::cout << "\t\t  [" << (k + 1) << "] " << dsNV[k]->layMa() << " - " << dsNV[k]->layHoTen();
          if (biTrung) {
            std::cout << " - Đang có ca trùng giờ";
          }
          std::cout << "\n";
        }
        std::cout << "\t\t>> Nhập số thứ tự nhân viên làm thế ca hoặc [0] để hủy: ";
        std::string sNV;
        std::getline(std::cin, sNV);
        sNV = TienIchConsole::catKhoangTrang(sNV);
        if (sNV == "0" || sNV.empty() || sNV == "q" || sNV == "Q" || sNV == "huy") {
          TienIchConsole::hienThiLoi("Đã hủy thao tác đổi người.");
          Sleep(800);
          continue;
        }
        int idxNV = 0;
        try { idxNV = std::stoi(sNV) - 1; } catch (...) { idxNV = -1; }
        if (idxNV >= 0 && idxNV < (int)dsNV.size()) {
          int kq = dvCaTruc.doiNguoiLamThayChiTiet(dsCa[idx].layMaCa(), dsNV[idxNV]->layMa());
          if (kq == 0) {
            TienIchConsole::hienThiThanhCong("Đổi người làm thế ca thành công! Tiền công sẽ được tính cho nhân viên mới.");
          } else if (kq == 2) {
            TienIchConsole::hienThiLoi("Ca trực này đã chốt thanh toán lương! Không thể đổi người.");
          } else if (kq == 3) {
            TienIchConsole::hienThiLoi("Nhân viên này đã có ca trực trùng giờ trong ngày! Không thể đổi.");
          } else {
            TienIchConsole::hienThiLoi("Lỗi đổi người làm thế ca!");
          }
        } else {
          TienIchConsole::hienThiLoi("Lựa chọn nhân viên không hợp lệ!");
        }
        Sleep(1000);
      } else if (ch == '2') {
        std::cout << "\n\t\tChọn loại ca muốn xếp:\n";
        std::cout << "\t\t  [1] Ca Sáng: 08:00 đến 13:30\n";
        std::cout << "\t\t  [2] Ca Chiều: 13:30 đến 18:30\n";
        std::cout << "\t\t  [3] Ca Tối: 18:30 đến 23:30\n";
        std::cout << "\t\t  [4] Ca tùy chọn: Tự đặt tên ca và giờ làm việc\n";
        std::cout << "\t\t  [0] Hủy bỏ\n";
        std::cout << "\t\t>> Chọn ca: ";
        int cCa = TienIchConsole::docPhimDon();
        std::string tenCa, gBD, gKT;
        if (cCa == '1') {
          tenCa = "Ca Sáng"; gBD = "08:00"; gKT = "13:30";
        } else if (cCa == '2') {
          tenCa = "Ca Chiều"; gBD = "13:30"; gKT = "18:30";
        } else if (cCa == '3') {
          tenCa = "Ca Tối"; gBD = "18:30"; gKT = "23:30";
        } else if (cCa == '4') {
          std::cout << "\n\t\t>> Nhập tên ca làm việc hoặc [0] để hủy, Enter lấy mặc định Ca Tăng Cường: ";
          std::getline(std::cin, tenCa);
          tenCa = TienIchConsole::catKhoangTrang(tenCa);
          if (tenCa == "0" || tenCa == "q" || tenCa == "Q" || tenCa == "huy") {
            TienIchConsole::hienThiLoi("Đã hủy thao tác xếp ca.");
            Sleep(800);
            continue;
          }
          if (tenCa.empty()) tenCa = "Ca Tăng Cường";

          std::cout << "\t\t>> Nhập giờ bắt đầu định dạng HH:MM hoặc [0] để hủy - ví dụ 09:00: ";
          std::getline(std::cin, gBD);
          gBD = TienIchConsole::catKhoangTrang(gBD);
          if (gBD == "0" || gBD == "q" || gBD == "Q" || gBD == "huy") {
            TienIchConsole::hienThiLoi("Đã hủy thao tác xếp ca.");
            Sleep(800);
            continue;
          }

          std::cout << "\t\t>> Nhập giờ kết thúc định dạng HH:MM hoặc [0] để hủy - ví dụ 14:00: ";
          std::getline(std::cin, gKT);
          gKT = TienIchConsole::catKhoangTrang(gKT);
          if (gKT == "0" || gKT == "q" || gKT == "Q" || gKT == "huy") {
            TienIchConsole::hienThiLoi("Đã hủy thao tác xếp ca.");
            Sleep(800);
            continue;
          }

          if (!TienIchNgay::kiemTraDinhDangGio(gBD) || !TienIchNgay::kiemTraDinhDangGio(gKT)) {
            TienIchConsole::hienThiLoi("Định dạng giờ không hợp lệ! Vui lòng nhập đúng chuẩn HH:MM.");
            Sleep(1200);
            continue;
          }
          if (TienIchNgay::chuyenGioThanhPhut(gBD) >= TienIchNgay::chuyenGioThanhPhut(gKT)) {
            TienIchConsole::hienThiLoi("Giờ bắt đầu phải trước giờ kết thúc!");
            Sleep(1200);
            continue;
          }
        } else {
          TienIchConsole::hienThiLoi("Đã hủy thao tác xếp ca.");
          Sleep(800);
          continue;
        }

        if (dvCaTruc.kiemTraXungDotKhungGio(ngayXem, gBD, gKT)) {
          TienIchConsole::hienThiLoi("Trùng lịch rạp: Ngày này đã có ca trực trong khung giờ " + gBD + " đến " + gKT + "!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::vector<NguoiDung *> dsNV = layDanhSachNhanVien();
        if (dsNV.empty()) {
          TienIchConsole::hienThiLoi("Chưa có nhân viên thu ngân nào trong hệ thống!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::cout << "\n\t\tChọn nhân viên phân công:\n";
        for (size_t k = 0; k < dsNV.size(); k++) {
          bool biTrung = dvCaTruc.kiemTraXungDotCa(ngayXem, gBD, gKT, dsNV[k]->layMa());
          std::cout << "\t\t  [" << (k + 1) << "] " << dsNV[k]->layMa() << " - " << dsNV[k]->layHoTen();
          if (biTrung) {
            std::cout << " - Đang có ca trùng giờ";
          }
          std::cout << "\n";
        }
        std::cout << "\t\t>> Nhập số thứ tự nhân viên hoặc [0] để hủy: ";
        std::string sNV;
        std::getline(std::cin, sNV);
        sNV = TienIchConsole::catKhoangTrang(sNV);
        if (sNV == "0" || sNV.empty() || sNV == "q" || sNV == "Q" || sNV == "huy") {
          TienIchConsole::hienThiLoi("Đã hủy thao tác xếp ca.");
          Sleep(800);
          continue;
        }
        int idxNV = 0;
        try { idxNV = std::stoi(sNV) - 1; } catch (...) { idxNV = -1; }
        if (idxNV >= 0 && idxNV < (int)dsNV.size()) {
          std::string maCaTaoMoi;
          int kq = dvCaTruc.xepCaMoiChiTiet(ngayXem, tenCa, gBD, gKT, dsNV[idxNV]->layMa(), &maCaTaoMoi);
          if (kq == 0) {
            TienIchConsole::hienThiThanhCong("Xếp ca làm việc mới thành công! Mã ca: " + maCaTaoMoi);
          } else if (kq == 1) {
            TienIchConsole::hienThiLoi("Xung đột lịch trực: Nhân viên này đã có ca trực trùng giờ trong ngày!");
          } else if (kq == 3) {
            TienIchConsole::hienThiLoi("Trùng lịch rạp: Ngày này đã có ca trực trong khung giờ " + gBD + " đến " + gKT + "!");
          } else {
            TienIchConsole::hienThiLoi("Lỗi tham số ca trực không hợp lệ!");
          }
        } else {
          TienIchConsole::hienThiLoi("Lựa chọn nhân viên không hợp lệ!");
        }
        Sleep(1200);
      } else if (ch == '3') {
        if (dsCa.empty()) {
          TienIchConsole::hienThiLoi("Ngày này không có ca trực nào để xóa!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        std::cout << "\n\t\tNhập số thứ tự ca muốn xóa [1-" << dsCa.size() << "] hoặc [0] để hủy: ";
        std::string sChon;
        std::getline(std::cin, sChon);
        int idx = 0;
        try { idx = std::stoi(sChon) - 1; } catch (...) { idx = -1; }
        if (sChon == "0" || idx < 0 || idx >= (int)dsCa.size()) {
          TienIchConsole::hienThiLoi("Đã hủy thao tác xóa ca.");
          Sleep(800);
          continue;
        }

        const auto &caCanXoa = dsCa[idx];
        std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
        if (caCanXoa.layNgay() <= ngayHienTai && caCanXoa.daThanhToan()) {
          TienIchConsole::hienThiLoi("Bảo vệ tài chính: Không được phép xóa ca đã thanh toán lương!");
          TienIchConsole::choPhimBatKy();
          continue;
        }

        if (caCanXoa.layDoanhThu() > 0) {
          printf("\n\t\tCẢNH BÁO: Ca này đã phát sinh doanh thu %s VNĐ! Xác nhận xóa? [1/Y]: Đồng ý | [0/Khác]: Hủy: ",
                 TienIchConsole::dinhDangTien(caCanXoa.layDoanhThu()).c_str());
          int xn = TienIchConsole::docPhimDon();
          if (xn == '1' || xn == 'y' || xn == 'Y') {
            dvCaTruc.xoaCaTruc(caCanXoa.layMaCa(), true);
            TienIchConsole::hienThiThanhCong("Đã xóa ca trực thành công!");
          } else {
            TienIchConsole::hienThiLoi("Đã hủy thao tác xóa ca.");
          }
        } else {
          printf("\n\t\tBạn có chắc muốn xóa ca [%s] %s không? [1/Y]: Đồng ý | [0/Khác]: Hủy: ",
                 caCanXoa.layMaCa().c_str(), caCanXoa.layTenCa().c_str());
          int xn = TienIchConsole::docPhimDon();
          if (xn == '1' || xn == 'y' || xn == 'Y') {
            dvCaTruc.xoaCaTruc(caCanXoa.layMaCa(), false);
            TienIchConsole::hienThiThanhCong("Đã xóa ca trực thành công!");
          } else {
            TienIchConsole::hienThiLoi("Đã hủy thao tác xóa ca.");
          }
        }
        Sleep(1000);
      }
    }
  }

  // Hiển thị ma trận lịch tuần chuẩn 21 ca
  void hienThiMaTranLichMau() {
    int trang = 0;
    while (true) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("MA TRẬN LỊCH CA TRỰC TUẦN CHUẨN");
      char bSub[128];
      snprintf(bSub, sizeof(bSub), "Trang %d/2: %s", trang + 1,
               (trang == 0 ? "Thứ Hai đến Thứ Năm" : "Thứ Sáu đến Chủ Nhật"));
      TienIchConsole::veDong(bSub);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      int thuBatDau = (trang == 0) ? 2 : 6;
      int thuKetThuc = (trang == 0) ? 5 : 8;

      for (int thu = thuBatDau; thu <= thuKetThuc; thu++) {
        std::string tenThu = TienIchNgay::layTenThuDayDu(thu);
        std::string headerThu = "► " + tenThu + ":";
        TienIchConsole::veDong(headerThu);

        const char *caNames[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
        const char *gioCa[3] = {"08:00-13:30", "13:30-18:30", "18:30-23:30"};
        for (int c = 0; c < 3; c++) {
          const CaTrucMau *cm = dvLichMau.timCaMau(thu, caNames[c]);
          std::string maNV = (cm != nullptr) ? cm->layMaNV() : "TRONG";
          std::string tenNV = "";
          if (maNV != "TRONG" && !maNV.empty()) {
            NguoiDung *nd = timNguoiDungTheoMa(maNV);
            if (nd != nullptr) {
              tenNV = " - " + TienIchConsole::catChuoiTheoTu(nd->layHoTen(), 16);
            } else {
              tenNV = " - " + maNV;
            }
          } else {
            tenNV = " - Chưa phân công";
          }

          char bCa[128];
          snprintf(bCa, sizeof(bCa), "  • %-8s [%s]: %s%s", caNames[c], gioCa[c],
                   maNV.c_str(), tenNV.c_str());
          TienIchConsole::veDong(bCa);
        }

        if (thu < thuKetThuc) {
          TienIchConsole::veDuongKe();
        }
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDong("[N]: Trang sau  |  [P]: Trang trước  |  [0/Esc]: Quay lại");
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chuyển trang hoặc quay lại: ");
      int ch = TienIchConsole::docPhimDon();
      if (ch == '0' || ch == 27) {
        break;
      } else if (ch == 'n' || ch == 'N' || ch == 'p' || ch == 'P') {
        trang = 1 - trang;
      }
    }
  }

  // Phân công ca theo thứ tự: Thứ trong tuần -> Ca trực -> Nhân viên
  void phanCongLichMauInteractive() {
    while (true) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("PHÂN CÔNG LỊCH TUẦN CHUẨN");
      TienIchConsole::veDong("Thứ tự thao tác: Ngày trong tuần -> Ca trực -> Nhân viên");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("Bước 1: Chọn ngày trong tuần để xếp lịch:");
      TienIchConsole::veDong("  [2] Thứ Hai");
      TienIchConsole::veDong("  [3] Thứ Ba");
      TienIchConsole::veDong("  [4] Thứ Tư");
      TienIchConsole::veDong("  [5] Thứ Năm");
      TienIchConsole::veDong("  [6] Thứ Sáu");
      TienIchConsole::veDong("  [7] Thứ Bảy");
      TienIchConsole::veDong("  [8] Chủ Nhật");
      TienIchConsole::veDong("  [0] Quay lại");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhập thứ [2-8] hoặc [0] để quay lại: ");
      int cThu = TienIchConsole::docPhimDon();
      if (cThu == '0' || cThu == 27) {
        break;
      }
      if (cThu < '2' || cThu > '8') {
        TienIchConsole::hienThiLoi("Lựa chọn thứ không hợp lệ!");
        Sleep(800);
        continue;
      }
      int thu = cThu - '0';

      // Bước 2: Chọn Ca trực
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("PHÂN CÔNG LỊCH TUẦN CHUẨN");
      std::string infoThu = "Ngày đã chọn: " + TienIchNgay::layTenThuDayDu(thu);
      TienIchConsole::veDong(infoThu);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("Bước 2: Chọn ca trực muốn phân công:");

      const char *caNames[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
      const char *gioCa[3] = {"08:00 đến 13:30", "13:30 đến 18:30", "18:30 đến 23:30"};
      for (int c = 0; c < 3; c++) {
        const CaTrucMau *cm = dvLichMau.timCaMau(thu, caNames[c]);
        std::string hienTaiNV = (cm != nullptr && cm->coNhanVien()) ? cm->layMaNV() : "Chưa phân công";
        char bDongCa[128];
        snprintf(bDongCa, sizeof(bDongCa), "  [%d] %-8s : %s | Hiện tại: %s",
                 c + 1, caNames[c], gioCa[c], hienTaiNV.c_str());
        TienIchConsole::veDong(bDongCa);
      }
      TienIchConsole::veDong("  [0] Hủy và quay lại");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhập ca trực [1-3] hoặc [0] để hủy: ");
      int cCa = TienIchConsole::docPhimDon();
      if (cCa == '0' || cCa == 27) {
        continue;
      }
      if (cCa < '1' || cCa > '3') {
        TienIchConsole::hienThiLoi("Lựa chọn ca trực không hợp lệ!");
        Sleep(800);
        continue;
      }
      std::string tenCa = caNames[cCa - '1'];

      // Bước 3: Chọn Nhân viên
      std::vector<NguoiDung *> dsNV = layDanhSachNhanVien();
      if (dsNV.empty()) {
        TienIchConsole::hienThiLoi("Chưa có tài khoản nhân viên nào trong hệ thống!");
        TienIchConsole::choPhimBatKy();
        continue;
      }

      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("PHÂN CÔNG LỊCH TUẦN CHUẨN");
      std::string infoCa = "Mục tiêu: " + TienIchNgay::layTenThuDayDu(thu) + " - " + tenCa;
      TienIchConsole::veDong(infoCa);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("Bước 3: Chọn nhân viên phụ trách ca:");
      for (size_t i = 0; i < dsNV.size(); i++) {
        char bNV[128];
        snprintf(bNV, sizeof(bNV), "  [%d] %s - %s", (int)(i + 1),
                 dsNV[i]->layMa().c_str(), dsNV[i]->layHoTen().c_str());
        TienIchConsole::veDong(bNV);
      }
      TienIchConsole::veDong("  [X] Để trống ca này - Bỏ phân công");
      TienIchConsole::veDong("  [0] Hủy và quay lại");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      std::cout << "\n\t\t>> Nhập số thứ tự nhân viên [1-" << dsNV.size() << "], [X] để trống, hoặc [0] để hủy: ";
      std::string sChon;
      std::getline(std::cin, sChon);
      sChon = TienIchConsole::catKhoangTrang(sChon);
      if (sChon == "0" || sChon == "q" || sChon == "Q" || sChon == "huy") {
        TienIchConsole::hienThiLoi("Đã hủy thao tác phân công ca.");
        Sleep(800);
        continue;
      }

      std::string maNVChon = "TRONG";
      if (sChon == "x" || sChon == "X") {
        maNVChon = "TRONG";
      } else {
        int idx = -1;
        try { idx = std::stoi(sChon) - 1; } catch (...) { idx = -1; }
        if (idx < 0 || idx >= (int)dsNV.size()) {
          TienIchConsole::hienThiLoi("Lựa chọn nhân viên không hợp lệ!");
          Sleep(800);
          continue;
        }
        maNVChon = dsNV[idx]->layMa();
      }

      dvLichMau.capNhatPhanCong(thu, tenCa, maNVChon);

      // Tự động đồng bộ sang ca tương ứng của Tuần kế tiếp
      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      std::string t2TuanNay = TienIchNgay::layNgayThuHaiCuaTuan(ngayHienTai);
      std::string t2TuanToi = TienIchNgay::congNgay(t2TuanNay, 7);
      std::string ngayMucTieu = TienIchNgay::congNgay(t2TuanToi, thu - 2);

      const CaTrucMau *cmMoi = dvLichMau.timCaMau(thu, tenCa);
      std::string gBD = (cmMoi != nullptr) ? cmMoi->layGioBatDau() : "08:00";
      std::string gKT = (cmMoi != nullptr) ? cmMoi->layGioKetThuc() : "13:30";

      // Nếu tuần sau chưa được sinh ca, nhân bản toàn bộ tuần chuẩn sang tuần sau
      if (dvCaTruc.layDanhSachCaTheoNgay(t2TuanToi).empty()) {
        int soBoQua = 0;
        dvLichMau.apDungChoTuan(t2TuanToi, dvCaTruc, soBoQua);
      } else {
        dvCaTruc.dongBoCaTheoLichMau(ngayMucTieu, tenCa, gBD, gKT, maNVChon);
      }

      std::string thongBao = "Đã lưu lịch chuẩn và tự động đổi lịch " +
                             TienIchNgay::layTenThuDayDu(thu) + " tuần sau thành " +
                             (maNVChon == "TRONG" ? "Để trống" : maNVChon) + "!";
      TienIchConsole::hienThiThanhCong(thongBao);
      Sleep(1400);
    }
  }

  // Menu trung tâm: Quản lý Lịch ca trực tuần chuẩn
  void quanLyLichTuanMau() {
    bool dangOTrong = true;
    while (dangOTrong) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      bool khuyetMau = dvLichMau.coCaKhuyet();

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("QUẢN LÝ LỊCH CA TRỰC TUẦN CHUẨN");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("1. Xem ma trận lịch tuần chuẩn");

      if (khuyetMau) {
        std::vector<TienIchConsole::DoanMau> dm;
        dm.push_back(TienIchConsole::DoanMau("2. Chỉnh sửa lịch chuẩn", TienIchConsole::MAU_MAC_DINH));
        dm.push_back(TienIchConsole::DoanMau("                 [!] CÒN CA TRỐNG", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dm);
      } else {
        TienIchConsole::veDong("2. Chỉnh sửa lịch chuẩn - Tự động cập nhật tuần sau");
      }

      TienIchConsole::veDong("0. Quay lại");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1':
        hienThiMaTranLichMau();
        break;
      case '2':
        phanCongLichMauInteractive();
        break;
      case '0':
      case 27: // Esc
        dangOTrong = false;
        break;
      default:
        break;
      }
    }
  }

  // Màn hình 4: Bảng tính lương nhân viên toàn rạp
  void bangTinhLuongNhanVien() {
    TienIchConsole::xoaManHinh();
    printf("\n");
    TienIchConsole::veKhungTren();
    TienIchConsole::veDongGiua("BẢNG TÍNH LƯƠNG NHÂN VIÊN");
    char bGia[128];
    snprintf(bGia, sizeof(bGia), "Đơn giá: %s VNĐ một ca trực",
             TienIchConsole::dinhDangTien(dvCaTruc.layDonGiaCa()).c_str());
    TienIchConsole::veDong(bGia);
    TienIchConsole::veKhungGiua();
    TienIchConsole::veDongTrong();

    std::vector<NguoiDung *> dsNV = layDanhSachNhanVien();
    double tongQuyLuongCanChi = 0.0;

    if (dsNV.empty()) {
      TienIchConsole::veDong("  Hiện chưa có tài khoản nhân viên nào trong hệ thống.");
    } else {
      for (size_t i = 0; i < dsNV.size(); i++) {
        NguoiDung *nv = dsNV[i];
        int caChuaTra = 0, caDaTra = 0;
        double tienChuaTra = 0.0, tienDaTra = 0.0;
        dvCaTruc.tinhLuongNhanVien(nv->layMa(), caChuaTra, tienChuaTra, caDaTra, tienDaTra);
        tongQuyLuongCanChi += tienChuaTra;

        char b1[128], bCa[128], bLuong[128];
        snprintf(b1, sizeof(b1), "  %d. %s: %s", (int)(i + 1), nv->layMa().c_str(), nv->layHoTen().c_str());
        TienIchConsole::veDong(b1);

        snprintf(bCa, sizeof(bCa), "     Số ca chưa trả : %d ca", caChuaTra);
        TienIchConsole::veDong(bCa);

        snprintf(bLuong, sizeof(bLuong), "     Lương cần trả  : %s VNĐ",
                 TienIchConsole::dinhDangTien(tienChuaTra).c_str());
        TienIchConsole::veDong(bLuong);

        std::vector<TienIchConsole::DoanMau> dm;
        dm.push_back(TienIchConsole::DoanMau("     Trạng thái     : ", TienIchConsole::MAU_MAC_DINH));
        if (caChuaTra > 0) {
          dm.push_back(TienIchConsole::DoanMau("Chưa thanh toán", TienIchConsole::MAU_DO));
        } else {
          dm.push_back(TienIchConsole::DoanMau("Đã thanh toán", TienIchConsole::MAU_DANG_CHIEU));
        }
        TienIchConsole::veDongNhieuMau(dm);

        if (i + 1 < dsNV.size()) {
          TienIchConsole::veDuongKe();
        }
      }
    }

    TienIchConsole::veDongTrong();
    TienIchConsole::veKhungGiua();
    char bTong[128];
    snprintf(bTong, sizeof(bTong), "Tổng quỹ lương cần thanh toán: %s VNĐ",
             TienIchConsole::dinhDangTien(tongQuyLuongCanChi).c_str());
    TienIchConsole::veDong(bTong);
    TienIchConsole::veKhungDuoi();
    TienIchConsole::choPhimBatKy();
  }

  void menuQuanLyNhanVien() {
    bool dangOTrongMenu = true;
    while (dangOTrongMenu) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      bool khuyetTT = dvCaTruc.coCaKhuyetThucTe(ngayHienTai);
      bool khuyetMau = dvLichMau.coCaKhuyet();

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("QUẢN LÝ TÀI KHOẢN NHÂN VIÊN");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("1. Danh sách nhân viên và xem chi tiết");
      TienIchConsole::veDong("2. Thêm tài khoản nhân viên mới");

      if (khuyetTT) {
        std::vector<TienIchConsole::DoanMau> dm;
        dm.push_back(TienIchConsole::DoanMau("3. Xếp lịch ca trực và đổi ca làm thay", TienIchConsole::MAU_MAC_DINH));
        dm.push_back(TienIchConsole::DoanMau("  [!] THIẾU CA", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dm);
      } else {
        TienIchConsole::veDong("3. Xếp lịch ca trực và đổi ca làm thay");
      }

      if (khuyetMau) {
        std::vector<TienIchConsole::DoanMau> dm;
        dm.push_back(TienIchConsole::DoanMau("4. Quản lý Lịch ca trực tuần chuẩn", TienIchConsole::MAU_MAC_DINH));
        dm.push_back(TienIchConsole::DoanMau("     [!] CÒN CA TRỐNG", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dm);
      } else {
        TienIchConsole::veDong("4. Quản lý Lịch ca trực tuần chuẩn");
      }

      TienIchConsole::veDong("5. Bảng tính lương nhân viên");
      TienIchConsole::veDong("0. Quay lại Menu Admin");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1':
        danhSachNhanVienInteractive();
        break;
      case '2':
        themNhanVienMoi();
        break;
      case '3':
        xepLichCaTrucVaDoiCa();
        break;
      case '4':
        quanLyLichTuanMau();
        break;
      case '5':
        bangTinhLuongNhanVien();
        break;
      case '0':
      case 27: // Esc
        dangOTrongMenu = false;
        break;
      default:
        break;
      }
    }
  }

  void manHinhChiTietPhimTheoKy(const ThongKeKyVM &tk) {
    int trangHienTai = 0;
    const int SO_PHIM_MOI_TRANG = 4;
    int tongPhim = static_cast<int>(tk.dsPhim.size());
    int tongSoTrang = (tongPhim == 0) ? 1 : ((tongPhim + SO_PHIM_MOI_TRANG - 1) / SO_PHIM_MOI_TRANG);

    bool dangXem = true;
    while (dangXem) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("DOANH SỐ CHI TIẾT THEO PHIM");
      char bufKy[128];
      snprintf(bufKy, sizeof(bufKy), "%s - Trang %d / %d", tk.tieuDeKy.c_str(), trangHienTai + 1, tongSoTrang);
      TienIchConsole::veDongGiua(bufKy);
      TienIchConsole::veKhungGiua();

      if (tk.dsPhim.empty()) {
        TienIchConsole::veDongTrong();
        TienIchConsole::veDongGiua("Chưa có vé nào được bán trong kỳ này");
        TienIchConsole::veDongTrong();
      } else {
        int batDau = trangHienTai * SO_PHIM_MOI_TRANG;
        int ketThuc = std::min(batDau + SO_PHIM_MOI_TRANG, tongPhim);

        for (int i = batDau; i < ketThuc; i++) {
          const auto &p = tk.dsPhim[i];
          char bTen[128], bChiTiet[128], bDT[128];
          std::string tenRutGon = p.tenPhim;
          if (TienIchConsole::doDaiHienThi(tenRutGon.c_str()) > 46) {
            tenRutGon = tenRutGon.substr(0, 43) + "...";
          }
          snprintf(bTen, sizeof(bTen), "  %d. %s", i + 1, tenRutGon.c_str());
          snprintf(bChiTiet, sizeof(bChiTiet), "     Số vé: %d vé - Tỷ trọng: %.1f %%", p.soVe, p.tyTrong);
          snprintf(bDT, sizeof(bDT), "     Doanh thu: %s VNĐ",
                   TienIchConsole::dinhDangTien(p.doanhThu).c_str());

          TienIchConsole::veDong(bTen);
          TienIchConsole::veDong(bChiTiet);
          TienIchConsole::veDong(bDT);
          if (i + 1 < ketThuc) {
            TienIchConsole::veDuongKe();
          }
        }
      }

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Phím chọn: [N]: Tiếp | [P]: Trước | [0]: Quay lại: ");
      int cmd = TienIchConsole::docPhimDon();
      if ((cmd == 'n' || cmd == 'N') && trangHienTai + 1 < tongSoTrang) {
        trangHienTai++;
      } else if ((cmd == 'p' || cmd == 'P') && trangHienTai > 0) {
        trangHienTai--;
      } else if (cmd == '0' || cmd == 27) {
        dangXem = false;
      }
    }
  }

  void manHinhChiTietBaoCaoKy(const std::string &tuNgayISO,
                              const std::string &denNgayISO,
                              const std::string &tieuDeKy) {
    ThongKeKyVM tk = dvDatVe.tinhThongKeTheoKy(tuNgayISO, denNgayISO, tieuDeKy, dvCaTruc);

    bool dangXem = true;
    while (dangXem) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua(tk.tieuDeKy);
      char bufRange[128];
      snprintf(bufRange, sizeof(bufRange), "Từ: %s  Đến: %s",
               TienIchNgay::dinhDangHienThi(tk.tuNgay).c_str(),
               TienIchNgay::dinhDangHienThi(tk.denNgay).c_str());
      TienIchConsole::veDongGiua(bufRange);
      TienIchConsole::veKhungGiua();

      // Phần 1: Doanh thu bán vé
      TienIchConsole::veDong("1. DOANH THU BÁN VÉ");
      char bVe[128], bHD[128], bDT[128];
      snprintf(bVe, sizeof(bVe), "  • Số vé đã bán:          %d vé", tk.tongSoVe);
      snprintf(bHD, sizeof(bHD), "  • Số hóa đơn đã lập:     %d hóa đơn", tk.tongSoHoaDon);
      snprintf(bDT, sizeof(bDT), "  • Doanh thu bán vé:      %s VNĐ",
               TienIchConsole::dinhDangTien(tk.tongDoanhThuVe).c_str());
      TienIchConsole::veDong(bVe);
      TienIchConsole::veDong(bHD);
      TienIchConsole::veDong(bDT);

      TienIchConsole::veKhungGiua();

      // Phần 2: Chi phí nhân sự vận hành
      TienIchConsole::veDong("2. CHI PHÍ NHÂN SỰ VẬN HÀNH");
      char bCa[128], bLTra[128], bLNo[128], bLTong[128];
      snprintf(bCa, sizeof(bCa), "  • Số ca đã thực hiện:    %d ca", tk.tongSoCaTrongKy);
      snprintf(bLTra, sizeof(bLTra), "  • Lương đã chi trả:      %s VNĐ",
               TienIchConsole::dinhDangTien(tk.tienLuongDaTra).c_str());
      snprintf(bLNo, sizeof(bLNo), "  • Lương công nợ chờ trả: %s VNĐ",
               TienIchConsole::dinhDangTien(tk.tienLuongChuaTra).c_str());
      snprintf(bLTong, sizeof(bLTong), "  • Tổng chi phí lương:    %s VNĐ",
               TienIchConsole::dinhDangTien(tk.tongChiPhiLuong).c_str());
      TienIchConsole::veDong(bCa);
      TienIchConsole::veDong(bLTra);
      TienIchConsole::veDong(bLNo);
      TienIchConsole::veDong(bLTong);

      TienIchConsole::veKhungGiua();

      // Phần 3: Hiệu quả tài chính & lợi nhuận
      TienIchConsole::veDong("3. HIỆU QUẢ TÀI CHÍNH & LỢI NHUẬN");
      char bTS[128], bDTThuc[128];
      snprintf(bTS, sizeof(bTS), "  • Tỷ suất lợi nhuận:     %.1f %%", tk.tySuatLoiNhuan);
      snprintf(bDTThuc, sizeof(bDTThuc), "  • Dòng tiền thuần:       %s VNĐ",
               TienIchConsole::dinhDangTien(tk.dongTienThucTe).c_str());

      // Tô màu lợi nhuận ròng: Xanh lá nếu >= 0, Đỏ nếu < 0
      std::vector<TienIchConsole::DoanMau> dmLN;
      dmLN.push_back(TienIchConsole::DoanMau("  • Lợi nhuận ròng:        ", TienIchConsole::MAU_MAC_DINH));
      dmLN.push_back(TienIchConsole::DoanMau(TienIchConsole::dinhDangTien(tk.loiNhuanRong) + " VNĐ",
                     (tk.loiNhuanRong >= 0) ? TienIchConsole::MAU_DANG_CHIEU : TienIchConsole::MAU_DO));
      TienIchConsole::veDongNhieuMau(dmLN);

      TienIchConsole::veDong(bTS);
      TienIchConsole::veDong(bDTThuc);

      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Phím lệnh: [P]: Xem chi tiết theo phim | [0]: Quay lại: ");
      int cmd = TienIchConsole::docPhimDon();
      if (cmd == 'p' || cmd == 'P') {
        manHinhChiTietPhimTheoKy(tk);
      } else if (cmd == '0' || cmd == 27) {
        dangXem = false;
      }
    }
  }

  void menuBaoCaoDoanhThuVaKinhDoanh() {
    bool dangOTrongMenu = true;
    while (dangOTrongMenu) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("BÁO CÁO DOANH THU & HIỆU QUẢ KINH DOANH");
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();

      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      int namHienTai = 2026;
      int thangHienTai = 11;
      if (ngayHienTai.length() >= 7) {
        try {
          namHienTai = std::stoi(ngayHienTai.substr(0, 4));
          thangHienTai = std::stoi(ngayHienTai.substr(5, 2));
        } catch (...) {}
      }

      char opt1[128], opt2[128];
      snprintf(opt1, sizeof(opt1), "1. Tháng hiện tại: Tháng %02d/%04d", thangHienTai, namHienTai);
      snprintf(opt2, sizeof(opt2), "2. Hôm nay: %s", TienIchNgay::dinhDangHienThi(ngayHienTai).c_str());

      TienIchConsole::veDong(opt1);
      TienIchConsole::veDong(opt2);
      TienIchConsole::veDong("3. Tra cứu theo tháng năm bất kỳ");
      TienIchConsole::veDong("4. Toàn bộ tích lũy từ ngày đầu đến nay");
      TienIchConsole::veDong("0. Quay lại Menu Admin");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1': {
        int soNgay = 31;
        if (thangHienTai == 2) {
          bool nhuan = (namHienTai % 4 == 0 && namHienTai % 100 != 0) || (namHienTai % 400 == 0);
          soNgay = nhuan ? 29 : 28;
        } else if (thangHienTai == 4 || thangHienTai == 6 || thangHienTai == 9 || thangHienTai == 11) {
          soNgay = 30;
        }
        char tuISO[32], denISO[32], tieuDe[64];
        snprintf(tuISO, sizeof(tuISO), "%04d-%02d-01", namHienTai, thangHienTai);
        snprintf(denISO, sizeof(denISO), "%04d-%02d-%02d", namHienTai, thangHienTai, soNgay);
        snprintf(tieuDe, sizeof(tieuDe), "BÁO CÁO THÁNG %02d/%04d", thangHienTai, namHienTai);
        manHinhChiTietBaoCaoKy(tuISO, denISO, tieuDe);
        break;
      }
      case '2': {
        char tieuDe[64];
        snprintf(tieuDe, sizeof(tieuDe), "BÁO CÁO HÔM NAY - %s", TienIchNgay::dinhDangHienThi(ngayHienTai).c_str());
        manHinhChiTietBaoCaoKy(ngayHienTai, ngayHienTai, tieuDe);
        break;
      }
      case '3': {
        TienIchConsole::xoaManHinh();
        printf("\n");
        TienIchConsole::veKhungTren();
        TienIchConsole::veDongGiua("TRA CỨU BÁO CÁO THEO THÁNG NĂM");
        TienIchConsole::veKhungGiua();
        TienIchConsole::veDongTrong();
        TienIchConsole::veDong("  Nhập tháng và năm theo định dạng MM/YYYY");
        TienIchConsole::veDong("  Ví dụ: 10/2026 hoặc 11/2026");
        TienIchConsole::veDong("  Nhấn Enter để trống hoặc [0] để hủy");
        TienIchConsole::veDongTrong();
        TienIchConsole::veKhungDuoi();

        printf("\n\t\t>> Nhập MM/YYYY: ");
        std::string input;
        std::getline(std::cin, input);
        while (!input.empty() && (input.front() == ' ' || input.front() == '\t')) input.erase(0, 1);
        while (!input.empty() && (input.back() == ' ' || input.back() == '\t' || input.back() == '\r')) input.pop_back();

        if (input.empty() || input == "0") {
          break;
        }

        if (input.length() != 7 || input[2] != '/') {
          TienIchConsole::hienThiLoi("Định dạng không hợp lệ! Vui lòng dùng MM/YYYY");
          Sleep(1200);
          break;
        }

        int m = 0, y = 0;
        try {
          m = std::stoi(input.substr(0, 2));
          y = std::stoi(input.substr(3, 4));
        } catch (...) {
          TienIchConsole::hienThiLoi("Dữ liệu tháng năm phải là số nguyên!");
          Sleep(1200);
          break;
        }

        if (m < 1 || m > 12 || y < 2000 || y > 2100) {
          TienIchConsole::hienThiLoi("Tháng [1-12] hoặc năm không nằm trong phạm vi hợp lệ!");
          Sleep(1200);
          break;
        }

        int soNgay = 31;
        if (m == 2) {
          bool nhuan = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
          soNgay = nhuan ? 29 : 28;
        } else if (m == 4 || m == 6 || m == 9 || m == 11) {
          soNgay = 30;
        }

        char tuISO[32], denISO[32], tieuDe[64];
        snprintf(tuISO, sizeof(tuISO), "%04d-%02d-01", y, m);
        snprintf(denISO, sizeof(denISO), "%04d-%02d-%02d", y, m, soNgay);
        snprintf(tieuDe, sizeof(tieuDe), "BÁO CÁO THÁNG %02d/%04d", m, y);
        manHinhChiTietBaoCaoKy(tuISO, denISO, tieuDe);
        break;
      }
      case '4': {
        std::string ngayDau = dvDatVe.layNgayDauTienCoDuLieuISO(dvCaTruc);
        manHinhChiTietBaoCaoKy(ngayDau, ngayHienTai, "BÁO CÁO TOÀN BỘ THỜI GIAN");
        break;
      }
      case '0':
      case 27: // Phím Esc
        dangOTrongMenu = false;
        break;
      default:
        break;
      }
    }
  }

  void menuAdmin() {
    bool dangOTrongMenu = true;
    while (dangOTrongMenu) {
      TienIchConsole::xoaManHinh();
      printf("\n");

      std::string ngayHienTai = TienIchNgay::layNgayHienTaiISO();
      bool khuyetTT = dvCaTruc.coCaKhuyetThucTe(ngayHienTai);
      bool khuyetMau = dvLichMau.coCaKhuyet();
      bool coKhuyet = khuyetTT || khuyetMau;

      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("MENU QUẢN TRỊ VIÊN");
      TienIchConsole::veDong(std::string("Xin chào: ") +
                             nguoiDungHienTai->layHoTen());

      if (khuyetTT) {
        TienIchConsole::veKhungGiua();
        std::vector<TienIchConsole::DoanMau> dmb;
        dmb.push_back(TienIchConsole::DoanMau("  • [!] CẢNH BÁO: Rạp đang có ca trực bị khuyết!", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dmb);
      }

      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      if (!dvPhim.layCanhBaoQuota().empty()) {
        TienIchConsole::veDong(dvPhim.layCanhBaoQuota());
        TienIchConsole::veDong("Điều chỉnh danh mục trước khi thêm/mở thêm phim.");
        TienIchConsole::veKhungGiua();
      }
      TienIchConsole::veDong("1. Quản lý danh sách phim");
      TienIchConsole::veDong("2. Thêm phim mới");
      TienIchConsole::veDong("3. Sắp xếp phim theo thời lượng");
      TienIchConsole::veDong("4. Xem lịch chiếu 5 phòng");
      TienIchConsole::veDong("5. Thống kê kinh doanh & doanh thu");

      if (coKhuyet) {
        std::vector<TienIchConsole::DoanMau> dm;
        dm.push_back(TienIchConsole::DoanMau("6. Quản lý tài khoản nhân viên", TienIchConsole::MAU_MAC_DINH));
        dm.push_back(TienIchConsole::DoanMau("        [!] CÓ CA KHUYẾT", TienIchConsole::MAU_DO));
        TienIchConsole::veDongNhieuMau(dm);
      } else {
        TienIchConsole::veDong("6. Quản lý tài khoản nhân viên");
      }

      TienIchConsole::veDong("0. Đăng xuất");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1': {
        // Tích hợp Early Pull-out Protection khi Admin đổi trạng thái sang Ngừng chiếu
        auto onDoiTrangThai = [&](const std::string &maPhim, int trangThaiMoi) {
          if (trangThaiMoi == 0) {
            std::vector<std::string> thongBaoGiuLai;
            int soHuy = dvDatVe.xuLyKhiPhimNgungChieu(maPhim, thongBaoGiuLai);
            std::cout << "\n\t\t";
            TienIchConsole::datMau(TienIchConsole::MAU_SAP_CHIEU);
            std::cout << "[HỆ THỐNG BẢO VỆ SUẤT CHIẾU KHI PHIM NGỪNG CHIẾU]:\n";
            TienIchConsole::resetMau();
            std::cout << "\t\t• Đã tự động hủy " << soHuy << " suất chiếu tương lai chưa có vé để giải phóng phòng.\n";
            if (!thongBaoGiuLai.empty()) {
              std::cout << "\t\t• Giữ lại " << thongBaoGiuLai.size() << " suất chiếu đã có khách mua vé:\n";
              for (const auto &tb : thongBaoGiuLai) {
                std::cout << "\t\t   - " << tb << "\n";
              }
            }
            TienIchConsole::choPhimBatKy();
          } else if (trangThaiMoi == 2) {
            dvDatVe.sinhLichChieuTuDong(dvPhim.layDanhSach(), DichVuDatVe::soNgayPhuLich());
            if (!dvDatVe.luuLichChieu()) {
              TienIchConsole::hienThiLoi("Đã lưu đợt phim nhưng không lưu được lịch. Cần kiểm tra trước khi bán vé.");
            } else {
              const auto p=dvPhim.timPhimTheoMa(maPhim);
              const auto homNay=TienIchNgay::layNgayHienTaiISO();
              const auto denNgay=TienIchNgay::congNgay(homNay,DichVuDatVe::soNgayPhuLich()-1);
              if (p && p->layNgayKhoiChieu()>denNgay) {
                TienIchConsole::hienThiThanhCong("Đã lưu Sắp chiếu. Ngày khởi chiếu nằm ngoài phạm vi lịch hiện tại.");
              } else if (p && dvDatVe.demSuatTheoPhim(maPhim,p->layNgayKhoiChieu(),denNgay)==0) {
                TienIchConsole::hienThiLoi("Đã lưu Sắp chiếu nhưng chưa có suất từ ngày khởi chiếu trong lịch hiện tại. Không còn khung giờ trống phù hợp; hãy chọn ngày khác hoặc điều chỉnh lịch.");
              } else {
                TienIchConsole::hienThiThanhCong("Đã lưu Sắp chiếu. Các suất hiện có được giữ nguyên.");
              }
            }
            TienIchConsole::choPhimBatKy();
          }
        };
        dvPhim.coLichTuongLai = [&](const std::string &ma) { return dvDatVe.coSuatTuongLai(ma); };
        dvPhim.coLichSauNgay = [&](const std::string &ma,const std::string &ngay) { return dvDatVe.coSuatSauNgay(ma,ngay); };
        dvPhim.onDoiTenPhim = [&](const std::string &ma, const std::string &tenMoi) { dvDatVe.capNhatTenPhimSuatChieu(ma, tenMoi); };
        dvPhim.hienThiDanhSach(5, "", true, onDoiTrangThai);
        break;
      }
      case '2':
        formThemPhimMoi();
        break;
      case '3':
        dvPhim.sapXepTheoThoiLuong(5);
        break;
      case '4':
        xemLichChieuInteractive(true); // Chế độ Admin: Giám sát lịch & sơ đồ công suất ghế, không bán vé
        break;
      case '5':
        menuBaoCaoDoanhThuVaKinhDoanh();
        break;
      case '6':
        menuQuanLyNhanVien();
        break;
      case '0':
      case 27: // Phím Esc
        TienIchConsole::hienThiThanhCong("Đăng xuất thành công!");
        Sleep(800);
        dangOTrongMenu = false;
        break;
      default:
        break;
      }
    }
  }

  void menuNhanVien() {
    bool dangOTrongMenu = true;
    while (dangOTrongMenu) {
      TienIchConsole::xoaManHinh();
      printf("\n");
      TienIchConsole::veKhungTren();
      TienIchConsole::veDongGiua("MENU NHÂN VIÊN BÁN VÉ");
      std::string dongXinChao = std::string("Thu ngân: ") + nguoiDungHienTai->layHoTen() +
                                "  |  Ngày trực: " + TienIchNgay::dinhDangHienThi(TienIchNgay::layNgayHienTaiISO());
      TienIchConsole::veDong(dongXinChao);
      TienIchConsole::veKhungGiua();
      TienIchConsole::veDongTrong();
      TienIchConsole::veDong("1. Bán vé tại quầy");
      TienIchConsole::veDong("2. Báo cáo doanh số ca trực của tôi");
      TienIchConsole::veDong("0. Đăng xuất và bàn giao ca");
      TienIchConsole::veDongTrong();
      TienIchConsole::veKhungDuoi();

      printf("\n\t\t>> Nhấn phím để chọn: ");
      int ch = TienIchConsole::docPhimDon();

      switch (ch) {
      case '1':
        xemLichChieuInteractive(false);
        break;

      case '2': {
        TienIchConsole::xoaManHinh();
        std::string tenNV = nguoiDungHienTai->layHoTen();
        int soVe = 0, soHD = 0;
        double tongTien = 0.0;
        dvDatVe.tinhDoanhThuCaTruc(tenNV, soVe, soHD, tongTien);

        // Đồng bộ doanh thu vào hệ thống ca trực
        dvCaTruc.capNhatDoanhThuCaHienTai(nguoiDungHienTai->layMa(), tongTien);

        printf("\n");
        TienIchConsole::veKhungTren();
        TienIchConsole::veDongGiua("BÁO CÁO DOANH SỐ CA TRỰC");
        TienIchConsole::veDong(std::string("Thu ngân: ") + tenNV + "  |  Ngày: " + TienIchNgay::dinhDangHienThi(TienIchNgay::layNgayHienTaiISO()));
        TienIchConsole::veKhungGiua();
        TienIchConsole::veDongTrong();

        char bVe[128], bHD[128], bDT[128];
        snprintf(bVe, sizeof(bVe), "  • Tổng số vé đã bán:       %d vé", soVe);
        snprintf(bHD, sizeof(bHD), "  • Tổng số hóa đơn đã lập:  %d hóa đơn", soHD);
        snprintf(bDT, sizeof(bDT), "  • TỔNG DOANH THU CA TRỰC:  %s VNĐ",
                 TienIchConsole::dinhDangTien(tongTien).c_str());

        TienIchConsole::veDong(bVe);
        TienIchConsole::veDong(bHD);
        TienIchConsole::veDuongKe();
        TienIchConsole::veDong(bDT);
        TienIchConsole::veDongTrong();
        TienIchConsole::veKhungDuoi();
        TienIchConsole::choPhimBatKy();
        break;
      }

      case '0':
      case 27: // Phím Esc
        TienIchConsole::hienThiThanhCong("Đăng xuất thành công! Chúc bạn ca làm việc thuận lợi.");
        Sleep(800);
        dangOTrongMenu = false;
        break;

      default:
        break;
      }
    }
  }

  void chay() {
    bool dangChay = true;
    while (dangChay) {
      if (dangNhap()) {
        if (nguoiDungHienTai->layVaiTro() == "Quản trị viên") {
          menuAdmin();
        } else if (nguoiDungHienTai->layVaiTro() == "Nhân viên") {
          menuNhanVien();
        }
        // Reset sau khi đăng xuất
        nguoiDungHienTai = nullptr;
      } else {
        if (std::cin.eof()) {
          dangChay = false;
          break;
        }
        while (true) {
          TienIchConsole::xoaManHinh();
          printf("\n");
          TienIchConsole::veKhungTren();
          TienIchConsole::veDongGiua("ĐĂNG NHẬP THẤT BẠI");
          TienIchConsole::veKhungGiua();
          TienIchConsole::veDongTrong();
          TienIchConsole::veDong("1. Thử đăng nhập lại");
          TienIchConsole::veDong("0. Thoát chương trình");
          TienIchConsole::veDongTrong();
          TienIchConsole::veKhungDuoi();

          printf("\n\t\t>> Nhấn phím để chọn: [1]: Thử lại | [0/Esc]: Thoát: ");
          int ch = TienIchConsole::docPhimDon();
          if (ch == '1') {
            dangChay = true;
            break;
          } else if (ch == '0' || ch == 27) {
            dangChay = false;
            break;
          }
        }
      }
    }

    TienIchConsole::xoaManHinh();
    printf("\n");
    TienIchConsole::veKhungTren();
    TienIchConsole::veDongTrong();
    TienIchConsole::veDongGiua("CẢM ƠN BẠN ĐÃ SỬ DỤNG HỆ THỐNG RẠP PHIM!");
    TienIchConsole::veDongTrong();
    TienIchConsole::veKhungDuoi();
    printf("\n");
  }
};

#endif // GIAODIENCONSOLE_H
