#include "hal/mb_hdmi.h"

#include <aui_hdmi.h>

#include "mb_ali_globals.h"
#include "common/mb_globals.h"

#include <atomic>
#include <cstring>

namespace {

// based on /opt/ali/buildroot-10.2.1.29/output/build/aui-2.7.L10.2.1-20231220-Auto/samples/sample_src/aui_hdmi_test.c

static mb::HDMI::cec_opcode get_response_opcode(mb::HDMI::cec_opcode opcode)
{
    switch(opcode)
    {
        case mb::HDMI::CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
            return mb::HDMI::CEC_OPCODE_ACTIVE_SOURCE;

        case mb::HDMI::CEC_OPCODE_GET_CEC_VERSION:
            return mb::HDMI::CEC_OPCODE_CEC_VERSION;

        case mb::HDMI::CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
            return mb::HDMI::CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;

        case mb::HDMI::CEC_OPCODE_GET_MENU_LANGUAGE:
            return mb::HDMI::CEC_OPCODE_SET_MENU_LANGUAGE;

        case mb::HDMI::CEC_OPCODE_GIVE_DECK_STATUS:
            return mb::HDMI::CEC_OPCODE_DECK_STATUS;

        case mb::HDMI::CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS:
            return mb::HDMI::CEC_OPCODE_TUNER_DEVICE_STATUS;

        case mb::HDMI::CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
            return mb::HDMI::CEC_OPCODE_DEVICE_VENDOR_ID;

        case mb::HDMI::CEC_OPCODE_GIVE_OSD_NAME:
            return mb::HDMI::CEC_OPCODE_SET_OSD_NAME;

        case mb::HDMI::CEC_OPCODE_MENU_REQUEST:
            return mb::HDMI::CEC_OPCODE_MENU_STATUS;

        case mb::HDMI::CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
            return mb::HDMI::CEC_OPCODE_REPORT_POWER_STATUS;

        case mb::HDMI::CEC_OPCODE_GIVE_AUDIO_STATUS:
            return mb::HDMI::CEC_OPCODE_REPORT_AUDIO_STATUS;

        case mb::HDMI::CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            return mb::HDMI::CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS;

        case mb::HDMI::CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
            return mb::HDMI::CEC_OPCODE_SET_SYSTEM_AUDIO_MODE;

        default:
            break;
    }

    return mb::HDMI::CEC_OPCODE_NONE;
}

static void data_dump(unsigned long addr, unsigned char *data, unsigned int len)
{
    for(auto i = 0u; i < len; i++)
    {
        if(i % 16 == 0)
        {
            DEBUG_MSG_NL(HAL, DEBUG, hex << setw(8) << setfill('0') << (addr + i) << "   ");
        }

        DEBUG_MSG_NL(HAL, DEBUG, hex << setw(2) << setfill('0') << (int)(*(data + i)));

        if((i + 1) % 8 == 0)
        {
            DEBUG_MSG_NL(HAL, DEBUG, "  ");
        }

        if((i + 1) % 16 == 0)
        {
            DEBUG_MSG_NL(HAL, DEBUG, "|\n");
        }
    }

    DEBUG_MSG_NL(HAL, DEBUG, "\n");
}

}

namespace mb {

struct HDMI::Data
{
    aui_hdl hnd { nullptr };

    static void edid_ready_callback(void *_user_data)
    {
        auto thiz = static_cast<HDMI *>(_user_data);
        auto hnd = thiz->m_p->hnd;
        unsigned int edid_len = 0;
        ALI_EXEC(aui_hdmi_get(hnd, AUI_HDMI_EDID_LEN_GET, &edid_len, nullptr));
        unsigned char edid_buf[edid_len];
        MB_ZERO(edid_buf);
        ALI_EXEC(aui_hdmi_ediddata_read(hnd, (unsigned long *)edid_buf, &edid_len, AUI_HDMI_RAW_EDID_ALL));
        DEBUG_MSG(HAL, DEBUG, "----------------------------------------\n");

        for(auto i = 0u; i < edid_len; i += HDMI_EDID_BLOCK_LENGTH)
        {
            data_dump(i, &edid_buf[i], HDMI_EDID_BLOCK_LENGTH);
            DEBUG_MSG_NL(HAL, DEBUG, "\n");
        }

        // Check EDID Header
        unsigned char EDID_HEADER[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

        if(edid_len > 8 && memcmp(edid_buf, EDID_HEADER, 8) == 0)
        {
            char c1 = (edid_buf[8] & 0b01111100) >> 2;
            char c2 = ((edid_buf[8] & 0b00000011) << 3) | ((edid_buf[9] & 0b11100000) >> 5);
            char c3 = (edid_buf[9] & 0b00011111);
            char year = edid_buf[17];
            DEBUG_MSG(HAL, DEBUG, "Manufactorer ID: " << hex << setw(2) << (int)c1 << setw(2) << (int)c2 << setw(2) << (int)c3 <<
                      "\nYear: " << dec << year + 1990 <<
                      "\nSerial: " << hex << setw(4) << *reinterpret_cast<uint32_t *>(&edid_buf[12]) <<
                      endl);
        }

        DEBUG_MSG(HAL, DEBUG, "----------------------------------------\n\n");
    }

    static void hdcp_fail_callback(unsigned char *puc_param1, unsigned char uc_param2, void *pv_user_data)
    {
        DEBUG_MSG(HAL, INFO, "CEC MESSAGE: " << puc_param1 << "\t|\t0x" << hex << setfill('0') << setw(2) << (int)uc_param2 << endl);
        ((HDMI *)(pv_user_data))->m_p->send_cec_respond(puc_param1);
    }

    static void cec_msg_callback(unsigned char *puc_param1, unsigned char uc_param2, void *pv_user_data)
    {
        DEBUG_MSG(HAL, INFO, "CEC MESSAGE: " << puc_param1 << "\t|\t0x" << hex << setfill('0') << setw(2) << (int)uc_param2 << endl);
        ((HDMI *)(pv_user_data))->m_p->send_cec_respond(puc_param1);
    }

    int send_cec_respond(unsigned char *buf);
    HDMI::cec_message_state send_cec_command(const cec_command *data);

    int report_physical_addr()
    {
        AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
        unsigned short hdmi_phy_addr;
        unsigned char hdmi_logical_addr = 0;
        ret |= aui_hdmi_get(hnd, AUI_HDMI_PHYSICAL_ADDR_GET, &hdmi_phy_addr, nullptr);
        ret |= aui_hdmi_get(hnd, AUI_HDMI_LOGICAL_ADDR_GET, &hdmi_logical_addr, nullptr);
        cec_command cmd;
        cmd.initiator          = (cec_logical_address)(hdmi_logical_addr);
        cmd.destination        = (cec_logical_address)(CEC_DEVICE_BROADCAST);
        cmd.opcode             = (cec_opcode)CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
        cmd.parameters.data[0] = (hdmi_phy_addr >> 8) & 0xFF;
        cmd.parameters.data[1] = (hdmi_phy_addr >> 0) & 0xFF;
        cmd.parameters.data[2] = hdmi_logical_addr;
        cmd.parameters.size    = 3;
        cmd.opcode_set         = 1;

        if(send_cec_command(&cmd) != CEC_MESSAGE_STATE_SENT_ACKED)
        {
            ret = -1;
        }

        return ret;
    }

    int report_vendor_id()
    {
        AUI_RTN_CODE ret                = AUI_RTN_SUCCESS;
        unsigned char hdmi_logical_addr = 0;
        cec_command cmd;
        unsigned long cec_vendor_id = 0xCECECE; // Fake ID
        ret |= aui_hdmi_get(hnd, AUI_HDMI_LOGICAL_ADDR_GET, &hdmi_logical_addr, nullptr);
        cmd.initiator          = (cec_logical_address)(hdmi_logical_addr);
        cmd.destination        = (cec_logical_address)(CEC_DEVICE_BROADCAST);
        cmd.opcode             = (cec_opcode)CEC_OPCODE_DEVICE_VENDOR_ID;
        cmd.parameters.data[0] = (cec_vendor_id >> 16) & 0xFF;
        cmd.parameters.data[1] = (cec_vendor_id >> 8) & 0xFF;
        cmd.parameters.data[2] = (cec_vendor_id >> 0) & 0xFF;
        cmd.parameters.size    = 3;
        cmd.opcode_set         = 1;

        if(send_cec_command(&cmd) != CEC_MESSAGE_STATE_SENT_ACKED)
        {
            ret = -1;
        }

        return ret;
    }


};

HDMI::cec_message_state HDMI::Data::send_cec_command(const cec_command *data)
{
    unsigned char buffer[CEC_MAX_FRAME_SIZE];
    int size             = 1;
    cec_message_state rc = CEC_MESSAGE_STATE_SENT_NOT_ACKED;

    if((size_t)data->parameters.size + data->opcode_set > sizeof(buffer))
    {
        DEBUG_MSG(HAL, ERROR, " buffer too small for data");
        return CEC_MESSAGE_STATE_ERROR;
    }

    buffer[0] = (data->initiator << 4) | (data->destination & 0x0f);

    if(data->opcode_set)
    {
        buffer[1] = data->opcode;
        size++;
        memcpy(&buffer[size], data->parameters.data, data->parameters.size);
        size += data->parameters.size;
    }

    if(aui_hdmi_cec_transmit(hnd, (unsigned char *)buffer, size) == AUI_RTN_SUCCESS)
    {
        rc = CEC_MESSAGE_STATE_SENT_ACKED;
    }
    else
    {
        rc = CEC_MESSAGE_STATE_SENT_NOT_ACKED;
    }

    return rc;
}

int HDMI::Data::send_cec_respond(unsigned char *buf)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned short hdmi_phy_addr;
    unsigned char hdmi_logical_addr = 0;
    cec_command cmd;
    cec_command rep;
    ret = aui_hdmi_get(hnd, AUI_HDMI_PHYSICAL_ADDR_GET, &hdmi_phy_addr, nullptr);
    ret = aui_hdmi_get(hnd, AUI_HDMI_LOGICAL_ADDR_GET, &hdmi_logical_addr, nullptr);
    auto len = strlen((const char *)buf);

    if(len > 0)
    {
        cmd.initiator   = (cec_logical_address)(buf[0] >> 4);
        cmd.destination = (cec_logical_address)(buf[0] & 0xF);
        rep.initiator   = cmd.destination;
        rep.destination = cmd.initiator;
    }

    if(len > 1)
    {
        cmd.opcode     = (cec_opcode)buf[1];
        cmd.opcode_set = 1;
        rep.opcode     = get_response_opcode(cmd.opcode);
    }

    switch(rep.opcode)
    {
        case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:
            ret = report_physical_addr();
            break;

        case CEC_OPCODE_DEVICE_VENDOR_ID:
            ret = report_vendor_id();
            break;

        case CEC_OPCODE_CEC_VERSION:
            rep.opcode_set         = 1;
            rep.parameters.data[0] = CEC_VERSION_1_4;
            rep.parameters.size    = 1;
            DEBUG_MSG(HAL, DEBUG, "<CEC VERSON>\n");

            if(send_cec_command(&rep) != CEC_MESSAGE_STATE_SENT_ACKED)
            {
                DEBUG_MSG(HAL, ERROR, "Send response fail\n");
                ret = -1;
            }

            break;

        case CEC_OPCODE_SET_OSD_NAME:
        {
            rep.opcode_set = 1;
            strncpy((char *)rep.parameters.data, MBGUI_PRODUCT_NAME, sizeof(rep.parameters.data));
            rep.parameters.size = strlen(MBGUI_PRODUCT_NAME);
            DEBUG_MSG(HAL, DEBUG, "<OSD Name>\n");

            if(send_cec_command(&rep) != CEC_MESSAGE_STATE_SENT_ACKED)
            {
                DEBUG_MSG(HAL, ERROR, "Send response fail\n");
                ret = -1;
            }

            break;
        }

        case CEC_OPCODE_REPORT_POWER_STATUS:
            rep.opcode_set         = 1;
            rep.parameters.data[0] = 0x90;
            rep.parameters.data[1] = 0x00;
            rep.parameters.size    = 2;
            DEBUG_MSG(HAL, DEBUG, "Report Power Status\n");

            if(send_cec_command(&rep) != CEC_MESSAGE_STATE_SENT_ACKED)
            {
                DEBUG_MSG(HAL, ERROR, "Send response fail\n");
                ret = -1;
            }

            break;

        default:
            break;
    }

    return ret;
}

HDMI::HDMI():
    m_p(std::make_unique<Data>())
{
    ALI_EXEC(aui_hdmi_init(nullptr));
    aui_attr_hdmi attr_hdmi;
    MB_ZERO(attr_hdmi);
    ALI_EXEC(aui_hdmi_open(&attr_hdmi, &m_p->hnd));
    ALI_EXEC(aui_hdmi_hdcp_err_ctrl_by_mw_off(&m_p->hnd));
    //ALI_EXEC(aui_hdmi_callback_reg(m_p->hnd, AUI_HDMI_CB_EDID_READY, (void *)Data::edid_ready_callback, this));
    ALI_EXEC(aui_hdmi_callback_reg(m_p->hnd, AUI_HDMI_CB_CEC_MESSAGE, (void *)Data::cec_msg_callback, this));
    ALI_EXEC(aui_hdmi_callback_reg(m_p->hnd, AUI_HDMI_CB_HDCP_FAIL, (void *)Data::hdcp_fail_callback, this));
    // cec_command rep;
    // MB_ZERO(rep);
    // rep.initiator
    // rep.destination
    // rep.opcode              = CEC_OPCODE_REPORT_POWER_STATUS;
    // rep.opcode_set          = 1;
    // rep.parameters.data[0]  = 0x90;
    // rep.parameters.data[1]  = 0x00;
    // rep.parameters.size     = 2;
    // if (m_p->send_cec_command(&rep) != CEC_MESSAGE_STATE_SENT_ACKED)
    // {
    //     DEBUG_MSG("Send response fail\n");
    // }
}

HDMI::~HDMI()
{
    ALI_EXEC(aui_hdmi_de_init(nullptr));
}

void HDMI::hdmi_callback(int /*event*/, void */*_data*/)
{
}

void HDMI::hdmi_init()
{
}

bool HDMI::is_hdmi_connected() const
{
    aui_hdmi_link_status link_status = AUI_HDMI_STATUS_UNLINK;
    ALI_EXEC(aui_hdmi_get(m_p->hnd, AUI_HDMI_CONNECT_INFO_GET, &link_status, NULL));

    switch(link_status)
    {
        /**
         Value to specify that HDMI is <b> not connected </b> to the HDMI sink device
         */
        case AUI_HDMI_STATUS_UNLINK:
            return false;

        /**
         Value to specify that HDMI is <b> connected </b> to the HDMI sink device
        */
        case AUI_HDMI_STATUS_LINK:
            return true;

        /**
         Value to specify that HDMI is <b> connected </b> to the HDMI sink device
        and the <b> HDCP Authentication </b> has been performed @b successfully
        */
        case AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED:
            return true;

        /**
         Value to specify that HDMI is <b> connected </b> to the HDMI sink device
         and the <b> HDCP Authentication failed </b> for some reasons
        */
        case AUI_HDMI_STATUS_LINK_HDCP_FAILED:
            return true;

        /**
         Value to specify that HDMI is <b> connected </b> to the HDMI sink device
         and the <b> HDCP Authentication </b> has been @b ignored
        */
        case AUI_HDMI_STATUS_LINK_HDCP_IGNORED:
            return true;

        /**
        Value to specify the <b> total number </b> of <b> HDMI Connections </b>
        */
        case AUI_HDMI_STATUS_MAX:
            return false;
    }

    return false;
}

void HDMI::hdmi_output_on()
{
    //if (is_hdmi_connected())
    {
        unsigned int on = 0;
        ALI_EXEC(aui_hdmi_on(m_p->hnd));
        ALI_EXEC(aui_hdmi_audio_on(m_p->hnd));
        ALI_EXEC(aui_hdmi_set(m_p->hnd, AUI_HDMI_AV_UNMUTE_SET, NULL, NULL));
        ALI_EXEC(aui_hdmi_set(m_p->hnd, AUI_HDMI_IOCT_SET_AV_BLANK, NULL, &on));
    }
}

void HDMI::hdmi_output_off()
{
    //if (is_hdmi_connected())
    {
        unsigned int off = 1;
        ALI_EXEC(aui_hdmi_audio_on(m_p->hnd));
        ALI_EXEC(aui_hdmi_off(m_p->hnd));
        ALI_EXEC(aui_hdmi_set(m_p->hnd, AUI_HDMI_AV_MUTE_SET, NULL, NULL));
        ALI_EXEC(aui_hdmi_set(m_p->hnd, AUI_HDMI_IOCT_SET_AV_BLANK, NULL, &off));
    }
}

}
