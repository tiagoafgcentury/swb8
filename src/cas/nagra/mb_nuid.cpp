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
    if(argc != 1)
    {
        auto bn = basename(argv[0]);
        cerr << "Usage:\n\t" << bn << "        - reads NUID" << endl;
        return EXIT_FAILURE;
    }

    auto ret = csdInitialize(nullptr); // This function does (almost) nothing

    if(CSD_NO_ERROR != ret)
    {
        cerr << "csdInitialize() returned error: " << ret << endl;
        return EXIT_FAILURE;
    }

    auto _ = mb::deferred_call([] { csdTerminate(nullptr); });
    TCsd4BytesVector nuid;
    ret = csdGetNuid(nuid);

    if(CSD_NO_ERROR != ret)
    {
        cerr << "csdGetNuid() returned error: " << ret << endl;
        return EXIT_FAILURE;
    }

    cout << hex
         << setw(2) << setfill('0') << (int)nuid[0]
         << setw(2) << setfill('0') << (int)nuid[1]
         << setw(2) << setfill('0') << (int)nuid[2]
         << setw(2) << setfill('0') << (int)nuid[3]
         << endl;
    return EXIT_SUCCESS;
}
