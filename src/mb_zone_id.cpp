#include "mb_zone_id.h"
#include "tasks/mb_task.h"

#include <stdio.h>
#include "common/mb_config.h"

namespace mb {

Zone_ID Zone_ID::s_instance;

Zone_ID::Zone_ID()
{
    auto fp = fopen(MBGUI_ZONE_ID_FILE, "rb");

    if(fp)
    {
        auto ret = fread(&m_zoneid_claro, sizeof(Zone_ID_t), 1, fp);

        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fread() failed: " << ret << "\n");
            fclose(fp);
            return;
        }

        Zone_ID_t sky_val = 0;
        ret = fread(&sky_val, sizeof(Zone_ID_t), 1, fp);
        m_zoneid_sky = sky_val;

        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fread() failed: " << ret << "\n");
            fclose(fp);
            return;
        }

        fclose(fp);
    }
    else
    {
        DEBUG_MSG(COMMON, DEBUG, "Error opening " MBGUI_ZONE_ID_FILE ": " << strerror(errno) << "\n");
    }
}

Zone_ID::~Zone_ID()
{
}

void Zone_ID::write_zone_id(Satellite_Operator _oper)
{
    auto fp = fopen(MBGUI_ZONE_ID_FILE, "r+b");
    if (!fp)
    {
        fp = fopen(MBGUI_ZONE_ID_FILE, "wb");
        if (fp)
        {
            fwrite(&m_zoneid_claro, sizeof(Zone_ID_t), 1, fp);
            Zone_ID_t sky_val = static_cast<Zone_ID_t>(m_zoneid_sky);
            fwrite(&sky_val, sizeof(Zone_ID_t), 1, fp);
            fclose(fp);
        }
        else
        {
            DEBUG_MSG(COMMON, DEBUG, "Error opening " MBGUI_ZONE_ID_FILE ": " << strerror(errno) << "\n");
        }
        return;
    }

    if (_oper == Satellite_Operator::Sky)
    {
        fseek(fp, sizeof(Zone_ID_t), SEEK_SET);
        Zone_ID_t sky_val = static_cast<Zone_ID_t>(m_zoneid_sky);
        auto ret = fwrite(&sky_val, sizeof(Zone_ID_t), 1, fp);
        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fwrite() failed: " << ret << "\n");
        }
    }
    else
    {
        fseek(fp, 0, SEEK_SET);
        auto ret = fwrite(&m_zoneid_claro, sizeof(Zone_ID_t), 1, fp);
        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fwrite() failed: " << ret << "\n");
        }
    }
    fclose(fp);
}

void Zone_ID::set_zone_id(const Zone_ID_t _new_zone_id, const Segment_ID_t _new_segment_id)
{
    auto config = Config::get_config();
    Satellite_Operator oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;

    bool changed = false;
    uint16_t old_val = 0;
    uint16_t new_val = 0;

    DEBUG_MSG(COMMON, INFO, "_new_zone_id: " << _new_zone_id << " _new_segment_id: " << _new_segment_id << "\n");
    if (oper == Satellite_Operator::Sky)
    {
        old_val = s_instance.m_zoneid_sky;
        new_val = _new_segment_id;
        if (s_instance.m_zoneid_sky != _new_segment_id)
        {
            s_instance.m_zoneid_sky = _new_segment_id;
            changed = true;
        }
    }
    else
    {
        old_val = s_instance.m_zoneid_claro;
        new_val = _new_zone_id;
        if (s_instance.m_zoneid_claro != _new_zone_id)
        {
            s_instance.m_zoneid_claro = _new_zone_id;
            changed = true;
        }
    }

    if (changed)
    {
        DEBUG_MSG(COMMON, INFO, "old_val: " << old_val << " new_val: " << new_val << "\n");
        s_instance.write_zone_id(oper);
        Task::post_event_zone_id_changed(old_val, new_val);
    }
}

uint16_t Zone_ID::get_zone_id(std::optional<Satellite_Operator> _oper)
{
    Satellite_Operator oper;
    if (_oper.has_value())
    {
        oper = _oper.value();
    }
    else
    {
        auto config = Config::get_config();
        oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    }

     switch(oper)
    {
        case Satellite_Operator::Claro:
            return s_instance.m_zoneid_claro;

        case Satellite_Operator::Sky:
            return s_instance.m_zoneid_sky;

        case Satellite_Operator::Generic:
        default:
            return 0;
    }
}

} // namespace mb
