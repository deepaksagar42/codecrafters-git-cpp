#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!\n";

    // TODO: Uncomment the code below to pass the first stage
    //
    if (argc < 2) {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    
    std::string command = argv[1];
    if (command == "init") {
        try {
            std::filesystem::create_directory(".git");
            std::filesystem::create_directory(".git/objects");
            std::filesystem::create_directory(".git/refs");
    
            std::ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                std::cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
    
            std::cout << "Initialized git directory\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        } }
    else if(command=="cat-file") {
            std:: string hash= argv[3];
            std::string dir= hash.substr(0,2);
            std::string file = hash.substr(2);
            std:: string path = ".git/objects/" + dir + "/" + file; 
            std::ifstream objectFile(path, std::ios::binary);
            if (!objectFile)
            {
                std::cerr << "Failed to open object file\n";
                return EXIT_FAILURE;
            }
            std::string compressed(
            (std::istreambuf_iterator<char>(objectFile)),
             std::istreambuf_iterator<char>()
             );
             uLongf decompressedSize = compressed.size() * 20; // large enough buffer
            std::string decompressed(decompressedSize, '\0');

            int result = uncompress(
                reinterpret_cast<Bytef*>(&decompressed[0]),
                &decompressedSize,
                reinterpret_cast<const Bytef*>(compressed.data()),
                compressed.size()
            );

            if (result != Z_OK)
            {
                std::cerr << "Failed to decompress object\n";
                return EXIT_FAILURE;
            }

            decompressed.resize(decompressedSize);
            size_t pos = decompressed.find('\0');
            if (pos == std::string::npos)
            {
                std::cerr << "Invalid object\n";
                return EXIT_FAILURE;
            }

            std::cout << decompressed.substr(pos + 1);
    }
    else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
