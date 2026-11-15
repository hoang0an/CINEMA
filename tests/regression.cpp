#include "dichvu/DichVuDatVe.h"
#include "dichvu/DichVuLichMau.h"
#include "dichvu/DichVuPhim.h"
#include <functional>
#include <stdexcept>

#define CHECK(x) do { if (!(x)) throw std::runtime_error(std::string(#x) + " @" + std::to_string(__LINE__)); } while (false)
namespace fs = std::filesystem;
void put(const std::string &p, const std::string &s) { std::ofstream f(p); f << s; CHECK(bool(f)); }
std::string get(const std::string &p) { std::ifstream f(p); return {std::istreambuf_iterator<char>(f), {}}; }
std::string row(const std::string &id, const std::string &ve, const std::string &date,
                const std::string &staff, const std::string &seat = "A1", const std::string &price = "100000") {
  return id + "|" + date + "|Same Name|" + ve + "|SC1|Film|Room|2026-01-01|10:00|" + seat + "|" + price + staff + "\n";
}
int main() {
  const auto root = fs::temp_directory_path() / ("pbl2-tests-" + std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64()));
  fs::create_directories(root); const auto old = fs::current_path(); fs::current_path(root);
  int passed = 0, failed = 0;
  auto test = [&](const std::string &name, const std::function<void()> &fn) {
    const auto dir = root / std::to_string(passed + failed); fs::create_directory(dir); fs::current_path(dir);
    try { fn(); ++passed; std::cout << "PASS " << name << "\n"; }
    catch (const std::exception &e) { ++failed; std::cerr << "FAIL " << name << ": " << e.what() << "\n"; }
    fs::current_path(root);
  };
  const auto today = TienIchNgay::layNgayHienTaiISO();
  test("inclusive one-day film and last date", [&] {
    Phim p("P", "Film", "Drama", "Director", 120, 1, "2030-01-01", 1);
    CHECK(p.layNgayKetThuc() == "2030-01-01"); CHECK(p.tinhTrangThaiTheoNgay("2030-01-02") == 0);
    CHECK(p.setNgayKetThuc("2030-01-01")); CHECK(p.laySoNgayChieu() == 1);
    CHECK(!p.setNgayKetThuc("2029-12-31"));
  });
  test("legacy film migration preserves end date and is idempotent", [&] {
    put("film", "260001|Test Film|Drama|John Doe|120|0|2026-01-01|30\n");
    DanhSachLienKet<Phim> a; CHECK(QuanLyFile::docDanhSachPhim("film", a)); CHECK(a.laySoLuong() == 1);
    CHECK(a.layDau()->duLieu.layNgayKetThuc() == "2026-01-31");
    CHECK(QuanLyFile::ghiDanhSachPhim("film", a)); DanhSachLienKet<Phim> b;
    CHECK(QuanLyFile::docDanhSachPhim("film", b)); CHECK(b.layDau()->duLieu.laySoNgayChieu() == 31);
  });
  test("film status normalized before quota", [&] {
    DichVuPhim dv;
    CHECK(dv.themPhim(Phim("P", "Test Film", "Drama", "John Doe", 120, 1, TienIchNgay::congNgay(today, 5), 1)));
    CHECK(dv.timPhimTheoMa("P")->layTrangThai() == 2);
    CHECK(!dv.themPhim(Phim("Q", "Test Film", "Drama", "John Doe", 700, 1, today, 1)));
  });
  test("read-only daily report separates employee IDs and dates", [&] {
    put("invoices", row("HD001", "VE001", today, "|NV1|C1|09:00") +
        row("HD002", "VE002", today, "|NV2|C1|09:00", "A2") +
        row("HD003", "VE003", "2020-01-01", "|NV1|C0|09:00", "A3"));
    DichVuDatVe dv; CHECK(dv.docDanhSachHoaDon("invoices"));
    auto original = get("invoices"); int v, h; double t;
    for (int i = 0; i < 2; ++i) { dv.tinhDoanhThuNhanVienTrongNgay("NV1", today, v, h, t); CHECK(v == 1 && h == 1 && t == 100000); }
    dv.tinhDoanhThuNhanVienTrongNgay("NV1", today, v, h, t, "wrong"); CHECK(t == 0);
    CHECK(get("invoices") == original);
  });
  test("legacy invoices retained but not guessed into daily employee totals", [&] {
    put("invoices", row("HD001", "VE001", today, "")); DichVuDatVe dv;
    CHECK(dv.docDanhSachHoaDon("invoices")); CHECK(dv.tinhTongDoanhThu() == 100000);
    int v,h;double t;dv.tinhDoanhThuNhanVienTrongNgay("NV1",today,v,h,t);CHECK(t == 0);
    CHECK(dv.luuToanBoHoaDon()); DichVuDatVe loaded; CHECK(loaded.docDanhSachHoaDon("invoices"));
    CHECK(loaded.tinhTongDoanhThu() == 100000);
  });
  for (const auto &price : {"-1", "nan", "inf", "100abc"}) test(std::string("reject price ") + price, [&] {
    put("invoices", row("HD001", "VE001", today, "", "A1", price));
    auto original = get("invoices"); DichVuDatVe dv;
    CHECK(!dv.docDanhSachHoaDon("invoices")); CHECK(!dv.luuToanBoHoaDon()); CHECK(get("invoices") == original);
  });
  test("reject duplicate ticket and inconsistent invoice header", [&] {
    put("invoices", row("HD001","VE001",today,"") + row("HD002","VE001",today,"","A2"));
    DichVuDatVe dv;CHECK(!dv.docDanhSachHoaDon("invoices"));
    put("invoices", row("HD001","VE001",today,"|NV1||09:00") + row("HD001","VE002",today,"|NV2||09:00","A2"));
    DichVuDatVe other;CHECK(!other.docDanhSachHoaDon("invoices"));
  });
  test("shift write failure restores memory", [&] {
    DichVuCaTruc dv;dv.docDanhSach("missing/shift");
    CHECK(dv.xepCaMoiChiTiet(today,"Morning","08:00","13:30","NV1") == -1);
    CHECK(dv.layDanhSachCa().empty()); CHECK(!fs::exists("missing/shift"));
  });
  test("shift revenue reconciliation is idempotent and uses exact shift", [&] {
    put("shift", "C1|"+today+"|Morning|08:00|13:30|NV1|NV1|0|0\nC2|"+today+"|Night|18:30|23:30|NV1|NV1|0|0\n");
    DichVuCaTruc ca;CHECK(ca.docDanhSach("shift"));
    CHECK(ca.timCaDangLam("NV1",today,"20:00")->layMaCa() == "C2");
    CHECK(!ca.timCaDangLam("NV1",today,"13:30"));
    CHECK(ca.doiSoatDoanhThu("C2",100000)); CHECK(ca.doiSoatDoanhThu("C2",100000));
    CHECK(ca.layCaTheoMa("C2")->layDoanhThu() == 100000);CHECK(ca.layCaTheoMa("C1")->layDoanhThu() == 0);
  });
  test("salary failure restores state and preserves disk", [&] {
    put("shift", "C1|2020-01-01|Morning|08:00|13:30|NV1|NV1|0|0\n");
    DichVuCaTruc ca;CHECK(ca.docDanhSach("shift")); const auto original=get("shift");
    fs::create_directory("shift.writing"); CHECK(ca.thanhToanLuongNhanVien("NV1") == -1);
    CHECK(!ca.layCaTheoMa("C1")->daThanhToan());CHECK(get("shift") == original);
  });
  test("shift substitution and deletion failure restore state", [&] {
    put("shift", "C1|2020-01-01|Morning|08:00|13:30|NV1|NV1|0|0\n");
    DichVuCaTruc ca;CHECK(ca.docDanhSach("shift")); fs::create_directory("shift.writing");
    CHECK(ca.doiNguoiLamThayChiTiet("C1","NV2") == -1);CHECK(ca.layCaTheoMa("C1")->layMaNVThucTe() == "NV1");
    CHECK(ca.xoaCaTruc("C1") == -1);CHECK(ca.layCaTheoMa("C1"));
  });
  test("overlap synchronization reports failure", [&] {
    put("shift", "C1|"+today+"|Morning|08:00|13:30|NV1|NV1|0|0\n");
    DichVuCaTruc ca;CHECK(ca.docDanhSach("shift"));
    CHECK(!ca.dongBoCaTheoLichMau(today,"Overlap","09:00","12:00","NV2"));CHECK(ca.layDanhSachCa().size() == 1);
  });
  test("employee reference guards include assignments and templates", [&] {
    put("shift", "C1|2020-01-01|Morning|08:00|13:30|NV1|NV2|0|1\n");
    DichVuCaTruc ca;CHECK(ca.docDanhSach("shift"));CHECK(ca.coThamChieuNhanVien("NV1"));CHECK(ca.coThamChieuNhanVien("NV2"));
    CHECK(!ca.coThamChieuNhanVien("NV3"));DichVuLichMau lm;lm.khoiTaoMauMacDinh({"NV1"});CHECK(lm.coThamChieuNhanVien("NV1"));
  });
  test("template write failure restores assignment", [&] {
    DichVuLichMau lm("missing/template");lm.khoiTaoMauMacDinh({"NV1"});auto ca=lm.layDanhSach().front();
    CHECK(!lm.capNhatPhanCong(ca.layThu(),ca.layTenCa(),"NV2"));CHECK(lm.layDanhSach().front().layMaNV() == "NV1");
  });
  test("atomic writer leaves destination intact on stream error", [&] {
    put("file","original");CHECK(!ghiFileAnToan("file",[](std::ostream &o){o << "partial";o.setstate(std::ios::badbit);}));
    CHECK(get("file") == "original");
  });
  test("booking rejects duplicate seats and persists metadata", [&] {
    fs::create_directory("dulieu");put("dulieu/hoadon.txt","");put("dulieu/suatchieu.txt","");
    DichVuDatVe dv; CHECK(dv.docDanhSachHoaDon());
    auto sc=new SuatChieu("SC1","P","Film",TienIchNgay::congNgay(today,1),"10:00","P01","Room",8,6);dv.themSuatChieu(sc);
    CHECK(!dv.datNhieuVe("SC1",{{0,0},{0,0}},"Same Name",today,"NV1","C1","09:00",false));
    CHECK(!sc->layPhongChieu()->kiemTraGheDaDat(0,0));
    CHECK(dv.datNhieuVe("SC1",{{0,0}},"Same Name",today,"NV1","C1","09:00",false));
    DichVuDatVe loaded;CHECK(loaded.docDanhSachHoaDon());int v,h;double t;
    loaded.tinhDoanhThuNhanVienTrongNgay("NV1",today,v,h,t,"C1");CHECK(v == 1 && h == 1 && t > 0);
    CHECK(dv.kiemTraDoiSoat().empty());
  });
  test("failed booking restores seats and invoice counters", [&] {
    fs::create_directory("dulieu");put("dulieu/hoadon.txt","");put("dulieu/suatchieu.txt","");
    DichVuDatVe dv;CHECK(dv.docDanhSachHoaDon());
    auto sc=new SuatChieu("SC1","P","Film",TienIchNgay::congNgay(today,1),"10:00","P01","Room",8,6);dv.themSuatChieu(sc);
    fs::create_directory("dulieu/hoadon.txt.tmp.writing");
    CHECK(!dv.datNhieuVe("SC1",{{0,0}},"Name",today,"NV1","C1","09:00",false));
    CHECK(!sc->layPhongChieu()->kiemTraGheDaDat(0,0));CHECK(dv.layTongSoHoaDon() == 0);CHECK(dv.layTongSoVeDaBan() == 0);
    CHECK(get("dulieu/hoadon.txt").empty());
  });
  test("future invoice-seat mismatch locks booking", [&] {
    fs::create_directory("dulieu");put("dulieu/suatchieu.txt","");
    const auto tomorrow = TienIchNgay::congNgay(today, 1);
    put("dulieu/hoadon.txt", "HD001|"+today+"|Name|VE001|SC1|Film|Room|"+tomorrow+"|10:00|A1|100000|NV1||09:00\n");
    DichVuDatVe dv;CHECK(dv.docDanhSachHoaDon());
    dv.themSuatChieu(new SuatChieu("SC1","P","Film",tomorrow,"10:00","P01","Room",8,6));
    CHECK(dv.kiemTraDoiSoat().size() == 1);
    CHECK(!dv.datNhieuVe("SC1",{{0,1}},"Name",today,"NV1","","09:00",false));
  });
  test("blocked marker prevents startup writing and sales", [&] {
    put("shows", "");put("shows.blocked", "recovery");put("invoices", "");
    DichVuDatVe dv;DanhSachLienKet<Phim> films;dv.khoiTaoLichChieu("shows",films);CHECK(dv.docDanhSachHoaDon("invoices"));
    CHECK(!dv.hoanTatKhoiTao(films));CHECK(get("shows").empty());CHECK(fs::exists("shows.blocked"));
  });
  test("malformed shift/template cannot be silently overwritten", [&] {
    put("shift", "bad\n");DichVuCaTruc ca;CHECK(!ca.docDanhSach("shift"));
    CHECK(ca.xepCaMoiChiTiet(today,"Morning","08:00","13:30","NV1") == -1);CHECK(get("shift") == "bad\n");
    put("template", "9|Morning|08:00|13:30|NV1\n");DichVuLichMau lm;
    CHECK(!lm.docDanhSach("template"));CHECK(!lm.ghiDanhSach("template"));CHECK(get("template") == "9|Morning|08:00|13:30|NV1\n");
  });
  test("malformed seat bitmap is rejected", [&] {
    put("shows", "SC1|P|Film|2030-01-01|10:00|P01|Room|8|6|0\n");
    DichVuDatVe dv;CHECK(!QuanLyFile::docDanhSachSuatChieu("shows",dv.layDanhSachSuatChieu()));CHECK(dv.layDanhSachSuatChieu().laySoLuong() == 0);
  });
  test("schedule cancellation failure restores in-memory shows", [&] {
    put("shows", "");DanhSachLienKet<Phim> films;DichVuDatVe dv;dv.khoiTaoLichChieu("shows",films);
    dv.themSuatChieu(new SuatChieu("SC1","P","Film",TienIchNgay::congNgay(today,1),"10:00","P01","Room",8,6));
    fs::create_directory("shows.writing");std::vector<std::string> messages;
    CHECK(dv.xuLyKhiPhimNgungChieu("P",messages) == -1);CHECK(dv.layDanhSachSuatChieu().laySoLuong() == 1);
  });
  test("startup completes stopped-film schedule cleanup and preserves sold seats", [&] {
    put("shows", "");put("invoices", "");DanhSachLienKet<Phim> films;
    films.themCuoi(Phim("P","Film","Drama","John Doe",120,0,today,30));
    DichVuDatVe dv;dv.khoiTaoLichChieu("shows",films);CHECK(dv.docDanhSachHoaDon("invoices"));
    const auto tomorrow=TienIchNgay::congNgay(today,1);
    dv.themSuatChieu(new SuatChieu("SC1","P","Film",tomorrow,"10:00","P01","Room",8,6));
    auto sold=new SuatChieu("SC2","P","Film",tomorrow,"13:00","P01","Room",8,6);
    sold->layPhongChieu()->datGhe(0,0);dv.themSuatChieu(sold);
    CHECK(dv.hoanTatKhoiTao(films));CHECK(dv.layDanhSachSuatChieu().laySoLuong()==1);
    CHECK(dv.layDanhSachSuatChieu().layDau()->duLieu==sold);
    CHECK(get("shows").find("SC1|")==std::string::npos);
  });
  test("unknown dependency data prevents employee deletion", [&] {
    DichVuCaTruc ca;CHECK(!ca.docDanhSach("missing"));CHECK(ca.coThamChieuNhanVien("NV1"));
    put("template","broken\n");DichVuLichMau lm;CHECK(!lm.docDanhSach("template"));CHECK(lm.coThamChieuNhanVien("NV1"));
    DichVuDatVe dv;CHECK(dv.coHoaDonNhanVien("NV1","Name"));
    put("invoices", "");CHECK(dv.docDanhSachHoaDon("invoices"));CHECK(!dv.coHoaDonNhanVien("NV1","Name"));
    put("invoices","bad\n");CHECK(!dv.docDanhSachHoaDon("invoices"));CHECK(dv.coHoaDonNhanVien("NV1","Name"));
  });
  test("reject invoice numeric aliases and seat numeric aliases", [&] {
    put("invoices",row("HD001","VE001",today,"")+row("HD1","VE002",today,"","A2"));
    DichVuDatVe dv;CHECK(!dv.docDanhSachHoaDon("invoices"));
    put("invoices",row("HD001","VE001",today,"")+row("HD002","VE002",today,"","A01"));
    DichVuDatVe other;CHECK(!other.docDanhSachHoaDon("invoices"));
  });
  fs::current_path(old);
  std::error_code ec;fs::remove_all(root,ec);
  std::cout << passed << " passed; " << failed << " failed\n";
  return failed ? 1 : 0;
}
