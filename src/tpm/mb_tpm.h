#pragma once

extern "C" {
#include <ca_defs.h>
#include "../cas/nagra/dpt/ca_dpt.h" 
#include "../cas/nagra/cak/ca_cak.h"
#include "../cas/nagra/cak/ca_sec.h"
#include "../cas/nagra/mb_nagra.h"
}

#undef min
#undef max


#include "../hal/mb_remote_control_keys.h"
#include "../hal/mb_diseqc.h"
#include <lvgl.h>

#include <tuple>
#include <iostream>

#include <map>
#include <vector>

namespace mb {

struct connection_context;
struct wait_remote_control_context;
struct wait_get_system_info;

namespace tpm {

struct RcuKey
{
    std::string name;
    Remote_Control_Key code;
};


static constexpr auto TPM_OK = 200;
static constexpr auto TPM_ERROR = 404;

typedef int TPM_Status_Code;
typedef const char *Body;
typedef int Body_Size;

typedef std::tuple<TPM_Status_Code, Body, Body_Size> Basic_TPM_Response;

Basic_TPM_Response system_info(connection_context *ctx);
Basic_TPM_Response system_reboot(connection_context *ctx);
Basic_TPM_Response system_power_off(connection_context *ctx);
Basic_TPM_Response update_hwcn(connection_context *ctx);
Basic_TPM_Response update_caid(connection_context *ctx);
Basic_TPM_Response update_otp_fuse(connection_context *ctx);
Basic_TPM_Response update_bootloader(connection_context *ctx);
Basic_TPM_Response copy_bootloader(connection_context *ctx);
Basic_TPM_Response burn_bootloader(connection_context *ctx);
Basic_TPM_Response format_data_partition(connection_context *ctx);
Basic_TPM_Response update_tpm_version(connection_context *ctx);
Basic_TPM_Response get_channel_list(connection_context *ctx);
Basic_TPM_Response post_channel_list(connection_context *ctx);
Basic_TPM_Response display_message(connection_context *ctx);
Basic_TPM_Response erase_license(connection_context *ctx);
Basic_TPM_Response system_license(connection_context *ctx);
Basic_TPM_Response system_update(connection_context *ctx);
Basic_TPM_Response get_encrypted_file(connection_context *ctx);
Basic_TPM_Response post_encrypted_files(connection_context *ctx);
Basic_TPM_Response get_led_list();
Basic_TPM_Response set_led_value(connection_context *ctx, std::string url);
Basic_TPM_Response get_led_value(std::string url);
Basic_TPM_Response get_rc_key_list(connection_context *ctx);
Basic_TPM_Response get_last_key_pressed(wait_remote_control_context *ctx);
Basic_TPM_Response get_tuner_list(connection_context *ctx);
Basic_TPM_Response get_tuner_info(connection_context *ctx, std::string url);
Basic_TPM_Response set_tuner_info(connection_context *ctx, std::string url);
Basic_TPM_Response set_tuner_state(connection_context *ctx, std::string url);
Basic_TPM_Response set_tuner_unlock(connection_context *ctx);
Basic_TPM_Response get_diseqc_info(connection_context *ctx);
Basic_TPM_Response set_diseqc_info(connection_context *ctx);
Basic_TPM_Response get_button_list(connection_context *ctx);
Basic_TPM_Response get_pressed_button(connection_context *ctx);
Basic_TPM_Response update_hdcp_key(connection_context *ctx);
Basic_TPM_Response update_hdcp_decrypt_key(connection_context *ctx);
Basic_TPM_Response update_cas_vendor_data(connection_context *ctx);
Basic_TPM_Response update_cas_provider_data(connection_context *ctx);
Basic_TPM_Response get_nsc_data(connection_context *ctx);
Basic_TPM_Response update_license(connection_context *ctx);
Basic_TPM_Response get_pasl_status(connection_context *ctx);
Basic_TPM_Response update_mptool_license(connection_context *ctx);
Basic_TPM_Response get_mptool_license(connection_context *ctx);
Basic_TPM_Response get_mptool_config(connection_context *ctx);
Basic_TPM_Response get_mptool_logs(connection_context *ctx);
Basic_TPM_Response update_nagra_csc(connection_context *ctx);
Basic_TPM_Response update_nagra_pairing_key(connection_context *ctx);
Basic_TPM_Response nagra_report(connection_context *ctx);
Basic_TPM_Response update_nagra_secret_key(connection_context *ctx);
Basic_TPM_Response default_route(connection_context *ctx);
Basic_TPM_Response get_usb_device_list(connection_context *ctx);
Basic_TPM_Response store_usb_file(connection_context *ctx);
Basic_TPM_Response read_usb_file(connection_context *ctx);
Basic_TPM_Response get_usb_file_list(connection_context *ctx);
Basic_TPM_Response usb_list_file(connection_context *ctx, std::string url);
Basic_TPM_Response usb_write_file(connection_context *ctx, std::string url);
Basic_TPM_Response usb_read_file(connection_context *ctx, std::string url);
Basic_TPM_Response usb_delete_file(connection_context *ctx, std::string url);
Basic_TPM_Response update_nagra_fpk(connection_context *ctx);

bool is_there_flashdisk();
std::string file_find(std::string path, std::string filter);
std::string format_partition_by_name(const char *partition_name, const char *folder_name);
bool update_otp(const char *casn, const char *marketi, TUnsignedInt16 size, const char *PvValue);
bool update_FPK(char *path, char *fpk);
bool update_CSC(char *path, TUnsignedInt8 *csc, TUnsignedInt16 csc_len);
bool update_PK(char *path, TUnsignedInt8 *clearPK, TUnsignedInt16 pk_len);
void convert_String2Hex(const char *src, TUnsignedInt8 *out, TUnsignedInt16 len);

std::string get_path_by_device_id(std::string url);
std::map<std::string, std::string> get_device_map();
std::string usb_get_devices_by_id(std::string id);
std::vector<std::string> split_by_char(const std::string &str, char delimiter);
std::string read_file_by_name(std::string path, std::string filter);
std::tuple<std::string, int> exec_system_command(std::string command);
std::tuple<std::array<unsigned char, 4>, std::string, std::string> get_casn();
std::tuple<std::array<unsigned char, 4>, std::string> get_nuid();
std::tuple<std::array<unsigned char, 4>, std::string> get_nuid_check_number();
int get_cscd_check_number(std::array<unsigned char, 16> cscd, std::array<unsigned char, 4> &cscd_check_number);

void create_background(void);
void clear_background(void);
void restore_background(void);
std::map<std::string, std::string> get_tpm_json();


}

}

