#include <headers/commands.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace std::filesystem;

static string bytesToHex(const string &bytes)
{
    ostringstream ss;
    ss << hex << setfill('0');
    for (unsigned char byte : bytes)
    {
        ss << setw(2) << static_cast<unsigned int>(byte);
    }
    return ss.str();
}

static string execCommand(const string &cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw runtime_error("popen failed");

    string result;
    char buffer[4096];
    while (true)
    {
        size_t bytesRead = fread(buffer, 1, sizeof(buffer), pipe);
        if (bytesRead == 0) break;
        result.append(buffer, bytesRead);
    }
    pclose(pipe);
    return result;
}

static void checkoutTree(const string &tree_sha, const path &dest_dir, const string &repo_git_dir)
{
    string dir = tree_sha.substr(0, 2);
    string file_name = tree_sha.substr(2);
    path objectPath = repo_git_dir + "/objects/" + dir + "/" + file_name;

    string content = decompressZlib(readFileContents(objectPath.string()));
    size_t nullPos = content.find('\0');
    if (nullPos == string::npos) return;

    size_t i = nullPos + 1;
    while (i < content.size())
    {
        size_t spacePos = content.find(' ', i);
        if (spacePos == string::npos) break;

        string mode_str = content.substr(i, spacePos - i);

        size_t nullPosEntry = content.find('\0', spacePos);
        if (nullPosEntry == string::npos) break;

        string name = content.substr(spacePos + 1, nullPosEntry - spacePos - 1);
        string sha_bytes = content.substr(nullPosEntry + 1, 20);
        string sha_hex = bytesToHex(sha_bytes);

        path entry_dest = dest_dir / name;
        int mode = stoi(mode_str);

        if (mode == 40000) // Directory / Tree
        {
            create_directories(entry_dest);
            checkoutTree(sha_hex, entry_dest, repo_git_dir);
        }
        else // Blob
        {
            string blob_dir = sha_hex.substr(0, 2);
            string blob_file = sha_hex.substr(2);
            path blob_path = repo_git_dir + "/objects/" + blob_dir + "/" + blob_file;
            string blob_data = decompressZlib(readFileContents(blob_path.string()));

            size_t blob_nullPos = blob_data.find('\0');
            string file_content = "";
            if (blob_nullPos != string::npos)
            {
                file_content = blob_data.substr(blob_nullPos + 1);
            }
            else
            {
                file_content = blob_data;
            }

            writeFileContents(entry_dest.string(), file_content);

            if (mode == 100755)
            {
                permissions(entry_dest, perms::owner_exec | perms::group_exec | perms::others_exec, perm_options::add);
            }
        }

        i = nullPosEntry + 1 + 20;
    }
}

int handleClone(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "Usage: clone <repository_url> <target_directory>\n";
        return EXIT_FAILURE;
    }

    string repo_url = argv[2];
    if (!repo_url.empty() && repo_url.back() == '/')
    {
        repo_url.pop_back();
    }
    path target_dir = argv[3];
    path repo_git_dir = target_dir / ".git";

    // 1. Initialize local repository directory structure
    try
    {
        create_directories(repo_git_dir / "objects");
        create_directories(repo_git_dir / "refs/heads");
    }
    catch (const exception &e)
    {
        return error("Failed to create target directories: " + string(e.what()));
    }

    // 2. Fetch references (GET request)
    string info_refs_url = repo_url + "/info/refs?service=git-upload-pack";
    string refs_cmd = "curl -s -L '" + info_refs_url + "'";
    string refs_data;
    try
    {
        refs_data = execCommand(refs_cmd);
    }
    catch (const exception &e)
    {
        return error("Failed to fetch references: " + string(e.what()));
    }

    string head_sha = "";
    string branch_name = "master"; // Default

    size_t offset = 0;
    while (offset < refs_data.size())
    {
        if (offset + 4 > refs_data.size()) break;
        string len_str = refs_data.substr(offset, 4);
        int length = stoi(len_str, nullptr, 16);
        if (length == 0)
        {
            offset += 4;
            continue;
        }

        string payload = refs_data.substr(offset + 4, length - 4);
        size_t head_pos = payload.find(" HEAD");
        if (head_pos != string::npos && head_pos == 40)
        {
            head_sha = payload.substr(0, 40);

            size_t symref_pos = payload.find("symref=HEAD:");
            if (symref_pos != string::npos)
            {
                size_t space_pos = payload.find(' ', symref_pos);
                size_t null_pos = payload.find('\0', symref_pos);
                size_t nl_pos = payload.find('\n', symref_pos);
                size_t end_pos = string::npos;
                if (space_pos != string::npos) end_pos = space_pos;
                if (null_pos != string::npos && null_pos < end_pos) end_pos = null_pos;
                if (nl_pos != string::npos && nl_pos < end_pos) end_pos = nl_pos;

                string full_ref = payload.substr(symref_pos + 12, end_pos - (symref_pos + 12));
                if (full_ref.rfind("refs/heads/", 0) == 0)
                {
                    branch_name = full_ref.substr(11);
                }
            }
        }
        offset += length;
    }

    if (head_sha.empty())
    {
        return error("Failed to parse HEAD SHA from references.");
    }

    // Write initial HEAD file
    writeFileContents((repo_git_dir / "HEAD").string(), "ref: refs/heads/" + branch_name + "\n");

    // 3. Negotiate and fetch packfile (POST request)
    string want_line = "want " + head_sha + " side-band-64k\n";
    ostringstream want_pkt;
    want_pkt << setw(4) << setfill('0') << hex << (want_line.size() + 4) << want_line;
    string post_body = want_pkt.str() + "00000009done\n";

    path temp_post_body_path = repo_git_dir / "temp_post_body.bin";
    writeFileContents(temp_post_body_path.string(), post_body);

    string upload_pack_url = repo_url + "/git-upload-pack";
    string pack_cmd = "curl -s -L -X POST -H \"Content-Type: application/x-git-upload-pack-request\" -H \"Accept: application/x-git-upload-pack-result\" --data-binary @" + temp_post_body_path.string() + " '" + upload_pack_url + "'";

    string response_data;
    try
    {
        response_data = execCommand(pack_cmd);
    }
    catch (const exception &e)
    {
        remove(temp_post_body_path);
        return error("Failed to fetch packfile: " + string(e.what()));
    }
    remove(temp_post_body_path);

    // 4. Parse response sideband streams to extract raw packfile
    string pack_data = "";
    size_t post_offset = 0;
    while (post_offset < response_data.size())
    {
        if (post_offset + 4 > response_data.size()) break;
        string len_str = response_data.substr(post_offset, 4);
        int length = stoi(len_str, nullptr, 16);
        if (length == 0)
        {
            post_offset += 4;
            continue;
        }

        string payload = response_data.substr(post_offset + 4, length - 4);
        if (!payload.empty())
        {
            unsigned char band = payload[0];
            if (band == 1) // Packfile data stream
            {
                pack_data += payload.substr(1);
            }
        }
        post_offset += length;
    }

    if (pack_data.empty())
    {
        return error("Received empty packfile from server.");
    }

    path temp_pack_path = repo_git_dir / "temp.pack";
    writeFileContents(temp_pack_path.string(), pack_data);

    // 5. Unpack objects
    string unpack_cmd = "cd " + target_dir.string() + " && git unpack-objects < .git/temp.pack";
    int unpack_ret = system(unpack_cmd.c_str());
    remove(temp_pack_path);

    if (unpack_ret != 0)
    {
        return error("Failed to unpack downloaded Git objects.");
    }

    // Write references branch SHA
    writeFileContents((repo_git_dir / "refs/heads" / branch_name).string(), head_sha + "\n");

    // 6. Checkout files recursively starting from the HEAD commit's tree
    string commit_dir = head_sha.substr(0, 2);
    string commit_file = head_sha.substr(2);
    path commit_path = repo_git_dir / "objects" / commit_dir / commit_file;

    string commit_content = decompressZlib(readFileContents(commit_path.string()));
    size_t commit_nullPos = commit_content.find('\0');
    if (commit_nullPos == string::npos)
    {
        return error("Invalid commit object: null separator not found.");
    }
    string commit_data = commit_content.substr(commit_nullPos + 1);

    string tree_sha = "";
    if (commit_data.rfind("tree ", 0) == 0)
    {
        tree_sha = commit_data.substr(5, 40);
    }

    if (tree_sha.empty())
    {
        return error("Failed to parse tree SHA from HEAD commit.");
    }

    try
    {
        checkoutTree(tree_sha, target_dir, repo_git_dir.string());
    }
    catch (const exception &e)
    {
        return error("Failed to checkout repository tree: " + string(e.what()));
    }

    // 7. Initialize Git index by running git read-tree HEAD
    string read_tree_cmd = "cd " + target_dir.string() + " && git read-tree HEAD";
    (void)system(read_tree_cmd.c_str());

    return EXIT_SUCCESS;
}
