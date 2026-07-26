#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;
int handleWriteTree(int argc, char *argv[])
{
    path p;
    vector<string> blobs;
    p = findRepoRoot(current_path());

    error_code ec;
    if (!exists(p, ec))
    {
        return error("Path does not exist: " + p.string());
    }
    string sha = map_directory(p, ec).second;
    cout << sha << endl;
    return 0;
}