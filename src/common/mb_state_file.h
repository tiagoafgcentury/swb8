#pragma once

#include "mb_globals.h"
#include "mb_types.h"
#include "mb_assert.h"
#include "mb_config.h"

#ifndef MB_DEFAULT_STANDBY_VALUE
    #define MB_DEFAULT_STANDBY_VALUE true
#endif

namespace mb {

class State_File
{
public:
    struct App_State_File
    {
        App_State_File(bool *_loaded_ok = nullptr)
        {
            auto fp = fopen(MBGUI_CONFIG_FILE, "rb");

            if(fp)
            {
#ifdef CHECK_CONFIG_FILE_SIZE
                fseek(fp, 0, SEEK_END);
                auto size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
#endif // CHECK_CONFIG_FILE_SIZE
                uint8_t disk_file_version = 0;
                auto disk_file_version_read = fread(&disk_file_version, sizeof(disk_file_version), 1, fp);

                if(disk_file_version_read != sizeof(disk_file_version))
                {
                    // Invalid read, invalidate version
                    disk_file_version = ~ file_version;
                }

                fseek(fp, 0, SEEK_SET);

                if(
#ifdef CHECK_CONFIG_FILE_SIZE
                    size != sizeof(App_State_File) or
#endif // #ifdef CHECK_CONFIG_FILE_SIZE
                    disk_file_version != file_version)
                {
                    migrate_file_from(disk_file_version);
                }
                else
                {
                    fseek(fp, 0, SEEK_SET);
                    char *ptr = reinterpret_cast<char *>(this);
                    auto read_size = sizeof(*this);
                    auto ret = fread(ptr, read_size, 1, fp);

                    if(ret == 0)
                    {
                        DEBUG_MSG(COMMON, ERROR, "fread() failed: " << ret << "\n");
                        return;
                    }

                    fclose(fp);

                    if(_loaded_ok)
                    {
                        *_loaded_ok = true;
                    }
                }
            }
        }

        void migrate_file_from(uint /*_file_version*/)
        {
            //this->file_version = _file_version;
            //write();
        }

        bool write()
        {
            auto fp = fopen(MBGUI_CONFIG_FILE, "wb");
            mb_assert(fp);

            if(fp)
            {
                const char *ptr = reinterpret_cast<char *>(this);
                auto ret = fwrite(ptr, sizeof(*this), 1, fp);
                mb_assert(ret > 0);

                if(ret == 0)
                {
                    return false;
                }

                fclose(fp);
                return (ret == 1);
            }

            return false;
        }

        uint8_t file_version { 3 };
        Service_ID_t service_id { 0 };
        Transponder_Id::Id_Type transponder_id { 0 };
        Volume_t volume { 50 };
        uint8_t mute { 0 };
        char time_zone { static_cast<uint8_t>(Timezone_Mode::Brasilia_UTC_3) };
        uint8_t clock_status { static_cast<uint8_t>(Clock_Type::Auto) };
        uint8_t language_mode { static_cast<uint8_t>(Language_Mode::Portugues) };
        uint8_t resolution { static_cast<uint8_t>(Resolution_Standard::_1080i_30Hz) };
        uint8_t color_standard { static_cast<uint8_t>(Color_Standard::PAL_M_60) };
        uint8_t aspect_mode { static_cast<uint8_t>(Aspect_Mode::AUTO) };
        NID_t network_id { Network_Id_Claro };
        Band band = Band::Ku;
        LNBF_Type lnbf_type = LNBF_Type::Universal;
        bool lnbf_inverted = false;
        bool stand_by = MB_DEFAULT_STANDBY_VALUE;
        bool easy_install_finish = false;
        uint8_t channel_list_type { static_cast<uint8_t>(Channel_List_Type::MY_TV_CHANNELS) };
        Viewer_Channel_t current_channel;
        bool stand_by_in_production_mode = true;
        uint8_t parental_rating { 0 }; // Parental control disabled by default
        char parental_password[5] { '0', '0', '0', '0', '\0'};
        uint16_t current_satellite_id { 0 };
        Subtitle_Font_Size subtitle_font_size { Subtitle_Font_Size::Medium };
        uint8_t subtitle_font_color { 0 }; // OSD_COLOR_WHITE
        uint8_t subtitle_background_color { 0 }; // OSD_COLOR_GREY
        uint8_t slide_show {1}; // Desabilitado por padrão
        uint8_t slide_show_transition_time {0};
        uint8_t slide_show_transition_type {0};
        uint8_t slide_show_aspect_ratio {0};
        uint8_t satellite_flags { 0 }; // bit 0: Sky, bit 1: Claro, bit 2

    } __attribute__((packed));
};

}
