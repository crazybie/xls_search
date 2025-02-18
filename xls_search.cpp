using namespace xls;

using namespace std;
using namespace std::filesystem;
using namespace std::chrono;

struct Profiler {
  const char* name;
  steady_clock::time_point begin = steady_clock::now();
  ~Profiler() {
    auto n = duration_cast<milliseconds>(steady_clock::now() - begin).count();
    printf("\n%s cost %.2fs.\n", name, n / 1000.0f);
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

string search_file(string_view f, string_view tex) {
  ostringstream os;
  auto file_base = f.substr(f.rfind("\\") + 1);
  xls_error_t error = LIBXLS_OK;
  auto wb = xls_open_file(f.data(), "UTF-8", &error);
  if (wb == NULL) {
    printf("--- Waring: %s %s\n", xls_getError(error), f.data());
    return {};
  }
  for (int sheetIdx = 0; sheetIdx < wb->sheets.count; sheetIdx++) {
    string_view sheet_name = wb->sheets.sheet[sheetIdx].name;
    auto sheet = xls_getWorkSheet(wb, sheetIdx);
    if (auto err = xls_parseWorkSheet(sheet); err != LIBXLS_OK) {
      printf("--- Waring: %s %s %s\n", xls_getError(err), f.data(),
             sheet_name.data());
      return {};
    }

    int rowEnd = sheet->rows.lastrow, colEnd = sheet->rows.lastcol;
    for (int rowIdx = 0; rowIdx <= rowEnd; rowIdx++) {
      auto row = xls_row(sheet, rowIdx);
      auto cells = row->cells.cell;
      auto found = false;
      for (int colIdx = 0; colIdx <= colEnd; colIdx++) {
        auto c = cells[colIdx];
        if (c.str && strstr(c.str, tex.data())) {
          os << "===\n";
          char buf[255];
          sprintf(buf, "file:%-16s sheet:%-8s row:%-4d col:%-4s  cell:%s\n",
                  file_base.data(), sheet_name.data(), rowIdx + 1,
                  ColName(colIdx).c_str(), c.str);
          os << buf;
          found = true;
        }
      }
      // output the row
      if (found) {
        os << string(80, '-') << endl;
        for (int colIdx = 0; colIdx <= colEnd; colIdx++) {
          auto c = cells[colIdx];
          if (colIdx) os << ",";
          os << (c.str ? c.str : "<NULL>");
        }
        os << endl;
      }
    }
  }
  xls_close_WB(wb);
  return os.str();
}

void search(string_view path, string_view tex) {
  Profiler pro{"search"};
  vector<string> items{};
  for (const auto& i : recursive_directory_iterator(path)) {
    const auto& s = i.path().string();
    if (s.ends_with(".xls")) {
      items.push_back(s);
    }
  }
  printf("searching in %s\n", path.data());
  printf("%s\n", string(80, '=').c_str());
  printf("total files to scan: %zd\n", items.size());
  for_each(execution::par, items.begin(), items.end(), [&](auto& f) {
    auto s = search_file(f, tex);
    printf("%s", s.c_str());
  });
}

void usage(const char* app) {
  printf("Usage: %s <xls path> <text>\n", app);
  printf("  or set XLS_SEARCH_DIR env variable for interactive mode\n");
}

int main(int argc, const char* argv[]) {
  SetConsoleOutputCP(CP_UTF8);
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
