#include "../include/giaodien/GiaoDienConsole.h"
#include <Windows.h>
#include <iostream>

using namespace std;

int main() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  GiaoDienConsole app;
  app.chay();

  return 0;
}
