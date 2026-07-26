#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>
#include <utility>
#include <filesystem>
#include <system_error>

std::string decompressZlib(const std::string &compressed);
std::string sha1(const std::string &input);
std::string sha1_byte(const std::string &input);
std::string compressZlib(const std::string &input);
int error(const std::string &msg);
// File helpers
std::string readFileContents(const std::string &path);
void writeFileContents(const std::string &path, const std::string &data);
std::string makeTreeBlobFromFile(const std::string &filePath, const std::string &entryName);
std::string to_tree_blob(const std::string &name, int mode, const std::string &content);
std::string write_tree_blob(const std::string &content);
std::pair<std::string, std::string> map_directory(std::filesystem::path p, std::error_code ec);
std::filesystem::path findRepoRoot(std::filesystem::path start);
#endif