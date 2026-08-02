#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;
int handleLsTree(int argc, char *argv[])
{
    if (argc < 4)
    {
       return error("Usage: ls-tree --name-only <tree_sha>");
    }
    string mode = argv[2];
    if (mode != "--name-only")
    {
       return error("mode not supported");
    }
    string sha = argv[3];
    string dir = sha.substr(0, 2);
    string file_name = sha.substr(2);
    string path = ".git/objects/" + dir + "/" + file_name;
    string content;
    try
    {
        content = readFileContents(path);
    }
    catch (const exception &e)
    {
        return error(string("cannot open file ") + path + ": " + e.what());
    }
    string decompressed = decompressZlib(content);
    //   this will be format after decompress
    //    tree <size>\0
    //   <mode> <name>\0<20_byte_sha>
    //   <mode> <name>\0<20_byte_sha>
    int i = decompressed.find('\0') + 1;
        while (i < decompressed.size())
        {
            if (decompressed[i] == ' ')
            {
                int st = i + 1;
                int en = decompressed.find('\0', st);
                cout << decompressed.substr(st, en - st) << '\n';
                i = en + 1 + 20;
                continue;
            }
            i++;
        }
    return EXIT_SUCCESS;
}