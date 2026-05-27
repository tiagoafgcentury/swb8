#include <iostream>
#include <fstream>
#include <filesystem>
#include <string_view>

using std::cout;

constexpr auto FILE_PREFIX = std::string_view("file://");
constexpr auto QUANTITY_PREFIX = std::string_view("Quantity=");
constexpr auto RECORD_SIZE_PREFIX = std::string_view("Record_size=");

int main(int argc, const char **argv)
{
    for(int i = 1; i < argc; i++)
    {
        auto pk_file_name = std::string_view(argv[i]);

        if(pk_file_name.starts_with(FILE_PREFIX))
        {
            pk_file_name.remove_prefix(FILE_PREFIX.length());
        }

        if(!std::filesystem::exists(pk_file_name))
        {
            cout << "File not found: " << pk_file_name << "\n";
            return EXIT_FAILURE;
        }

        auto ini_name = std::string(pk_file_name) + "_info.ini";

        if(!std::filesystem::exists(ini_name))
        {
            cout << "Ini File not found: " << ini_name << "\n";
            return EXIT_FAILURE;
        }

        auto record_size = 0;
        auto count = 0;
        {
            auto ini = std::ifstream(ini_name);

            while(ini.good() and (record_size == 0 or count == 0))
            {
                std::string line;
                std::getline(ini, line);

                if(line.starts_with(QUANTITY_PREFIX))
                {
                    count = atoi(line.c_str() + QUANTITY_PREFIX.length());
                    cout << "Quantity = " << count << "\n";
                }
                else if(line.starts_with(RECORD_SIZE_PREFIX))
                {
                    record_size = atoi(line.c_str() + RECORD_SIZE_PREFIX.length());
                    cout << "Record size = " << record_size << "\n";
                }
            }

            if(record_size == 0)
            {
                cout << "Missing Record Size in ini file: " << ini_name << "\n";
                return EXIT_FAILURE;
            }

            if(count == 0)
            {
                cout << "Missing Quantity in ini file: " << count << "\n";
                return EXIT_FAILURE;
            }
        }
        int pk_num = 0;

        for(auto pk_file = std::ifstream(pk_file_name.data(), std::ios::binary); pk_file.good() and pk_num < count; pk_num++)
        {
            char record[record_size];
            auto total_read = 0;
            auto remain = record_size;

            while(remain > 0)
            {
                auto read = pk_file.readsome(record + total_read, remain);
                total_read += read;
                remain -= read;
            }

            if(total_read != record_size)
            {
                cout << "Error reading record: read " << total_read << " expected " << record_size << "\n";
                return EXIT_FAILURE;
            }

            if(pk_file.tellg() % record_size != 0)
            {
                cout << "Read position error: " << pk_file.tellg() << " % " << pk_file.tellg() % record_size << "\n";
                return EXIT_FAILURE;
            }

            // Length = first four bytes
            auto this_record_size = (record[0] << 24) + (record[1] << 16) + (record[2] << 8) + (record[3]);

            if(this_record_size != record_size - 4 - 2)  // - 4 record size/length, - 2 crc
            {
                cout << "Rocord size mismatch: ini = " << record_size << " pk record = " << this_record_size << "\n";
                return EXIT_FAILURE;
            }

            char out_file_name[FILENAME_MAX];
            snprintf(out_file_name, sizeof(out_file_name), "0303.pk.%d", pk_num);

            if(std::filesystem::exists(out_file_name))
            {
                cout << "File already exists. Skip: " << out_file_name << "\n";
                continue;
            }

            std::ofstream out_file(out_file_name, std::ios::binary);

            if(!out_file.good())
            {
                cout << "Error creating file.  Skip: " << out_file_name << "\n";
                continue;
            }

            // +4 Skip Length
            out_file.write(record + 4, this_record_size);
            cout << out_file_name << " created with " << out_file.tellp() << "bytes.\n";
        }
    }

    return EXIT_SUCCESS;
}
