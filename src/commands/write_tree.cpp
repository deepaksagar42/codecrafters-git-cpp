#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;
int handleWriteTree(int argc, char *argv[])
{
    path p;
    vector<string> blobs;
    p = findRepoRoot(current_path());// current_path is standalone function in filesystem namespace

    error_code ec;   //its store the error rather than throwing exception 1 means error 0 means no error also it sire the ec.value, ec,category
    if (!exists(p, ec))
    {
        if(ec)
        {
            std::cerr << "OS error accessing path" << ec.message() << "\n";
            return;
        }
        std::cerr << "Path does not exist: " + p.string() << "\n";
        return ;
    }
    string sha = map_directory(p, ec).second;
    cout << sha << endl;
    return 0;
}