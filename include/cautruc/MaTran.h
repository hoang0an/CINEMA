#ifndef MATRAN_H
#define MATRAN_H

#include <iostream>
template <typename T> class MaTran {
private:
  int soHang;
  int soCot;
  T **duLieu;

public:
  MaTran(const MaTran&) = delete;
  MaTran& operator=(const MaTran&) = delete;
  MaTran(int soHang, int soCot) {
    this->soHang = soHang;
    this->soCot = soCot;
    this->duLieu = new T *[soHang];
    for (int i = 0; i < soHang; i++) {
      this->duLieu[i] = new T[soCot];
    }
  }

  ~MaTran() {
    for (int i = 0; i < soHang; i++) {
      delete[] duLieu[i];
    }
    delete[] duLieu;
  }

  int laySoHang() const { return soHang; }
  int laySoCot() const { return soCot; }

  bool datGiaTri(int hang, int cot, T giaTri) {
    if (0 <= hang && hang < soHang && 0 <= cot && cot < soCot) {
      duLieu[hang][cot] = giaTri;
      return true;
    }
    return false;
  }

  T layGiaTri(int hang, int cot) const { return duLieu[hang][cot]; }

  void hienThi() const {
    for (int i = 0; i < soHang; i++) {
      for (int j = 0; j < soCot; j++) {
        std::cout << duLieu[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }
};
#endif