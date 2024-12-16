#include "spdlog/spdlog.h"
#include <fstream>
#ifdef WIN32
#include <filesystem>
#include <io.h>
#include <windows.h>
namespace fs = ::std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = ::std::experimental::filesystem;
#endif

using namespace std;

namespace path_helper {

string to_lowercase(const string &s) {
  string ret = s;
  for (char &c : ret) {
    c = ::tolower(c);
  }
  return ret;
}

string filename(const string &filepath) {
  return fs::path(filepath).filename().string();
}

bool startswith(const string &path, const string &pattern) {
  if (path.size() < pattern.size()) {
    return false;
  }
  return (path.substr(0, pattern.size()) == pattern);
}

bool endswith(const string &path, const string &pattern) {
  if (path.size() < pattern.size()) {
    return false;
  }
  return (path.substr(path.size() - pattern.size()) == pattern);
}

string strip(const string &line, char pattern) {
  string ret_line;
  ret_line = line;
  while (ret_line.front() == pattern) {
    ret_line = ret_line.substr(1);
  }
  while (ret_line.back() == pattern) {
    ret_line = ret_line.substr(0, ret_line.size() - 1);
  }
  return ret_line;
}

string normalize_path(const string &path) {
  string ret = strip(path, ' ');
  if (ret.back() == '/') {
    ret = ret.substr(0, ret.size() - 1);
  }
  return ret;
}

string join_path(const string &p1, const string &p2) {
  string ret = normalize_path(p1) + "/" + normalize_path(p2);
  return ret;
}

void list_files_(const string &dir_path, vector<string> &files) {
  fs::path dir(dir_path);
  for (const auto &file : fs::directory_iterator(dir)) {
    if (fs::is_regular_file(file)) {
      files.emplace_back(file.path().string());
    }
  }
}

void list_files_recursive_(const string &dir_path, vector<string> &files) {
  fs::path dir(dir_path);
  for (const auto &file : fs::directory_iterator(dir)) {
    if (fs::is_regular_file(file)) {
      files.emplace_back(file.path().string());
    }
  }
}

#ifdef WIN32
string unicode2utf8(const wstring &unicode_str) {
  int utf8_len = WideCharToMultiByte(CP_UTF8, 0, unicode_str.c_str(),
                                     unicode_str.length(), NULL, 0, NULL, NULL);
  std::vector<char> utf8_str(utf8_len);
  WideCharToMultiByte(CP_UTF8, 0, unicode_str.c_str(), unicode_str.length(),
                      &utf8_str[0], utf8_len, NULL, NULL);
  return string(utf8_str.begin(), utf8_str.end());
}
#endif

//
// void iter_dirfiles(const std::string input_path, vector<string> &dirlist) {
//  auto d = opendir(input_path.c_str());
//  if (d) {
//    for (auto dir = readdir(d); dir != NULL; dir = readdir(d)) {
//      if ((strcmp(dir->d_name, ".") == 0) || (strcmp(dir->d_name, "..") == 0))
//      {
//        continue;
//      }
//      struct stat path_stat;
//      auto p = join_path(input_path, dir->d_name);
//      stat(p.c_str(), &path_stat);
//      if (S_ISDIR(path_stat.st_mode)) {
//        dirlist.push_back(p);
//        // recursivily
//        iter_dirfiles(p, dirlist);
//      }
//    }
//    closedir(d);
//  }
//  // only when there is no sub folder
//  if (dirlist.size() == 0) {
//    dirlist.push_back(input_path);
//  }
//}
//
// void list_files_(const string &dir_path, vector<string> &files) {
//  auto d = opendir(dir_path.c_str());
//  if (d) {
//    for (auto file = readdir(d); file != NULL; file = readdir(d)) {
//      if (file->d_type == DT_REG) {
//        files.push_back(join_path(dir_path, file->d_name));
//      }
//    }
//  }
//}
//
// void list_files_recursive_(const string &dir_path, vector<string> &files) {
//  vector<string> images;
//  vector<string> dirs;
//  iter_dirfiles(dir_path, dirs);
//  for (auto dir : dirs) {
//    list_files_(dir, files);
//  }
//}

void list_files(const string &dir_path, vector<string> &files, bool recursive) {
  if (recursive) {
    list_files_recursive_(dir_path, files);
  } else {
    list_files_(dir_path, files);
  }
}

void read_file(const string &file_path, const char *&data, int &len) {
  ifstream ins(file_path, ios::binary);
  if (ins) {
    ins.seekg(0, ins.end);
    len = ins.tellg();
    ins.seekg(0, ins.beg);
    char *tmp = new char[len];
    ins.read(tmp, len);
    data = tmp;
    ins.close();
  } else {
    spdlog::critical("failed to open file: {}", file_path);
    exit(1);
  }
}

} // namespace path_helper
