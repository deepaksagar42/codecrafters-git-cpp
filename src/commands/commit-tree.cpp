#include <headers/commands.hpp>
#include <ctime>
#include <vector>
#include <iostream>

using namespace std;
using namespace std::filesystem;

int handleCommitTree(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "Usage: commit-tree <tree_sha> [-p <parent_sha>]* -m <message>\n";
        return EXIT_FAILURE;
    }

    string tree_sha = argv[2];
    vector<string> parents;
    string message = "";
    bool has_message = false;

    for (int i = 3; i < argc; ++i)
    {
        string arg = argv[i];
        if (arg == "-p")
        {
            if (i + 1 < argc)
            {
                parents.push_back(argv[++i]);
            }
            else
            {
                cerr << "Error: -p option requires a parent commit SHA\n";
                return EXIT_FAILURE;
            }
        }
        else if (arg == "-m")
        {
            if (i + 1 < argc)
            {
                message = argv[++i];
                has_message = true;
            }
            else
            {
                cerr << "Error: -m option requires a message\n";
                return EXIT_FAILURE;
            }
        }
        else
        {
            cerr << "Error: Unknown argument " << arg << "\n";
            return EXIT_FAILURE;
        }
    }

    if (!has_message)
    {
        cerr << "Error: -m option with a commit message is required\n";
        return EXIT_FAILURE;
    }

    // Build the commit object contents
    string content = "tree " + tree_sha + "\n";
    for (const auto &parent : parents)
    {
        content += "parent " + parent + "\n";
    }

    auto now = time(nullptr);
    string timestamp = to_string(now);
    // Format: name <email> timestamp timezone
    content += "author Jane Doe <jane@example.com> " + timestamp + " +0000\n";
    content += "committer Jane Doe <jane@example.com> " + timestamp + " +0000\n";
    content += "\n";
    content += message + "\n";

    // Format commit object header and data
    string header = "commit " + to_string(content.size()) + '\0';
    string full_object = header + content;

    // Hash, compress, and save the object
    string sha = sha1(full_object);
    string dir = sha.substr(0, 2);
    string file_name = sha.substr(2);
    string zlib_compressed = compressZlib(full_object);

    try
    {
        create_directories(".git/objects/" + dir);
    }
    catch (const exception &e)
    {
        return error("Failed to create object directory: " + string(e.what()));
    }

    string path = ".git/objects/" + dir + "/" + file_name;
    try
    {
        writeFileContents(path, zlib_compressed);
    }
    catch (const exception &e)
    {
        return error(string("Failed to write object file: ") + e.what());
    }

    cout << sha << endl;
    return EXIT_SUCCESS;
}
