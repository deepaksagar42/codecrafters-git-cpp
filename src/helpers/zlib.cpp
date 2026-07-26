#include <headers/commands.hpp>
#include <zlib.h>
#include <vector>
using namespace std;

string decompressZlib(const string &compressed)
{
    z_stream zs{};
    if (inflateInit(&zs) != Z_OK)
    {
        throw runtime_error("inflateInit failed");
    }

    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(compressed.data()));
    zs.avail_in = static_cast<uInt>(compressed.size());

    string out;
    vector<char> buffer(32768);

    int ret;
    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(buffer.data());
        zs.avail_out = static_cast<uInt>(buffer.size());

        ret = inflate(&zs, 0);
        if (ret != Z_OK && ret != Z_STREAM_END)
        {
            inflateEnd(&zs);
            throw runtime_error("inflate failed");
        }

        out.append(buffer.data(), buffer.size() - zs.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&zs);
    return out;
}

string compressZlib(const string &input)
{
    z_stream zs{};
    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        throw runtime_error("deflateInit failed");
    }

    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
    zs.avail_in = static_cast<uInt>(input.size());

    string out;
    vector<char> buffer(32768);

    int ret;
    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(buffer.data());
        zs.avail_out = static_cast<uInt>(buffer.size());

        ret = deflate(&zs, Z_FINISH);
        if (ret != Z_OK && ret != Z_STREAM_END)
        {
            deflateEnd(&zs);
            throw runtime_error("deflate failed");
        }

        out.append(buffer.data(), buffer.size() - zs.avail_out);
    } while (ret != Z_STREAM_END);

    deflateEnd(&zs);
    return out;
}