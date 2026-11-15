#ifndef GHIFILEANTOAN_H
#define GHIFILEANTOAN_H

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <string>

// Chỉ thay file đích sau khi đã ghi và đóng thành công file tạm.
// Ứng dụng chỉ hỗ trợ một tiến trình ghi dữ liệu tại một thời điểm.
template <typename Ghi> bool ghiFileAnToan(const std::string &path, Ghi ghi) {
  const std::filesystem::path dich(path), tam(path + ".writing");
  std::ofstream file(tam, std::ios::binary | std::ios::trunc);
  if (!file) return false;
  ghi(file);
  file.close();
  if (!file.fail() && MoveFileExW(tam.c_str(), dich.c_str(),
                                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return true;
  std::error_code ec;
  std::filesystem::remove(tam, ec);
  return false;
}
#endif
