#include <headers/commands.hpp>
using namespace std;

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    cerr << unitbuf;
    
    cerr << "Logs from your program will appear here!\n";
    if (argc < 2)
    {
        cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    return handleCommands(argc, argv);
}
