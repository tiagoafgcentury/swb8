extern "C" {
#include <ca_dpt.h>
#include "cak/ca_cak.h"
#include "cak/nv_debug.h"
#include "cak/ngwm_ree.h"
#include "cak/ca_sec.h"
#include "cak/ca_cert.h"
#include "cak/ca_dmx.h"

    // This definition is missing and causes compiler error in C++
    enum ECsdScsSize
    {
        ESCS_NAND_BOOT,
        ESCS_NOR_BOOT,
        ESCS_UNPROGRAMMED_BOOT,
        ESCS_NOT_SUPPORT_BOOT,
    };

#include <nocs_csd_impl.h>
#include <nocs_csd.h>
}

#undef min
#undef max

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "common/mb_globals.h"

using namespace std;

int main(int argc, char **argv)
{
    if(argc > 2)
    {
        auto bn = basename(argv[0]);
        cerr << "Usage:\n\t" << bn << "        - reads CASN\n\t" << bn << " [CASN] - writes CASN" << endl;
        return EXIT_FAILURE;
    }

    auto ret = csdInitialize(nullptr); // This function does (almost) nothing

    if(CSD_NO_ERROR != ret)
    {
        cerr << "csdInitialize() returned error: " << ret << endl;
        return EXIT_FAILURE;
    }

    auto _ = mb::deferred_call([] { csdTerminate(nullptr); });

    switch(argc)
    {
        case 1: // Read CASN
        {
            TCsd4BytesVector casn;
            ret = csdGetStbCaSn(casn);

            if(CSD_NO_ERROR != ret)
            {
                cerr << "csdGetStbCaSn() returned error: " << ret << endl;
                return EXIT_FAILURE;
            }

            cout << hex
                 << setw(2) << setfill('0') << (int)casn[0]
                 << setw(2) << setfill('0') << (int)casn[1]
                 << setw(2) << setfill('0') << (int)casn[2]
                 << setw(2) << setfill('0') << (int)casn[3]
                 << endl;
            return EXIT_SUCCESS;
        }

        case 2: // Write CASN
        {
            if(strlen(argv[1]) != 8)
            {
                cerr << "[CASN] should be 8 chars long" << endl;
                return EXIT_FAILURE;
            }

            char buffer[3];
            buffer[2] = 0;
            TCsd4BytesVector casn;
            buffer[0] = argv[1][0];
            buffer[1] = argv[1][1];
            casn[0] = strtol(buffer, nullptr, 16);
            buffer[0] = argv[1][2];
            buffer[1] = argv[1][3];
            casn[1] = strtol(buffer, nullptr, 16);
            buffer[0] = argv[1][4];
            buffer[1] = argv[1][5];
            casn[2] = strtol(buffer, nullptr, 16);
            buffer[0] = argv[1][6];
            buffer[1] = argv[1][7];
            casn[3] = strtol(buffer, nullptr, 16);
            auto ret = csdSetStbCaSn(casn);

            if(CSD_NO_ERROR != ret)
            {
                switch(ret)
                {
                    case CSD_ERROR_INVALID_PARAMETERS:
                        cerr << "The passed parameter is invalid." << endl;
                        break;

                    case CSD_ERROR_OPERATION_NOT_ALLOWED:
                    {
                        TCsd4BytesVector other;
                        csdGetStbCaSn(other);
                        cerr << "The STBCASN has already been set to a different value (" << other << ") and has been locked." << endl;
                        break;
                    }

                    case CSD_ERROR:
                        cerr << "The function terminated abnormally. The intended operation failed." << endl;
                        break;

                    default:
                        cerr << "Unknown error." << endl;
                        break;
                }

                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }

        default:
            return EXIT_FAILURE;
    }
}
