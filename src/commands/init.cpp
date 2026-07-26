#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;

int handleInit(int argc, char *argv[])
{
    try
    {
        create_directory(".git");
        create_directory(".git/objects");
        create_directory(".git/refs");
        try
        {
            writeFileContents(".git/HEAD", string("ref: refs/heads/main\n"));
        }
        catch (const exception &e)
        {
            return error(string("Failed to create .git/HEAD file: ") + e.what());
        }
        cout << "Initialized git directory\n";
        return EXIT_SUCCESS;
    }
    catch (const filesystem_error &e)
    {
        return error(string(e.what()));
    }

    return EXIT_SUCCESS;
}