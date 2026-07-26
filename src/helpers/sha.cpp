#include <headers/commands.hpp>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
using namespace std;

string sha1_byte(const string &input)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(input.data()), input.size(), hash);
    return string(reinterpret_cast<char *>(hash), SHA_DIGEST_LENGTH);
}

string sha1(const string &input)
{
    auto raw = sha1_byte(input);
    ostringstream ss;
    ss << hex << setfill('0');
    for (unsigned char byte : raw)
    {
        ss << setw(2) << static_cast<unsigned int>(byte);
    }
    return ss.str();
}