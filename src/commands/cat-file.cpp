#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;

int handleCatFile(int argc, char *argv[])
{
    if (argc < 4)
    {
       return error("Usage: cat-file -p <object_sha>");
    }
    string mode = argv[2];
    if (mode != "-p")
    {
        cerr << mode << '\n';
        return EXIT_FAILURE;
    }
    string arg = static_cast<string>(argv[3]);
    if (arg.size() !=40)
    {
        return error("Invalid object hash.");
    }
    string dir = arg.substr(0, 2);
    string file_name = arg.substr(2);
    path objectPath = ".git/objects/" + dir + "/" + file_name;
    if (!exists(objectPath))
    {
        return error("Object not found.");
    }
    string content;
    try
    {
        content = readFileContents(objectPath.string());
    }
    catch (const exception &e)
    {
        return error(string("Failed to open object file: ") + e.what());
    }

    string decompressed;
    try
    {
        decompressed = decompressZlib(content);
    }
    catch (const exception &e)
    {
        return error("Failed to decompress object file.");
    }

    size_t nullPos = decompressed.find('\0'); // size_t is unsgined , int is signed
    if (nullPos != string::npos)
    {
        cout << decompressed.substr(nullPos + 1);
    }
    else
    {
        cout << decompressed;
    }
    return EXIT_SUCCESS;
}