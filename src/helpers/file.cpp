#include <headers/commands.hpp>
#include <filesystem>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::filesystem;

string readFileContents(const string &pathStr)
{
    std::ifstream ifs(pathStr, ios::binary);
    if (!ifs)
        throw runtime_error("Failed to open file: " + pathStr);
    string content((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    return content;
}

void writeFileContents(const string &pathStr, const string &data)
{
    std::filesystem::path p(pathStr);
    if (pathStr.find(".git/objects/") != string::npos && std::filesystem::exists(p))
    {
        return;
    }
    if (!p.parent_path().empty())
        std::filesystem::create_directories(p.parent_path());

    std::ofstream ofs(pathStr, ios::binary | ios::trunc);
    if (!ofs)
        throw runtime_error("Failed to open file for writing: " + pathStr);

    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
}

string makeTreeBlobFromFile(const string &filePath, const string &entryName)
{
    error_code ec;
    auto st = status(path(filePath), ec);
    if (ec)
    {
        throw runtime_error("Error getting status: " + ec.message());
    }
    auto pms = st.permissions();
    int mode = REGULAR_FILE;
    if ((pms & perms::owner_exec) != perms::none ||
        (pms & perms::group_exec) != perms::none ||
        (pms & perms::others_exec) != perms::none)
    {
        mode = EXECUTABLE_FILE;
    }
    string name = entryName;
    return to_tree_blob(name, mode, readFileContents(filePath));
}

string write_tree_blob(const string &content)
{
    string tree_blob = "tree " + to_string(content.size()) + '\0' + content;
    string zlib_compressed = compressZlib(tree_blob);
    string sha = sha1(tree_blob);
    string dir = sha.substr(0, 2);
    string file_name = sha.substr(2);
    string path = ".git/objects/" + dir + "/" + file_name;
    writeFileContents(path, zlib_compressed);
    return sha;
}

std::pair<string, string> map_directory(path p, std::error_code   ec) //passsing ec by refernce
{
     string content = "";
     if (is_directory(p, ec))
    {
        vector<directory_entry> entries;
        for (auto &entry : directory_iterator(p, ec))
        {
            if (entry.path().filename().string() == ".git")
                continue;
            entries.push_back(entry);
        }

        sort(entries.begin(), entries.end(), [](const directory_entry &a, const directory_entry &b)
             { return a.path().filename().string() < b.path().filename().string(); });

        for (auto &entry : entries)
        {
            if (ec)
            {
                throw runtime_error("Error reading directory: " + ec.message());
            }
            if (entry.is_directory())
            {
                auto [child_content, child_sha] = map_directory(entry.path(), ec);
                content += to_string(DIRR) + " " + entry.path().filename().string() + '\0' +
                           sha1_byte("tree " + to_string(child_content.size()) + '\0' + child_content);
            }
            else
            {
                content += makeTreeBlobFromFile(entry.path().string(), entry.path().filename().string());
            }
        }
    }
     else
     {
        content = makeTreeBlobFromFile(p.string(), p.filename().string());
      }
      string sha = write_tree_blob(content);
      return {content, sha};
  }

path findRepoRoot(path start)
{
    error_code ec;
    path cur = absolute(start, ec);
    while (!cur.empty())
    {
        if (exists(cur / ".git", ec))
            return cur;
        if (cur == cur.root_path())
            break;
        cur = cur.parent_path();
    }
    return start;
}