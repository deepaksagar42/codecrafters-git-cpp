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
    string sha = static_cast<string>(argv[3]);
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
    size_t nullPos = decompressed.find('\0');
    if (nullPos == string::npos)
    {
        return error("Invalid object format: header separator not found.");
    }
    size_t i = nullPos + 1;
    while (i < decompressed.size())
    {
        // Find space after mode
        size_t spacePos = decompressed.find(' ', i);
        if (spacePos == string::npos)
            break;
        string mode = decompressed.substr(i, spacePos - i);

        // Find null after name
        size_t nullPosEntry = decompressed.find('\0', spacePos);
        if (nullPosEntry == string::npos)
            break;
        string name = decompressed.substr(spacePos + 1, nullPosEntry - spacePos - 1);

        // Skip SHA (20 bytes)
        i = nullPosEntry + 1 + 20;

        // Print name
        cout << name << endl;
    }
    return EXIT_SUCCESS;
}