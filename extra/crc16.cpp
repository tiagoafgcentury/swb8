#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

// global variable to store the CRC table (faster computation)
static unsigned short crc_tabccitt[256];

void init_crcccitt_tab()
{
    for(int i = 0; i < 256; i++)
    {
        unsigned short crc = 0;
        unsigned short c = ((unsigned short) i) << 8;

        for(int j = 0; j < 8; j++)
        {
            if((crc ^ c) & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }

            c = c << 1;
        }

        crc_tabccitt[i] = crc;
    }
}

unsigned short update_crc_ccitt(unsigned short crc, char c)
{
    unsigned short short_c = 0x00ff & (unsigned short)c;
    unsigned short tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc_tabccitt[tmp];
    return crc;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cerr << "Missing file name" << endl;
        return EXIT_FAILURE;
    }

    init_crcccitt_tab();
    unsigned short calc_crc16 = 0;
    ifstream in(argv[1]);

    if(not in.is_open())
    {
        cerr << "Unable to open: " << argv[1] << endl;
        return EXIT_FAILURE;
    }

    while(not in.eof())
    {
        char c;
        in.read(&c, 1);
        calc_crc16 = update_crc_ccitt(calc_crc16, c);
    }

    cout << hex << setfill('0') << setw(4) << calc_crc16 << endl;
    return EXIT_SUCCESS;
}
