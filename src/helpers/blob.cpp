#include <headers/commands.hpp>
using namespace std;
using namespace std::filesystem;
string to_tree_blob(const string &name, int mode, const string &content) {
  string blobObj = "blob " + to_string(content.size()) + '\0' + content;
  string blob_string = to_string(mode) + " " + name + '\0' + sha1_byte(blobObj);
  string sha = sha1(blobObj);
  string dir = sha.substr(0, 2);
  string file = sha.substr(2);
  string objPath = ".git/objects/" + dir + "/" + file;
  writeFileContents(objPath, compressZlib(blobObj));
  return blob_string;
}