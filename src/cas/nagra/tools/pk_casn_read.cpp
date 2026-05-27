#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>

typedef unsigned int uint;
typedef unsigned char uchar;

namespace fs = std::filesystem;

enum FILE_FORMAT_TYPE
{
    UNDEFINED = 0,
    FILE_TRUNCATED = 8,
    FILE_IS_WHOLE = 12,
};

int main(int argc, const char **const argv)
{
    for(int i = 1; i < argc; i++)
    {
        if(fs::exists(argv[i]))
        {
            auto file = std::ifstream(argv[i], std::ios::binary);

            if(file.is_open())
            {
                FILE_FORMAT_TYPE file_type = UNDEFINED;

                for(auto format_version_location :
                        {
                            FILE_TRUNCATED, FILE_IS_WHOLE
                        })
                {
                    char format_version[4];
                    file.seekg(format_version_location);
                    file.read(format_version, 2);

                    if(file.gcount() != 2)
                    {
                        std::cerr << "Error reading format version " << argv[i] << ":" << (file.tellg() - 2l) << "\n";
                        continue;
                    }

                    if(format_version[0] == 0x03 and format_version[1] == 0x03)
                    {
                        file_type = format_version_location;
                        break;
                    }
                }

                if(UNDEFINED == file_type)
                {
                    std::cerr << "File format not found " << argv[i] << "\n";
                    continue;
                }

                file.seekg(file_type - 4 - 4);
                uint32_t record_length = 0;
                file.read((char *)&record_length, 4);
                file.seekg(file_type - 4);
                uint32_t casn;
                file.read((char *)&casn, 4);
                printf("Record Length: %u CA SN: %08x\n", std::byteswap(record_length), casn);
            }
            else
            {
                std::cerr << "Unable to open '" << argv[i] << "'\n";
            }
        }
        else
        {
            std::cerr << "File does not exist '" << argv[i] << "'\n";
        }
    }

    return EXIT_SUCCESS;
}
