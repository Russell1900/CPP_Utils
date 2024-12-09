#pragma once
#include <string>
#include <vector>

namespace path_helper {
std::string to_lowercase(const std::string &s);
std::string filename(const std::string &filepath);
bool startswith(const std::string &path, const std::string &pattern);
bool endswith(const std::string &path, const std::string &pattern);
extern std::string strip(const std::string &line, char pattern = ' ');
extern std::string normalize_path(const std::string &path);
extern std::string join_path(const std::string &p1, const std::string &p2);
extern void list_files(const std::string &dir_path,
                       std::vector<std::string> &files, bool recursive = true);
extern void read_file(const std::string &file_path, const char *&data,
                      int &len);

#ifdef WIN32
extern std::string unicode2utf8(const std::wstring &unicode_str);
#endif

} // namespace path_helper
