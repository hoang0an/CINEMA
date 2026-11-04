#ifndef DANHSACHLIENKET_H
#define DANHSACHLIENKET_H

#include <cstddef>
#include <iostream>

template <typename T>

struct Nut {
  T duLieu;
  Nut<T> *tiepTheo;
  Nut(T giaTri) : duLieu(giaTri), tiepTheo(nullptr) {}
};

template <typename T> class DanhSachLienKet {
private:
  Nut<T> *dau;
  Nut<T> *cuoi;
  int soLuong;

public:
  DanhSachLienKet() : dau(nullptr), cuoi(nullptr), soLuong(0) {}

  // Copy Constructor (Rule of Three)
  DanhSachLienKet(const DanhSachLienKet<T> &other) : dau(nullptr), cuoi(nullptr), soLuong(0) {
    Nut<T> *hienTai = other.dau;
    while (hienTai != nullptr) {
      themCuoi(hienTai->duLieu);
      hienTai = hienTai->tiepTheo;
    }
  }

  // Copy Assignment Operator
  DanhSachLienKet<T> &operator=(const DanhSachLienKet<T> &other) {
    if (this != &other) {
      while (dau != nullptr) {
        xoaDau();
      }
      Nut<T> *hienTai = other.dau;
      while (hienTai != nullptr) {
        themCuoi(hienTai->duLieu);
        hienTai = hienTai->tiepTheo;
      }
    }
    return *this;
  }

  ~DanhSachLienKet() {
    while (dau != nullptr) {
      xoaDau();
    }
  }

  void themCuoi(T giaTri) {
    Nut<T> *nutMoi = new Nut<T>(giaTri);

    if (dau == nullptr) {
      dau = cuoi = nutMoi;
    } else {
      cuoi->tiepTheo = nutMoi;
      cuoi = nutMoi;
    }
    soLuong++;
  }

  void duyet() {
    Nut<T> *hienTai = dau;
    while (hienTai != nullptr) {
      std::cout << hienTai->duLieu << "->";
      hienTai = hienTai->tiepTheo;
    }
    std::cout << "nullptr" << std::endl;
  }

  bool xoaDau() {
    if (dau == nullptr) {
      return false;
    }
    Nut<T> *tam = dau;
    dau = dau->tiepTheo;
    delete tam;
    soLuong--;
    if (dau == nullptr) {
      cuoi = nullptr;
    }
    return true;
  }

  bool xoaTheoGiaTri(T giaTri) {
    if (dau == nullptr) {
      return false;
    }
    if (dau->duLieu == giaTri) {
      return xoaDau();
    }

    Nut<T> *truoc = dau;
    Nut<T> *hienTai = dau->tiepTheo;

    while (hienTai != nullptr && hienTai->duLieu != giaTri) {
      truoc = hienTai;
      hienTai = hienTai->tiepTheo;
    }
    if (hienTai == nullptr) {
      return false;
    }
    truoc->tiepTheo = hienTai->tiepTheo;
    if (hienTai == cuoi) {
      cuoi = truoc;
    }
    delete hienTai;
    soLuong--;
    return true;
  }

  void themDau(T giaTri) {
    Nut<T> *nutMoi = new Nut<T>(giaTri);
    if (dau == nullptr) {
      dau = cuoi = nutMoi;
    } else {
      nutMoi->tiepTheo = dau;
      dau = nutMoi;
    }
    soLuong++;
  }

  int laySoLuong() const { return soLuong; }

  Nut<T> *timKiem(T giaTri) {
    Nut<T> *hienTai = dau;
    while (hienTai != nullptr) {
      if (hienTai->duLieu == giaTri) {
        return hienTai;
      }
      hienTai = hienTai->tiepTheo;
    }
    return nullptr;
  }

  Nut<T> *layDau() const { return dau; }
};

#endif
