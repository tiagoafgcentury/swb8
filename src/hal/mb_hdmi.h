#pragma once

#include <memory>

namespace mb {

class HDMI
{
public:
    static constexpr auto HDMI_EDID_BLOCK_LENGTH = 128u;
    static constexpr auto CEC_MAX_FRAME_SIZE = 16u;
    static constexpr auto CEC_MAX_DATA_PACKET_SIZE = (16u * 4u);

    enum cec_opcode
    {
        CEC_OPCODE_ACTIVE_SOURCE                 = 0x82,
        CEC_OPCODE_IMAGE_VIEW_ON                 = 0x04,
        CEC_OPCODE_TEXT_VIEW_ON                  = 0x0D,
        CEC_OPCODE_INACTIVE_SOURCE               = 0x9D,
        CEC_OPCODE_REQUEST_ACTIVE_SOURCE         = 0x85,
        CEC_OPCODE_ROUTING_CHANGE                = 0x80,
        CEC_OPCODE_ROUTING_INFORMATION           = 0x81,
        CEC_OPCODE_SET_STREAM_PATH               = 0x86,
        CEC_OPCODE_STANDBY                       = 0x36,
        CEC_OPCODE_RECORD_OFF                    = 0x0B,
        CEC_OPCODE_RECORD_ON                     = 0x09,
        CEC_OPCODE_RECORD_STATUS                 = 0x0A,
        CEC_OPCODE_RECORD_TV_SCREEN              = 0x0F,
        CEC_OPCODE_CLEAR_ANALOGUE_TIMER          = 0x33,
        CEC_OPCODE_CLEAR_DIGITAL_TIMER           = 0x99,
        CEC_OPCODE_CLEAR_EXTERNAL_TIMER          = 0xA1,
        CEC_OPCODE_SET_ANALOGUE_TIMER            = 0x34,
        CEC_OPCODE_SET_DIGITAL_TIMER             = 0x97,
        CEC_OPCODE_SET_EXTERNAL_TIMER            = 0xA2,
        CEC_OPCODE_SET_TIMER_PROGRAM_TITLE       = 0x67,
        CEC_OPCODE_TIMER_CLEARED_STATUS          = 0x43,
        CEC_OPCODE_TIMER_STATUS                  = 0x35,
        CEC_OPCODE_CEC_VERSION                   = 0x9E,
        CEC_OPCODE_GET_CEC_VERSION               = 0x9F,
        CEC_OPCODE_GIVE_PHYSICAL_ADDRESS         = 0x83,
        CEC_OPCODE_GET_MENU_LANGUAGE             = 0x91,
        CEC_OPCODE_REPORT_PHYSICAL_ADDRESS       = 0x84,
        CEC_OPCODE_SET_MENU_LANGUAGE             = 0x32,
        CEC_OPCODE_DECK_CONTROL                  = 0x42,
        CEC_OPCODE_DECK_STATUS                   = 0x1B,
        CEC_OPCODE_GIVE_DECK_STATUS              = 0x1A,
        CEC_OPCODE_PLAY                          = 0x41,
        CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS      = 0x08,
        CEC_OPCODE_SELECT_ANALOGUE_SERVICE       = 0x92,
        CEC_OPCODE_SELECT_DIGITAL_SERVICE        = 0x93,
        CEC_OPCODE_TUNER_DEVICE_STATUS           = 0x07,
        CEC_OPCODE_TUNER_STEP_DECREMENT          = 0x06,
        CEC_OPCODE_TUNER_STEP_INCREMENT          = 0x05,
        CEC_OPCODE_DEVICE_VENDOR_ID              = 0x87,
        CEC_OPCODE_GIVE_DEVICE_VENDOR_ID         = 0x8C,
        CEC_OPCODE_VENDOR_COMMAND                = 0x89,
        CEC_OPCODE_VENDOR_COMMAND_WITH_ID        = 0xA0,
        CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN     = 0x8A,
        CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP       = 0x8B,
        CEC_OPCODE_SET_OSD_STRING                = 0x64,
        CEC_OPCODE_GIVE_OSD_NAME                 = 0x46,
        CEC_OPCODE_SET_OSD_NAME                  = 0x47,
        CEC_OPCODE_MENU_REQUEST                  = 0x8D,
        CEC_OPCODE_MENU_STATUS                   = 0x8E,
        CEC_OPCODE_USER_CONTROL_PRESSED          = 0x44,
        CEC_OPCODE_USER_CONTROL_RELEASE          = 0x45,
        CEC_OPCODE_GIVE_DEVICE_POWER_STATUS      = 0x8F,
        CEC_OPCODE_REPORT_POWER_STATUS           = 0x90,
        CEC_OPCODE_FEATURE_ABORT                 = 0x00,
        CEC_OPCODE_ABORT                         = 0xFF,
        CEC_OPCODE_GIVE_AUDIO_STATUS             = 0x71,
        CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0x7D,
        CEC_OPCODE_REPORT_AUDIO_STATUS           = 0x7A,
        CEC_OPCODE_SET_SYSTEM_AUDIO_MODE         = 0x72,
        CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST     = 0x70,
        CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS      = 0x7E,
        CEC_OPCODE_SET_AUDIO_RATE                = 0x9A,

        /* CEC 1.4 */
        CEC_OPCODE_START_ARC          = 0xC0,
        CEC_OPCODE_REPORT_ARC_STARTED = 0xC1,
        CEC_OPCODE_REPORT_ARC_ENDED   = 0xC2,
        CEC_OPCODE_REQUEST_ARC_START  = 0xC3,
        CEC_OPCODE_REQUEST_ARC_END    = 0xC4,
        CEC_OPCODE_END_ARC            = 0xC5,
        CEC_OPCODE_CDC                = 0xF8,
        /* when this opcode is set, no opcode will be sent to the device. this is one of the
         reserv*ed numbers */
        CEC_OPCODE_NONE = 0xFD
    };

    enum cec_version
    {
        CEC_VERSION_UNKNOWN = 0x00,
        CEC_VERSION_1_2     = 0x01,
        CEC_VERSION_1_2A    = 0x02,
        CEC_VERSION_1_3     = 0x03,
        CEC_VERSION_1_3A    = 0x04,
        CEC_VERSION_1_4     = 0x05
    };

    enum cec_logical_address
    {
        CEC_DEVICE_UNKNOWN          = -1, // not a valid logical address
        CEC_DEVICE_TV               = 0,
        CEC_DEVICE_RECORDINGDEVICE1 = 1,
        CEC_DEVICE_RECORDINGDEVICE2 = 2,
        CEC_DEVICE_TUNER1           = 3,
        CEC_DEVICE_PLAYBACKDEVICE1  = 4,
        CEC_DEVICE_AUDIOSYSTEM      = 5,
        CEC_DEVICE_TUNER2           = 6,
        CEC_DEVICE_TUNER3           = 7,
        CEC_DEVICE_PLAYBACKDEVICE2  = 8,
        CEC_DEVICE_RECORDINGDEVICE3 = 9,
        CEC_DEVICE_TUNER4           = 10,
        CEC_DEVICE_PLAYBACKDEVICE3  = 11,
        CEC_DEVICE_RESERVED1        = 12,
        CEC_DEVICE_RESERVED2        = 13,
        CEC_DEVICE_FREEUSE          = 14,
        CEC_DEVICE_UNREGISTERED     = 15,
        CEC_DEVICE_BROADCAST        = 15
    };

    enum cec_message_state
    {
        CEC_MESSAGE_STATE_UNKNOWN = 0,        /**< the initial state */
        CEC_MESSAGE_STATE_WAITING_TO_BE_SENT, /**< waiting in the send queue of the adapter, or timed out */
        CEC_MESSAGE_STATE_SENT,               /**< sent and waiting on an ACK */
        CEC_MESSAGE_STATE_SENT_NOT_ACKED,     /**< sent, but failed to ACK */
        CEC_MESSAGE_STATE_SENT_ACKED,         /**< sent, and ACK received */
        CEC_MESSAGE_STATE_INCOMING,           /**< received from another device */
        CEC_MESSAGE_STATE_ERROR               /**< an error occurred */
    };

    struct cec_datapacket
    {
        unsigned char data[CEC_MAX_DATA_PACKET_SIZE]; /**< the actual data */
        unsigned char size {};                        /**< the size of the data */
    };

    struct cec_command
    {
        cec_logical_address initiator {};   /**< the logical address of the initiator of this message */
        cec_logical_address destination {}; /**< the logical address of the destination of this message */
        cec_opcode opcode {};               /**< the opcode of this message */
        cec_datapacket parameters {};       /**< the parameters attached to this message */
        char opcode_set {};                 /**< 1 when an opcode is set, 0 otherwise (POLL message) */
    };

private:
    struct Data;
    std::unique_ptr<Data> m_p;

    static void hdmi_callback(int event, void *_data);

    void hdmi_init();

public:
    HDMI();
    ~HDMI();

    void hdmi_output_on();
    void hdmi_output_off();
    void send_cec_volume_up_to_tv();

    bool is_hdmi_connected() const;
};

}
