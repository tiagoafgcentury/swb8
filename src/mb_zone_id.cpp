#include "mb_zone_id.h"
#include "tasks/mb_task.h"

#include <stdio.h>

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

        ret = fread(&m_zoneid_sky, sizeof(Zone_ID_t), 1, fp);

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

void Zone_ID::write_zone_id(Satellite_Operator _operator, Zone_ID_t _zone_id)
{
    switch(_operator)
    {
        case Satellite_Operator::Claro:
            m_zoneid_claro = _zone_id;
            break;

        case Satellite_Operator::Sky:
            m_zoneid_sky = _zone_id;
            break;

        default:
            break;
    }

    auto fp = fopen(MBGUI_ZONE_ID_FILE, "wb");

    if(fp)
    {
        auto ret = fwrite(&m_zoneid_claro, sizeof(Zone_ID_t), 1, fp);

        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fwrite() failed: " << ret << "\n");
            return;
        }

        ret = fwrite(&m_zoneid_sky, sizeof(Zone_ID_t), 1, fp);

        if(ret == 0)
        {
            DEBUG_MSG(COMMON, ERROR, "fwrite() failed: " << ret << "\n");
            return;
        }

        fclose(fp);
    }
    else
    {
        DEBUG_MSG(COMMON, DEBUG, "Error opening " MBGUI_ZONE_ID_FILE ": " << strerror(errno) << "\n");
    }
}

void Zone_ID::set_zone_id(Satellite_Operator _operator, const Zone_ID_t _new_zone_id)
{
    const auto old_zone_id = s_instance.get_zone_id(_operator);

    if(_new_zone_id != old_zone_id)
    {
        s_instance.write_zone_id(_operator, _new_zone_id);
        Task::post_event_zone_id_changed(old_zone_id, _new_zone_id);
    }
}

Zone_ID_t Zone_ID::get_zone_id(Satellite_Operator _operator)
{
     switch(_operator)
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
