#include <headers/commands.hpp>
using namespace std;

int handleCommands(int argc, char *argv[])
{
    if (argc < 2)
    {
        return error("No command provided");
    }

    string command = argv[1];
    if (command == "init")
        return handleInit(argc, argv);
    else if (command == "cat-file")
        return handleCatFile(argc, argv);
    else if (command == "hash-object")
        return handleHashObject(argc, argv);
    else if (command == "ls-tree")
        return handleLsTree(argc, argv);
    else if (command == "write-tree")
        return handleWriteTree(argc, argv);
    else if (command == "commit-tree")
        return handleCommitTree(argc, argv);
    else if (command == "clone")
        return handleClone(argc, argv);
    else
    {
        return error("Unknown command " + command);
    }

    return EXIT_SUCCESS;
}