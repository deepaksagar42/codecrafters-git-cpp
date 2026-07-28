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
        return error(string(e.what()));   // e.what() is a standard method used to get a human-readable description of an exception (error)
    }

    return EXIT_SUCCESS;   //0 means succefful execution, 1 means error
}