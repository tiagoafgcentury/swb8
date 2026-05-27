#include "../mb_system.h"
#include "common/mb_globals.h"

#include <mt_common.h>
#include <mt_unf_pm.h>

#include <cstring>
#include <ctime>
#include <atomic>

#ifndef NDEBUG
#include <mt_unf_misc.h>
#endif

SINGULARITY_TOKEN

namespace mb {

System::System()
{
    SINGULARITY_CHECK(false);
    //TODO Set correct Time Zone
    g_timezone_offset = -3;
    g_clock_set = 0;
    MT_EXEC(mt_sys_init());
#ifndef NDEBUG
    mt_sys_version_s sys_chip_info;
    MB_ZERO(sys_chip_info);
    MT_EXEC(mt_sys_get_version(&sys_chip_info));
    const char *value { getenv("BOARD") };

    if((!value) || (strlen(value) <= 0) || (strlen(value) > 20))
    {
        value = "[not set]";
    }

    std::cerr << "Chip: " << sys_chip_info.enChipVersion << " Board: " << value << std::endl;

    if(mt_chip_is_symphony2(sys_chip_info.enChipVersion))
    {
        std::cerr << "Chip is Symphony 2\n" << std::endl;
    }
    else if(mt_chip_is_symphony4(sys_chip_info.enChipVersion))
    {
        std::cerr << "Chip is Symphony 4\n" << std::endl;
    }
    else
    {
        std::cerr << "Chip is Symphony 1\n" << std::endl;
    }

    MT_EXEC(MT_UNF_PMOC_Init());
    MT_EXEC(MT_UNF_PMOC_SetDevType(FD650));
    //uint32_t temp = 0;
    //ret = mt_unf_misc_temperature_get(&temp);
#endif
}

System::~System()
{
    MT_EXEC(MT_UNF_PMOC_DeInit());
    MT_EXEC(mt_sys_deinit());
    SINGULARITY_CHECK(true);
}

void System::poweroff()
{
    MT_EXEC(MT_UNF_PMOC_SwitchSystemMode());
}


} // namespace mb
