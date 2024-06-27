#include <chrono>
#include <execution>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "xls.h"

namespace fs = std::filesystem;
using namespace std;
using namespace xls;

struct Profiler {
  using clock = chrono::steady_clock;
  const char* name;
  clock::time_point begin = clock::now();
  ~Profiler() {
    auto cost =
        chrono::duration_cast<chrono::milliseconds>(clock::now() - begin)
            .count();
    printf("%s cost %.2fs.\n", name, cost / 1000.0f);
  }
};

string ColName(int i) {
  char buf[255];
  if (i < 26)
    sprintf(buf, "%c", 'A' + i);
  else {
    sprintf(buf, "%d", i);
  }
  return buf;
}

void search_file(string_view f, string_view tex) {
  auto file_base = f.substr(f.rfind("\\") + 1);
  xls_error_t error = LIBXLS_OK;
  auto wb = xls_open_file(f.data(), "UTF-8", &error);
  if (wb == NULL) {
    printf("Waring: %s %s\n", xls_getError(error), f.data());
    return;
  }
  for (int sheetIdx = 0; sheetIdx < wb->sheets.count; sheetIdx++) {
    string_view sheet_name = wb->sheets.sheet[sheetIdx].name;
    auto sheet = xls_getWorkSheet(wb, sheetIdx);
    if (auto err = xls_parseWorkSheet(sheet); err != LIBXLS_OK) {
      printf("Waring: %s %s %s\n", xls_getError(err), f.data(),
             sheet_name.data());
      return;
    }

    int rowEnd = sheet->rows.lastrow, colEnd = sheet->rows.lastcol;
    for (int rowIdx = 0; rowIdx <= rowEnd; rowIdx++) {
      auto row = xls_row(sheet, rowIdx);
      auto cells = row->cells.cell;
      for (int colIdx = 0; colIdx <= colEnd; colIdx++) {
        auto c = cells[colIdx];
        if (c.str && strstr(c.str, tex.data())) {
          printf("file:%-16s sheet:%-8s row:%-4d col:%-4s  cell:%s\n",
                 file_base.data(), sheet_name.data(), rowIdx + 1,
                 ColName(colIdx).c_str(), c.str);
        }
      }
    }
  }
  xls_close_WB(wb);
}

void search(string_view path, string_view tex) {
  Profiler pro{"search"};
  vector<string> items{};
  for (const auto& i : fs::recursive_directory_iterator(path)) {
    const auto& s = i.path().string();
    if (s.ends_with(".xls")) {
      items.push_back(s);
    }
  }
  printf("searching in %s\n", path.data());
  printf("total files to scan: %zd\n", items.size());
  std::for_each(std::execution::par, items.begin(), items.end(),
                [&](const auto& f) { search_file(f, tex); });
}

void usage(const char* app) {
  printf("Usage: %s <xls path> <text>\n", app);
  printf("  or set XLS_SEARCH_DIR env variable for interactive mode\n");
}

int main(int argc, const char* argv[]) {
  span args{argv, size_t(argc)};

#if _DEBUG
  auto path = R"(C:\Samo\Trunk\data\GameDatas\datas)";
  auto tex = "TALENT_NODE_N_218";
  search(path, tex);
  return 0;
#else  
  if (args.size() == 1) {
    if (auto v = getenv("XLS_SEARCH_DIR"); !v) {
      usage(args[0]);
      return 0;
    } else {
      printf("text to search:");
      string tex;
      cin >> tex;
      search(v, tex);      
      system("pause");
    }
  } else if (args.size() == 3) {
    search(args[1], args[2]);
  } else {
    usage(args[0]);
    return 0;
  }
  return 0;
#endif  
}
