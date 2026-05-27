#include "mb_task_http_server.h"
#include "mb_tpm.h"

#include "../../project_version.h"

#include "common/mb_globals.h"
#include "common/mb_hash.h"

#include "hal/mb_gpio.h"
#include "hal/mb_lnb_config.h"
#include "hal/mb_system.h"
#include "hal/mb_tuner.h"

#include "mb_connection_context.h"
#include "mb_events.h"
#include "mb_http_utils.h"
#include "tpm_api.h"

#include <arpa/inet.h>
#include <aui_gpio.h>
#include <chrono>
#include <common/mb_hash.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system_error>
#include <thread>

namespace fs = std::filesystem;

extern "C" {
#include <ca_dpt.h>

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

extern const char *GIT_VERSION;
char rootfs_img_path[] = "/mnt/bootloader";

#define MAX_HEX_PK_SIZE 1030
#define MAX_TEXT_PK_SIZE (MAX_HEX_PK_SIZE*2 + 1)
#define MAX_HEX_CSCD_SIZE 3000
#define MAX_TEXT_CSCD_SIZE (MAX_HEX_CSCD_SIZE*2 + 1)

#define PK_START_ADDR 0x9000
#define CSCD_START_ADDR 0x8000
#define FPK_START_ADDR 0x6F0

lv_obj_t *m_main            { nullptr };
lv_obj_t *m_title_box       { nullptr };
lv_obj_t *m_title_text      { nullptr };
lv_obj_t *m_details_box     { nullptr };
lv_obj_t *m_details_text    { nullptr };

lv_font_t *font_40_semi = (lv_font_t *) &lv_font_segoeui_semi_bold_40_4bpp;
const auto title = "TESTE TPM\nMIDIABOX B8";

uint8_t CA_SN[4] = {0};
char CA_SN_STR[9] = "00000000";

// Constante com lista de arquivos e partições que devem estar no pendrive.
const std::vector<std::pair<std::string, std::string >> file_partition_list =
{
    //std::make_pair("bootenv.bin", "/dev/mtd3"),
    std::make_pair("boot_total_area.abs", "/dev/mtd0"),
    //std::make_pair("checksum.sha256", ""),
    //std::make_pair("db2.bin", "/dev/mtd11"),
    //std::make_pair("db2.bin", "/dev/mtd12"),
    //std::make_pair("db3.bin", "/dev/mtd13"),
    //std::make_pair("db3.bin", "/dev/mtd14"),
    std::make_pair("dtb.ubo", "/dev/mtd2"),
    //std::make_pair("logo.ubo", "/dev/mtd5"),
    std::make_pair("main.ubo", "/dev/mtd8"),
    std::make_pair("rootfs.squashfs", "/dev/mtd16"),
    std::make_pair("roothash.ubo", "/dev/mtd15"),
    std::make_pair("see.ubo", "/dev/mtd7"),
    std::make_pair("see_bl.ubo", "/dev/mtd6"),
    std::make_pair("uboot.ubo", "/dev/mtd1"),
    std::make_pair("uboot.ubo", "/dev/mtd9"),
    //std::make_pair("upg_onepackage.ubo", "/dev/mtd17"),
};

namespace mb {

namespace tpm {

namespace {

#define BASIC_RESPONSE_TMPL R"json({"message":"%s"})json"
#define ERROR_RESPONSE_TMPL R"json({"error":"%s","description":"%s"})json"

GPIO gpio_button_power(GPIO_NUM::POWER_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chup(GPIO_NUM::CHUP_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chdown(GPIO_NUM::CHDOWN_BUTTON, AUI_GPIO_I_DIR);

Basic_TPM_Response json_print(TPM_Status_Code _status_code, const cJSON *item)
{
    auto result = cJSON_Print(item);
    return {_status_code, result, strlen(result)};
}

}

std::vector<std::string> split_by_char(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;

    while(std::getline(ss, token, delimiter))
    {
        result.push_back(token);
    }

    return result;
}

Basic_TPM_Response system_info(connection_context *ctx)
{
    (void)ctx;
    auto[casn, casn_str, casn_str_dec] = get_casn();
    auto[nuid, nuid_str] = get_nuid();
    std::string hwcn;
    {
        std::string file_name = "/usr/mnt_app/mbgui/hwcn.txt";
        std::ifstream file(file_name);

        if(file.is_open())
        {
            std::getline(file, hwcn);
            file.close();
        }
        else
        {
            hwcn = "0000000000000000";
        }
    }
    std::string manufacture_date;
    {
        std::string file_name = "/usr/mnt_app/mbgui/manufacture_date.txt";
        std::ifstream file(file_name);

        if(file.is_open())
        {
            std::getline(file, manufacture_date);
            file.close();
        }
        else
        {
            manufacture_date = "00/00/0000";
        }
    }
    std::string hdcp_key = "00000000";
    {
        std::ifstream hdcp_file(HDCP_KEY_SAVE_PATH, std::ios::binary);

        if(hdcp_file.is_open())
        {
            std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(hdcp_file), {});
            hdcp_file.close();
            std::stringstream ss;

            for(auto byte : buffer)
            {
                ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }

            hdcp_key = ss.str();
        }
    }
    // @todo - verificar se caid está individualizado
    return memsnprintf(TPM_OK,
                       R"json({
    "hwcn":"%s",
    "nuid":"%s",
    "tpm":"%s",
    "rest_sw_version":"%s",
    "hdcp_key":"%s",
    "model":"MidiaBox B8",
    "ca_id":"%s",
    "indiv_flag":"%s"
})json",
                       hwcn.c_str(),
                       nuid_str.c_str(),
                       manufacture_date.c_str(),
                       PROJECT_TPM_VERSION.c_str(),
                       hdcp_key.c_str(),
                       casn_str_dec.c_str(),
                       "false"          // @todo verificar se placa está individualizada
                      );
}

Basic_TPM_Response get_last_key_pressed(wait_remote_control_context *ctx)
{
    (void)ctx;
    std::stringstream ss;

    if(Task_HTTP_Server::s_last_key_pressed.has_value())
    {
        auto _key = Task_HTTP_Server::s_last_key_pressed.value();
        Task_HTTP_Server::s_last_key_pressed = {};
        // Converte tecla para hexadecimal
        ss << std::hex << "\"0x" << static_cast<int>(_key) << '"';
    }
    else
    {
        ss << "null";
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, ss.str().c_str());
}

Basic_TPM_Response update_hwcn(connection_context *ctx)
{
    GET_JSON_VALUE(value);
    // String tem que ser de 16 bytes hexadecimal
    std::string hwcn = value.first.value();

    if(hwcn.size() > 32)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "HWCN é um campo de até 16 bytes");
    }

    std::string file_name = "hwcn.txt";
    std::string path_name = "/usr/mnt_app/mbgui";
    // Verify if folder exists, if it doesn't create it mkdir -p
    std::string command = "mkdir -p " + path_name;
    auto [msg, err] = exec_system_command(command.c_str());

    if(err != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "Falha ao criar pasta para salvar HWCN");
    }

    // Salvar variável phase em arquivo
    {
        command = "echo " + value.first.value() + " > " + path_name + "/" + file_name;
        auto [msg, err] = exec_system_command(command.c_str());

        if(err != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "Falha ao salvar HWCN no arquivo");
        }
    }
    // Em caso de sucesso, ecoar hwcn
    return memsnprintf(TPM_OK, R"json({"value":"%s"})json", hwcn.c_str());
}

Basic_TPM_Response update_caid(connection_context *ctx)
{
    GET_JSON_VALUE(value);
    // @todo - implementar escrita de caid''
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Partição de dados não acessível");
    }
    return memsnprintf(TPM_OK, R"json({"value":"%s"})json", value.first.value().c_str());
}

Basic_TPM_Response system_reboot(connection_context *ctx)
{
    (void)ctx;

    if(system("umount -a -t jffs2") != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "unmount failed");
    }

    if(system("(sleep 1 ; reboot) &") != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "reboot failed");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Receptor reiniciando...");
}

Basic_TPM_Response update_otp_fuse(connection_context *ctx)
{
    (void)ctx;
    auto size = 4;
    auto value = "00000000";
    const char *marketId = "0000028D";
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    csdInitialize(&xInitParameters);
    // 0000028D - valor fixo para o midiabox B8
    auto result = update_otp(CA_SN_STR, marketId, size, value);
    csdTerminate(&xTermParameters);

    if(result)
    {
        return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Success, update_otp_fuse done!");
    }

    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "update_otp_fuse not done");
}

Basic_TPM_Response update_bootloader(connection_context *ctx)
{
    GET_JSON_VALUE(usb_id);
    GET_JSON_VALUE(files_path);
    // @todo - implementar escrita nos fusíveis
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "update_bootloader não implementada");
    }
    return memsnprintf(TPM_OK, R"json({"size":"%s","value":"%s"})json", usb_id.first.value().c_str(), files_path.first.value().c_str());
}

Basic_TPM_Response format_data_partition(connection_context *ctx)
{
    (void)ctx;
    // Format partition /usr/mnt_vfs
    std::string res = format_partition_by_name("userfs2", "/usr/mnt_vfs");

    if(res != "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "304", res.c_str());
    }

    // Format partition /usr/mnt_app
    res = format_partition_by_name("userfs1", "/usr/mnt_app");

    if(res != "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "304", res.c_str());
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Formatadas partições /usr/mnt_app e /usr/mnt_vfs");
}

Basic_TPM_Response update_tpm_version(connection_context *ctx)
{
    GET_JSON_VALUE(phase);
    std::string file_name = "manufacture_date.txt";
    std::string path_name = "/usr/mnt_app/mbgui";
    // Verify if folder exists, if it doesn't create it
    auto path = fs::path(path_name);

    if(!fs::exists(path))
    {
        fs::create_directory(path);
    }

    // Salvar variável phase em arquivo
    auto p = fs::path(path_name + "/" + file_name);
    std::ofstream file(p);
    file << phase.first.value();
    file.close();
    auto tmpl = R"json({"phase":"%s"})json";
    return memsnprintf(TPM_OK, tmpl, phase.first.value().c_str());
}

Basic_TPM_Response display_message(connection_context *ctx)
{
    (void)ctx;
    // **** BLOCO USADO APENAS PARA TESTES ****
    auto res = get_tpm_json();

    if(res.find("error") != res.end())
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", res["error"].c_str());
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "OK");
}

Basic_TPM_Response copy_bootloader(connection_context *ctx)
{
    GET_JSON_VALUE(usb_id);
    GET_JSON_VALUE(files_path);

    if(!is_there_flashdisk())
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "404", "Pendrive não encontrado");
    }

    // Check if folder exists. If it doesn't exist, create it
    std::string destination_folder = rootfs_img_path;
    {
        std::string command = "mkdir -p " + destination_folder;
        auto [msg, err] = exec_system_command(command.c_str());

        if(err != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Falha ao criar pasta de bootloader");
        }
    }
    {
        std::string command = "rm -rf " + destination_folder + "/*";
        auto [msg, err] = exec_system_command(command.c_str());

        if(err != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Falha ao apagar pasta destino");
        }
    }
    // Get USB path
    DEBUG_MSG(COMMON, DEBUG, "usb_id = " << usb_id.first.value() << "\n");
    auto usb_path = usb_get_devices_by_id(usb_id.first.value());

    if(usb_path == "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "404", "Pendrive com este ID não encontrado");
    }

    DEBUG_MSG(COMMON, DEBUG, "usb_path = " << usb_path << "\n");
    // Copy all files from files_path to dest_path /mnt/usb/sda1
    std::string origin_path = usb_path + "/" + files_path.first.value();
    {
        std::string command = "cp -r " + origin_path + "/* " + destination_folder;
        auto [msg, err] = exec_system_command(command.c_str());

        if(err != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "301", "Diretório origem não encontrada ou erro ao copiar arquivos");
        }
    }
    usleep(100 * 1000); // Atraso para finalizar operação de cópia
    // Carrega o arquivo sha256_results.txt, retornar em caso de erro
    {
        std::string command = "cd " + destination_folder + " && sha256sum -c checksum.sha256";
        auto [msg, err] = exec_system_command(command.c_str());

        if(err != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "305", "Erro de integridade dos arquivos do bootloader");
        }
    }
    // Listar arquivos copiados
    std::string file_list_str;
    {
        std::string command = "ls " + destination_folder;
        auto [msg, err] = exec_system_command(command.c_str());
        file_list_str = msg;
    }
    std::replace(file_list_str.begin(), file_list_str.end(), '\n', ' ');
    auto tmpl = R"json({"message":"%s","arquivos copiados":"%s"})json";
    return memsnprintf(TPM_OK, tmpl, destination_folder.c_str(), file_list_str.c_str());
}

Basic_TPM_Response burn_bootloader(connection_context *ctx)
{
    (void)ctx;
    // Set DB3 flag
    exec_system_command("fw_setenv db3_flag 0");
    exec_system_command("fw_setenv upg_flag 0");
    // Check if folder exits
    std::string bootloader_path = rootfs_img_path;
    auto path = fs::path(bootloader_path);

    if(!fs::exists(path))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "300", "Pasta de bootloader não encontrado");
    }

    // Carrega lista de arquivos a serem gravados
    for(auto &file : file_partition_list)
    {
        DEBUG_MSG(COMMON, DEBUG, "File: " << file.first << " - " << file.second << "\n");

        // Verificar se arquivo existe
        if(!fs::exists(bootloader_path + "/" + file.first))
        {
            std::string error = "Erro: Arquivo " + file.first + " não encontrado";
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "301", error.c_str());
        }
    }

    // Gravação do bootloader
    for(auto &file : file_partition_list)
    {
        auto name = file.first;
        auto partition = file.second;
        DEBUG_MSG(COMMON, DEBUG, "File: " << name << " - " << partition << "\n");

        // Verificar se arquivo existe
        if(!fs::exists(bootloader_path + "/" + name))
        {
            std::string error = "Erro: Arquivo " + name + " não encontrado";
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "301", error.c_str());
        }

        auto command = "flashcp -v " + bootloader_path + "/" + name + " " + partition;
        DEBUG_MSG(COMMON, DEBUG, "command: " << command << "\n");

        if(system(command.c_str()) != 0)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "Erro de escrita");
        }
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Bootloader gravado com sucesso");
}

Basic_TPM_Response erase_license(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "todo, apagar licensa");
}

Basic_TPM_Response system_update(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "todo, system_update");
}

Basic_TPM_Response system_power_off(connection_context *ctx)
{
    (void)ctx;

    if(system("umount -a -t jffs2") != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "unmount failed");
    }

    if(system("(cat /dev/zero > /dev/fb0 ; sleep 1 ; poweroff) &") != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "poweroff failed");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Receptor desligado");
}

Basic_TPM_Response get_encrypted_file(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response post_encrypted_files(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_led_list()
{
    auto result = cJSON_CreateObject();
    auto leds = cJSON_AddArrayToObject(result, "ledList");
    // Led vermelho
    auto item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "gpio", static_cast<int>(GPIO_NUM::LED_RED));
    cJSON_AddStringToObject(item, "name", "Led Vermelho");
    cJSON_AddItemToArray(leds, item);
    // Led verde
    item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "gpio", static_cast<int>(GPIO_NUM::LED_GREEN));
    cJSON_AddStringToObject(item, "name", "Led Verde");
    // Resultado final com a lista
    cJSON_AddItemToArray(leds, item);
    return json_print(TPM_OK, result);
}

Basic_TPM_Response set_led_value(connection_context *ctx, std::string url)
{
    // Define o pino
    int pin;
    auto args = split_by_char(url, '/');
    std::stringstream(args.back()) >> pin;

    if(pin != static_cast<int>(GPIO_NUM::LED_GREEN) && pin != static_cast<int>(GPIO_NUM::LED_RED))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "400", "Pino inválido");
    }

    GET_JSON_INT_VALUE(state);
    std::string level = std::to_string(state.first.value());
    std::string led = std::to_string(static_cast<int>(pin));
    std::string command = "echo " + level + " > /sys/class/gpio/gpio" + led + "/value";
    auto [msg, err] = exec_system_command(command.c_str());

    if(err != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro ao alterar estado do led");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Alterado estado do led");
}

Basic_TPM_Response get_led_value(std::string url)
{
    // Define o pino
    int pin;
    auto args = split_by_char(url, '/');
    std::stringstream(args.back()) >> pin;

    // Verifica se o pino é válido
    if(pin != static_cast<int>(GPIO_NUM::LED_GREEN) && pin != static_cast<int>(GPIO_NUM::LED_RED))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "200", "Pino inválido");
    }

    std::string led = std::to_string(static_cast<int>(pin));
    std::string cmd = "/sys/class/gpio/gpio" + led + "/value";
    auto value = atoi(cat(cmd.c_str()).c_str());
    auto message = value == 1 ? "Led ligado" : "Led desligado";
    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, message);
}

Basic_TPM_Response get_rc_key_list(connection_context *ctx)
{
    (void)ctx;
    std::vector<RcuKey> RcuKeyList =
    {
        {"KEY_1", Remote_Control_Key::KEY_1},
        {"KEY_2", Remote_Control_Key::KEY_2},
        {"KEY_3", Remote_Control_Key::KEY_3},
        {"KEY_4", Remote_Control_Key::KEY_4},
        {"KEY_5", Remote_Control_Key::KEY_5},
        {"KEY_6", Remote_Control_Key::KEY_6},
        {"KEY_7", Remote_Control_Key::KEY_7},
        {"KEY_8", Remote_Control_Key::KEY_8},
        {"KEY_9", Remote_Control_Key::KEY_9},
        {"KEY_0", Remote_Control_Key::KEY_0},
        {"KEY_POWER", Remote_Control_Key::KEY_POWER},
        {"KEY_SLEEP", Remote_Control_Key::KEY_SLEEP},
        {"KEY_TVRADIO", Remote_Control_Key::KEY_TVRADIO},
        {"KEY_OK", Remote_Control_Key::KEY_OK},
        {"KEY_CHDOWN", Remote_Control_Key::KEY_CHDOWN},
        {"KEY_CHUP", Remote_Control_Key::KEY_CHUP},
        {"KEY_VOLDOWN", Remote_Control_Key::KEY_VOLDOWN},
        {"KEY_VOLUP", Remote_Control_Key::KEY_VOLUP},
        {"KEY_VOLTAR", Remote_Control_Key::KEY_VOLTAR},
        {"KEY_MENU", Remote_Control_Key::KEY_MENU},
        {"KEY_INFO", Remote_Control_Key::KEY_INFO},
        {"KEY_PLUS", Remote_Control_Key::KEY_PLUS},
        {"KEY_LR", Remote_Control_Key::KEY_LR},
        {"KEY_CC", Remote_Control_Key::KEY_CC},
        {"KEY_LAST", Remote_Control_Key::KEY_LAST},
        {"KEY_MUTE", Remote_Control_Key::KEY_MUTE}
    };
    auto json = cJSON_CreateObject();
    auto devices = cJSON_AddArrayToObject(json, "keyList");

    for(auto &key : RcuKeyList)
    {
        auto device = cJSON_CreateObject();
        cJSON_AddStringToObject(device, "key_name", key.name.c_str());
        std::stringstream ss;
        ss << std::hex << static_cast<int>(key.code);
        std::string hexString = "0x" + ss.str();
        cJSON_AddStringToObject(device, "key_code", hexString.c_str());
        cJSON_AddItemToArray(devices, device);
    }

    return json_print(TPM_OK, json);
}

Basic_TPM_Response get_tuner_list(connection_context *ctx)
{
    (void)ctx;
    auto json = cJSON_CreateObject();
    auto tuners = cJSON_AddArrayToObject(json, "tunerlist");
    auto tuner = cJSON_CreateObject();
    cJSON_AddNumberToObject(tuner, "tuner_id", 0);
    cJSON_AddStringToObject(tuner, "type", "DVB-S2x");
    cJSON_AddBoolToObject(tuner, "is_locked", true);
    cJSON_AddItemToArray(tuners, tuner);
    return json_print(TPM_OK, json);
}

Basic_TPM_Response get_tuner_info(connection_context *ctx, std::string url)
{
    (void)ctx;
    // Separa a url
    auto args = split_by_char(url, '/');

    if(args.size() == 1)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "102", "tuner_id não especificado");
    }

    int tuner_id;
    std::stringstream(args.back()) >> tuner_id;

    if(tuner_id != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "tuner_id não especificado");
    }

    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response set_tuner_state(connection_context *ctx, std::string url)
{
    (void)ctx;
    (void)url;
    GET_JSON_INT_VALUE(lnbf_voltage);

    if(lnbf_voltage.first.value() != 13 && lnbf_voltage.first.value() != 18)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Somente 0 e 1 são aceitos para lnbf_voltage");
    }

    GET_JSON_INT_VALUE(tone);

    if(tone.first.value() != 0 && tone.first.value() != 1)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Somente 0 e 1 são aceitos para tone");
    }

    if(not tpm_set_lnb_power(lnbf_voltage.first.value()))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro ao configurar LNB");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "LNBF ligado!");
}

Basic_TPM_Response set_tuner_info(connection_context *ctx, std::string url)
{
    (void)url;
    char devnum[8] = "0";
    char nimtype[8] = "0";
    char freq[8] = "11940";
    char symbolrate[8] = "29892";
    char videotype[8] = "";
    char audiotype[8] = "";
    char pol[8] = "H";
    char lnbftype[8] = "1";
    char stoptype[8] = "0";
    GET_JSON_INT_VALUE(frequency);
    GET_JSON_INT_VALUE(sat_symbol_rate);
    GET_JSON_INT_VALUE(sat_polarity);
    GET_JSON_INT_VALUE(sat_band_type);
    GET_JSON_INT_VALUE(sat_lnbf_type);
    GET_JSON_INT_VALUE(sat_diseqc_type);
    // Parse the JSON string
    auto upload = dynamic_cast<upload_data_context *>(ctx);
    auto json = cJSON_Parse(upload->upload_data.c_str());

    if(json == nullptr)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Erro ao analisar JSON");
    }

    auto data = cJSON_GetObjectItemCaseSensitive(json, "stream_params");

    if(data == nullptr)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "stream_params não encontrado");
    }

    char videopid [8] = "";
    {
        auto json = cJSON_GetObjectItemCaseSensitive(data, "video_pid");

        if(json == nullptr)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "video_pid não encontrado");
        }

        snprintf(videopid, sizeof(videopid), "%d", json->valueint);
    }
    char audiopid[8] = "";
    {
        auto json = cJSON_GetObjectItemCaseSensitive(data, "audio_pid");

        if(json == nullptr)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "audio_pid não encontrado");
        }

        snprintf(audiopid, sizeof(audiopid), "%d", json->valueint);
    }
    char pcrpid[8] = "";
    {
        auto json = cJSON_GetObjectItemCaseSensitive(data, "pcr_pid");

        if(json == nullptr)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "pcr_pid não encontrado");
        }

        snprintf(pcrpid, sizeof(pcrpid), "%d", json->valueint);
    }
    int video_type = 0;
    {
        auto json = cJSON_GetObjectItemCaseSensitive(data, "video_type");

        if(json == nullptr)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "video_type não encontrado");
        }

        video_type = json->valueint;
    }
    int audio_type = 0;
    {
        auto json = cJSON_GetObjectItemCaseSensitive(data, "audio_type");

        if(json == nullptr)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "audio_type não encontrado");
        }

        audio_type = json->valueint;
    }
    char *play_para[12] = {devnum, nimtype, freq, symbolrate, videopid, audiopid, videopid, videotype, audiotype, stoptype, pol, lnbftype};
    snprintf(freq, sizeof(freq), "%d", frequency.first.value());
    snprintf(symbolrate, sizeof(symbolrate), "%d", sat_symbol_rate.first.value());
    snprintf(lnbftype, sizeof(lnbftype), "%d", sat_lnbf_type.first.value());
    pol[0] = sat_polarity.first.value() == 0 ? 'H' : 'V';

    // sat_lnbf_type universal：0， multi: 1
    if(sat_lnbf_type.first.value() != 0 && sat_lnbf_type.first.value() != 1)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Somente 0 e 1 são aceitos para sat_lnbf_type");
    }

    // video_type：0: MPEG   1: AVC(H.264)  10: HEVC(H.265)
    if(video_type == 2)
    {
        strcpy(videotype, "0");
    }
    else if(video_type == 27)
    {
        strcpy(videotype, "1");
    }
    else if(video_type == 36)
    {
        strcpy(videotype, "10");
    }
    else
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Somente 2, 27 e 36 são aceitos para video_type");
    }

    // audio_type：1:  MPEG2, 2:  AAC-LATM,  3:  AC3,  4:  DTS,  5:  PPCM,  6:  LPCM-V,  7:  LPCM-A,  8:  PCM,  9:  WMA,  10: RA8,  11: MP3,  12: AAC-ADTS,  13: OGG,  14: EC3,  25: OPUS
    if(audio_type == 4)
    {
        strcpy(audiotype, "1");
    }
    else if(audio_type == 15)
    {
        strcpy(audiotype, "2");
    }
    else if(audio_type == 17)
    {
        strcpy(audiotype, "3");
    }
    else
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Somente 4, 15 e 17 são aceitos para audio_type");
    }

    DEBUG_MSG(COMMON, DEBUG, "freq = " << freq << "\n");
    DEBUG_MSG(COMMON, DEBUG, "symbolrate = " << symbolrate << "\n");
    DEBUG_MSG(COMMON, DEBUG, "videopid = " << videopid << "\n");
    DEBUG_MSG(COMMON, DEBUG, "audiopid = " << audiopid << "\n");
    DEBUG_MSG(COMMON, DEBUG, "pcrpid = " << pcrpid << "\n");
    static unsigned long para_num = 12;
    tpm_service_stop();

    if(tpm_service_play(para_num, play_para) != AUI_RTN_SUCCESS)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Transponder não sintonizado");
    }

    // Apaga tela de fundo para exibir imagem
    clear_background();
    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Success, play_service done.");
}

Basic_TPM_Response set_tuner_unlock(connection_context *ctx)
{
    (void)ctx;
    Task::post_event_player_stop();
    tpm_service_stop();
    // Restaura tela de fundo
    restore_background();
    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Tuner unlocked");
}

Basic_TPM_Response get_diseqc_info(connection_context *ctx)
{
    (void)ctx;
    DiseqC_Controller diseqc;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response set_diseqc_info(connection_context *ctx)
{
    (void)ctx;
    DiseqC_Controller diseqc;
    //
    //      Somente um comando será executado, o primeiro que for encontrado
    //
    GET_JSON_INT_VALUE(sat_diseqc_type);
    GET_JSON_VALUE(command);
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, comando não encontrado");
}

Basic_TPM_Response get_button_list(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Função não implementada");
}

Basic_TPM_Response get_pressed_button(connection_context *ctx)
{
    (void)ctx;
    std::stringstream ss;

    if(!gpio_button_chdown.read())
    {
        ss << std::hex << "\"0x" << static_cast<int>(Remote_Control_Key::KEY_CHDOWN) << '"';
    }
    else if(!gpio_button_chup.read())
    {
        ss << std::hex << "\"0x" << static_cast<int>(Remote_Control_Key::KEY_CHUP) << '"';
    }
    else if(!gpio_button_power.read())
    {
        ss << std::hex << "\"0x" << static_cast<int>(Remote_Control_Key::KEY_POWER) << '"';
    }
    else
    {
        ss << "null";
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, ss.str().c_str());
}

Basic_TPM_Response update_hdcp_decrypt_key(connection_context *ctx)
{
    char str_protectkey[16 * 2] = {0};
    TUnsignedInt8 protectkey[16] = {0};
    GET_JSON_VALUE(value);
    strncpy(str_protectkey, value.first.value().c_str(), 16 * 2);
    convert_String2Hex(str_protectkey, protectkey, 16);

    if(tpm_write_hdcp_decrypt_key(protectkey, 16) != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "update_dhcp_decrypt_key not done");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Sucess, update_dhcp_decrypt_key done");
}

Basic_TPM_Response update_hdcp_key(connection_context *ctx)
{
    GET_JSON_VALUE(value);

    if(value.first.value().size() != HDCP_ENC_KEY_LEN * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Tamanho errado da chave");
    }

    char str_hdcpkey[HDCP_ENC_KEY_LEN * 2 + 1] = {0};
    memset(str_hdcpkey, 0, HDCP_ENC_KEY_LEN * 2 + 1);
    TUnsignedInt8 hdcpkey[HDCP_ENC_KEY_LEN] = {0};
    strncpy(str_hdcpkey, value.first.value().c_str(), HDCP_ENC_KEY_LEN * 2);
    convert_String2Hex(str_hdcpkey, hdcpkey, HDCP_ENC_KEY_LEN);

    if(tpm_write_hdcp_key(hdcpkey, (strlen(str_hdcpkey) / 2)) != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "202", "Erro interno");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Sucess, update_hdcp_key done");
}

Basic_TPM_Response update_cas_vendor_data(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response update_cas_provider_data(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_nsc_data(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response update_license(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_pasl_status(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response update_mptool_license(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_mptool_license(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_mptool_config(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response get_mptool_logs(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "Erro, função não implementada");
}

Basic_TPM_Response update_nagra_csc(connection_context *ctx)
{
    (void)ctx;
    char str_csc[MAX_TEXT_CSCD_SIZE] = {0};
    TUnsignedInt8 csc[MAX_HEX_CSCD_SIZE] = {0};
    TUnsignedInt16 csc_len = 0;
    TUnsignedInt8 crc16_array[2] = {0};
    TUnsignedInt16 crc_16 = 0;
    GET_JSON_VALUE(length);
    GET_JSON_VALUE(NUID);
    GET_JSON_VALUE(version);
    GET_JSON_VALUE(Provider_ID);
    GET_JSON_VALUE(CSC_Data_body);
    GET_JSON_VALUE(CSCD_checknumber);
    GET_JSON_VALUE(CRC16);
    // Lê nuid da placa
    auto[nuid_num1, nuid_num1_str] = get_nuid();

    // Verifica se o NUID é igual ao NUID da placa
    if(strcasecmp(NUID.first.value().c_str(), nuid_num1_str.c_str()) != 0)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "NUID value not equal to nuid_num");
    }

    csc_len = strtol(length.first.value().c_str(), nullptr, 16);

    if(length.first.value().length() != 4 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "Error, length not equal to 4*2");
    }
    else if(strlen(NUID.first.value().c_str()) != 4 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "NUID not equal to 4*2");
    }
    else if(strlen(version.first.value().c_str()) != 2 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "version not equal to 2*2");
    }
    else if(strlen(Provider_ID.first.value().c_str()) != 2 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "Provider_ID not equal to 2*2");
    }
    else if(strlen(CSCD_checknumber.first.value().c_str()) != 4 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "CSCD_checknumber not equal to 4*2");
    }
    else if(strlen(CRC16.first.value().c_str()) != 2 * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "CRC16 not equal to 2*2");
    }
    else if(strlen(CSC_Data_body.first.value().c_str()) != (csc_len - 18) * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "CSC_Data_body not equal to (csc_len - 18)*2");
    }
    else if(strtol(length.first.value().c_str(), nullptr, 16) != csc_len)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "length value not equal to csc_len");
    }

    if(csc_len > 3000)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "csc length is not correct");
    }

    if(((csc_len - 18) < 0) || ((csc_len - 18) > 2982))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "csc length is not correct");
    }

    if(strlen(CSC_Data_body.first.value().c_str()) != (csc_len - 18) * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "csc body length is not correct");
    }

    memset(str_csc, 0, MAX_TEXT_CSCD_SIZE);
    memset(csc, 0, MAX_HEX_CSCD_SIZE);
    strncpy(str_csc, length.first.value().c_str(), 4 * 2);
    strncpy(str_csc + strlen(str_csc), NUID.first.value().c_str(), 4 * 2);
    strncpy(str_csc + strlen(str_csc), version.first.value().c_str(), 2 * 2);
    strncpy(str_csc + strlen(str_csc), Provider_ID.first.value().c_str(), 2 * 2);
    strncpy(str_csc + strlen(str_csc), CSC_Data_body.first.value().c_str(), (csc_len - 18) * 2);
    strncpy(str_csc + strlen(str_csc), CSCD_checknumber.first.value().c_str(), 4 * 2);
    strncpy(str_csc + strlen(str_csc), CRC16.first.value().c_str(), 2 * 2);

    if(strlen(str_csc) != csc_len * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "csc length is not correct");
    }

    crc_16 = strtol(CRC16.first.value().c_str(), nullptr, 16);
    convert_String2Hex(str_csc, csc, csc_len);

    if(dptCrc16Ccitt(csc, csc_len - 2, crc16_array) == 0)
    {
        if((crc16_array[0] << 8 | crc16_array[1]) != crc_16)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "crc16 is not correct");
        }
    }
    else
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "calculate crc16 failed");
    }

    if(update_CSC(rootfs_img_path, csc, csc_len) == true)
    {
        return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Success, update CSC done!");
    }

    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "update CSC not done");
}

Basic_TPM_Response update_nagra_pairing_key(connection_context *ctx)
{
    (void)ctx;
    char str_clearPK[MAX_TEXT_PK_SIZE] = {0};
    TUnsignedInt8 clearPK[MAX_HEX_PK_SIZE] = {0};
    TUnsignedInt8 crc16_array[2] = {0};
    TUnsignedInt16 crc_16 = 0;
    TUnsignedInt16 pk_len = 0;
    GET_JSON_VALUE(ssv_length);
    GET_JSON_VALUE(pk_length);
    GET_JSON_VALUE(stb_ca_sn);
    GET_JSON_VALUE(pk_body);
    GET_JSON_VALUE(crc16);

    if((strlen(ssv_length.first.value().c_str()) != 4 * 2) || (strlen(pk_length.first.value().c_str()) != 4 * 2) || (strlen(stb_ca_sn.first.value().c_str()) != 4 * 2) || (strlen(crc16.first.value().c_str()) != 2 * 2))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Campo inválido");
    }

    if(strlen(pk_body.first.value().c_str()) != (strtol(ssv_length.first.value().c_str(), nullptr, 16) - 8) * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Campo inválido");
    }

    pk_len = strtol(ssv_length.first.value().c_str(), nullptr, 16) + 6;

    if(((pk_len - 14) < 616) || ((pk_len - 14) > 1016))
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Campo inválido");
    }

    memset(str_clearPK, 0, MAX_TEXT_PK_SIZE);
    memset(clearPK, 0, MAX_HEX_PK_SIZE);
    strncpy(str_clearPK, ssv_length.first.value().c_str(), 8);
    strncpy(str_clearPK + strlen(str_clearPK), pk_length.first.value().c_str(), 8);
    strncpy(str_clearPK + strlen(str_clearPK), stb_ca_sn.first.value().c_str(), 8);
    strncpy(str_clearPK + strlen(str_clearPK), pk_body.first.value().c_str(), (pk_len - 14) * 2);
    strncpy(str_clearPK + strlen(str_clearPK), crc16.first.value().c_str(), 4);

    if(strlen(str_clearPK) != pk_len * 2)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Campo inválido");
    }

    crc_16 = strtol(crc16.first.value().c_str(), nullptr, 16);
    convert_String2Hex(str_clearPK, clearPK, pk_len);

    if(dptCrc16Ccitt(clearPK, pk_len - 2, crc16_array) == 0)
    {
        if((crc16_array[0] << 8 | crc16_array[1]) != crc_16)
        {
            return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "crc16 is not correct");
        }
    }
    else
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "calculate crc16 failed");
    }

    convert_String2Hex(stb_ca_sn.first.value().c_str(), CA_SN, 4);
    strncpy(CA_SN_STR, stb_ca_sn.first.value().c_str(), 8);

    if(update_PK(rootfs_img_path, clearPK, pk_len) == true)
    {
        return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Success, update PK done!");
    }

    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "update PK not done");
}

Basic_TPM_Response nagra_report(connection_context *ctx)
{
    (void)ctx;
    uint8_t chip_extension_str[21] = { 'c', 'h', 'i', 'p', 'e', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', '\0' };
    uint8_t chip_set_cut_str[21] = { 'c', 'h', 'i', 'p', 's', 'e', 't', 'c', 'u', 't', '\0' };
    auto[nuid_num, nuid_str] = get_nuid();
    auto[data, nuid_check_number_str] = get_nuid_check_number();
    auto[casn, casn_str, casn_str_dec] = get_casn();
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    csdInitialize(&xInitParameters);
    csdGetChipRevision(chip_set_cut_str);
    csdGetChipExtension(chip_extension_str);
    std::string file_name = "/mnt/csc.bin";
    std::ifstream file_csc(file_name);
    std::string csc_data_check_number;
    char csc_data_check_number_str[9] = {0, 0};
    uint8_t pxCscData1[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    if(file_csc.is_open())
    {
        std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file_csc)), std::istreambuf_iterator<char>());
        file_csc.close();
        memcpy(pxCscData1, file_data.data(), 16);

        if(csdGetCSCDCheckNumber(pxCscData1, data.data()) != CSD_NO_ERROR)
        {
            printf("csdGetCSCDCheckNumber error\n");
        }
        else
        {
            for(int i = 0; i < 4; i++)
            {
                sprintf(csc_data_check_number_str + i * 2, "%02X", data[i]);
                printf("%02X ", data[i]);
            }

            printf("\n");
            printf("csc_data_check_number_str = %s\n", csc_data_check_number_str);
        }

        printf("csdGetCSCDCheckNumber called\n");
    }
    else
    {
        printf("csc.bin file not found\n");
    }

    csdTerminate(&xTermParameters);
    printf("csdTerminate called\n");
    char csc_data_config_str[9] = {0};
    uint8_t pxCscData[MAX_HEX_CSCD_SIZE] = {0};
    std::string csc_data_config;
    std::ifstream file(file_name);

    if(file.is_open())
    {
        std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        memcpy(pxCscData, file_data.data(), MAX_HEX_CSCD_SIZE);

        if(dptGetCscDataConfiguration(pxCscData, data.data()) != 0)
        {
            printf("dptGetCscDataConfiguration error\n");
        }
        else
        {
            for(int i = 0; i < 4; i++)
            {
                sprintf(csc_data_config_str + i * 2, "%02X", data[i]);
            }
        }
    }

    uint8_t cert_report_check_number[8];
    dptGetCertCheckNumber(DPT_CERT_REPORT_CHECK_NUMBER, cert_report_check_number);
    char cert_report_check_number_str[17] = { 0 };

    for(int i = 0; i < 8; i++)
    {
        snprintf(cert_report_check_number_str + i * 2, 3, "%02X", cert_report_check_number[i]);
    }

    return memsnprintf(TPM_OK, R"json({"chipsetExtension":"%s",
    "chipsetCut":"%s",
    "nuid":"%s",
    "nuidCheckNumber":"%s",
    "stb_ca_sn":"%s",
    "cscDataConfig":"%s",
    "cscDataCheckNumber":"%s",
    "certReportCheckNumber":"%s"
})json",
                       chip_extension_str,
                       chip_set_cut_str,
                       nuid_str.c_str(),
                       nuid_check_number_str.c_str(),
                       casn_str.c_str(),
                       csc_data_config_str,
                       csc_data_check_number_str,
                       cert_report_check_number_str);
}

Basic_TPM_Response update_nagra_secret_key(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "-1", "Erro, função não implementada");
}

Basic_TPM_Response post_channel_list(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "-1", "Erro, função não implementada");
}

Basic_TPM_Response get_channel_list(connection_context *ctx)
{
    (void)ctx;
    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "-1", "Erro, função não implementada");
}

Basic_TPM_Response default_route(connection_context *ctx)
{
#if 0
    DEBUG_MSG(COMMON, DEBUG, dec << __LINE__ << " - " << __FUNCTION__ << "\n");
    // Bloco normal para rotas sem definição.
    // Comentar para efutar testes.
    {
        auto upload = dynamic_cast<upload_data_context *>(ctx);
        DEBUG_MSG(COMMON, DEBUG, upload->upload_data << "\n");
        auto tmpl =;
        return memsnprintf(TPM_ERROR, tmpl, "100", "Rota não implementada\n");
    }
    // Grava usando flashcp
    system("time flashcp -v /mnt/bootloader/upg_onepackage.ubo /dev/mtd17");
    system("sync");
    system("dd if=/dev/mtd17 bs=4096 count=100 | sha256sum");
    // Grava dados aleatórios usando dd
    system("time dd if=/dev/random of=/dev/mtd17");
    system("sync");
    system("time dd if=/dev/zero bs=4K count=1056 | tr '\000' '\377' | dd of=/dev/mtd17");
    system("time dd if=/mnt/bootloader/upg_onepackage.ubo of=/dev/mtd17");
    system("sync");
    system("dd if=/dev/mtd17 bs=4096 count=100 | sha256sum");
#endif
    return memsnprintf(TPM_OK, R"json({"tempo":"%s ms","comando":"%s"})json", "0", "Teste realizado");
}

Basic_TPM_Response usb_write_file(connection_context *ctx, std::string url)
{
    const char *message = "";
    const char *error = nullptr;

    do
    {
        auto path = get_path_by_device_id(url);

        if(path == "")
        {
            message = "101: dispositivo não encontrado";
            error = "101";
            break;
        }

        // Cria estrutura de diretórios caso seja necessário
        auto args = split_by_char(url, '/');

        for(unsigned long  i = 3; i < (args.size() - 1); i++)
        {
            // Check if the folder exists
            if(!std::filesystem::exists(path))
            {
                // Create the folder
                if(std::filesystem::create_directory(path))
                {
                    DEBUG_MSG(COMMON, DEBUG, "Folder created successfully.\n");
                }
                else
                {
                    message = "101: falha ao tentar criar diretório";
                    error = "101";
                    break;
                }
            }
            else
            {
                DEBUG_MSG(COMMON, DEBUG, "Folder already exists.\n");
            }
        }

        auto upload = dynamic_cast<upload_data_context *>(ctx);
        DEBUG_MSG(COMMON, DEBUG, upload->upload_data << "\n");
        std::string filePath = path + + "/" + args.back();
        DEBUG_MSG(COMMON, DEBUG, "Nome completo: " << filePath << "\n");

        // Check if the file exists
        if(!std::filesystem::exists(filePath))
        {
            // Create and open the file
            std::ofstream file(filePath);

            if(file.is_open())
            {
                // Write data to the file
                file << upload->upload_data;
                // Close the file
                file.close();
                message = "arquivo criado com sucesso";
                break;
            }
            else
            {
                message = "101: falha ao criar arquivo";
                error = "101";
                break;
            }
        }
        else
        {
            DEBUG_MSG(COMMON, DEBUG, "File already exists.\n");
            // Create and open the file
            std::ofstream file(filePath);

            if(file.is_open())
            {
                // Write data to the file
                file << upload->upload_data;
                // Close the file
                file.close();
            }

            message = "arquivo atualizado com sucesso";
        }
    }
    while(false);

    if(error)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, error, message);
    }
    else
    {
        return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, message);
    }
}

Basic_TPM_Response usb_read_file(connection_context *ctx, std::string url)
{
    (void)ctx;
    auto path = get_path_by_device_id(url);

    if(path == "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Dispositivo não encontrado");
    }

    auto args = split_by_char(url, '/');
    std::string filePath = path + + "/" + args.back();
    DEBUG_MSG(COMMON, DEBUG, "Ler arquivo " << filePath << "\n");
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string data = buffer.str();
    auto result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "name", data.c_str());
    return json_print(TPM_OK, result);
}

Basic_TPM_Response usb_delete_file(connection_context *ctx, std::string url)
{
    (void)ctx;
    auto path = get_path_by_device_id(url);

    if(path == "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Dispositivo não encontrado");
    }

    auto args = split_by_char(url, '/');
    std::string filePath = path + + "/" + args.back();
    DEBUG_MSG(COMMON, DEBUG, "Apagar arquivo " << filePath << "\n");

    if(fs::exists(filePath))
    {
        fs::remove(filePath);
    }
    else
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "101", "Erro : arquivo não encontrado");
    }

    return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Arquivo apagado com sucesso");
}

Basic_TPM_Response usb_list_file(connection_context *ctx, std::string url)
{
    (void)ctx;

    // Verifica se existe um pendrive conectado
    if(is_there_flashdisk() == false)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "400", "Pendrive não encontrado");
    }

    auto path = get_path_by_device_id(url);

    if(path == "")
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "500", "id não encontrado");
    }

    auto result = cJSON_CreateObject();
    auto list = cJSON_AddArrayToObject(result, "dirList");
    // Lista os arquivos de path
    auto dir = cJSON_CreateObject();
    cJSON_AddStringToObject(dir, "path", path.c_str());
    auto files = cJSON_AddArrayToObject(dir, "fileList");

    for(const auto &entry : fs::directory_iterator(path))
    {
        if(fs::is_regular_file(entry))
        {
            auto file = cJSON_CreateObject();
            cJSON_AddStringToObject(file, "name", entry.path().string().c_str());
            cJSON_AddNumberToObject(file, "size", fs::file_size(entry));
            cJSON_AddItemToArray(files, file);
        }
    }

    cJSON_AddItemToArray(list, dir);

    // Lista todos os diretórios e arquivos recursivamente

    for(const auto &entry : fs::recursive_directory_iterator(path))
    {
        if(fs::is_directory(entry))
        {
            auto dir = cJSON_CreateObject();
            cJSON_AddStringToObject(dir, "path", entry.path().string().c_str());
            auto files = cJSON_AddArrayToObject(dir, "fileList");

            for(const auto &entries : fs::directory_iterator(entry.path()))
            {
                if(fs::is_regular_file(entries))
                {
                    auto file = cJSON_CreateObject();
                    cJSON_AddStringToObject(file, "name", entries.path().string().c_str());
                    cJSON_AddNumberToObject(file, "size", fs::file_size(entries));
                    cJSON_AddItemToArray(files, file);
                }
            }

            cJSON_AddItemToArray(list, dir);
        }
    }

    return json_print(TPM_OK, result);
}

Basic_TPM_Response get_usb_device_list(connection_context *ctx)
{
    (void)ctx;

    // Verifica se existe um pendrive conectado
    if(is_there_flashdisk() == false)
    {
        return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "400", "Pendrive não encontrado");
    }

    // Separa os dispositivos
    auto devs = get_device_map();
    auto json = cJSON_CreateObject();
    auto devices = cJSON_AddArrayToObject(json, "usbDeviceList");

    for(auto &d : devs)
    {
        auto device = cJSON_CreateObject();
        cJSON_AddStringToObject(device, "usb_id", d.first.c_str());
        cJSON_AddStringToObject(device, "path", d.second.c_str());
        cJSON_AddItemToArray(devices, device);
        DEBUG_MSG(COMMON, DEBUG, "device: " << d.first << " - " << d.second << "\n");
    }

    return json_print(TPM_OK, json);
}

Basic_TPM_Response update_nagra_fpk(connection_context *ctx)
{
    (void)ctx;
    GET_JSON_VALUE(data);
    char str_fpk[16 * 2] = {0};
    strncpy(str_fpk, data.first.value().c_str(), 32);

    if(update_FPK(rootfs_img_path, str_fpk) == true)
    {
        return memsnprintf(TPM_OK, BASIC_RESPONSE_TMPL, "Success,update FPK done!");
    }

    return memsnprintf(TPM_ERROR, ERROR_RESPONSE_TMPL, "303", "fpk não gravado");
}

std::map<std::string, std::string> get_device_map()
{
    auto res = exec_system_command("blkid");
    auto text = std::get<0>(res);
    // Separa os dispositivos
    std::map<std::string, std::string> result;
    auto lines = split_by_char(text, '\n');

    for(auto &line : lines)
    {
        line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
        DEBUG_MSG(COMMON, DEBUG, "line: " << line << "\n");
        auto uuid = line.substr(line.find("UUID=") + 5, line.size() - 2);
        DEBUG_MSG(COMMON, DEBUG, "uuid: " << uuid << "\n");
        auto path = "/mnt/usb" + line.substr(line.find("/dev") + 4, line.find(":") - 4);
        DEBUG_MSG(COMMON, DEBUG, "path: " << path << "\n");
        result.insert(std::make_pair(uuid, path));
    }

    return result;
}

std::string get_path_by_device_id(std::string url)
{
    // Separa o id
    auto args = split_by_char(url, '/');
    std::string id = args[2];
    // Separa os dispositivos
    auto devs = get_device_map();
    return devs[id];
}

std::string usb_get_devices_by_id(std::string id)
{
    std::string text;
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("blkid", "r"), pclose);

    if(!pipe)
    {
        return "";
    }

    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        text += buffer.data();
    }

    auto lines = split_by_char(text, '\n');

    for(auto &line : lines)
    {
        DEBUG_MSG(COMMON, DEBUG, line << "\n");
        auto pos = line.find("UUID=");

        if(pos == std::string::npos)
        {
            DEBUG_MSG(COMMON, DEBUG, "UUIID not found\n");
            continue;
        }

        auto uuid = line.substr(pos + 5, line.find(" ", pos) - pos - 5);

        if(uuid.back() == '"')
        {
            uuid.pop_back();
        }

        if(uuid.front() == '"')
        {
            uuid.erase(0, 1);
        }

        DEBUG_MSG(COMMON, DEBUG, "uuid: " << uuid << "\n");

        if(uuid == id)
        {
            auto path = "/mnt/usb" + line.substr(line.find("/dev") + 4, line.find(":") - 4);
            DEBUG_MSG(COMMON, DEBUG, "path: " << path << "\n");
            return path;
        }
    }

    return "";
}

bool update_otp(const char *casn, const char *marketi, TUnsignedInt16 size, const char *PvValue)
{
    TCsdUnsignedInt8 *pxPvValue = nullptr;
    TCsd4BytesVector xStbCaSn;
    TCsd4BytesVector xMarketSegmentId;
    convert_String2Hex(casn, xStbCaSn, 4);
    convert_String2Hex(marketi, xMarketSegmentId, 4);
    DEBUG_MSG(COMMON, DEBUG, "size= " << size << " PvValue= '" << PvValue << "''\n");
    auto result = false;

    do
    {
        // casn, marketi, size, PvValue
        if(csdSetStbCaSn(xStbCaSn) != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdSetStbCaSn error\n");
            break;
        }

        // Product code, fixed for this project
        if(csdSetMarketSegmentId(xMarketSegmentId) != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdSetMarketSegmentId error\n");
            break;
        }

        pxPvValue = (TCsdUnsignedInt8 *)malloc(sizeof(TCsdUnsignedInt8) * size);

        if(pxPvValue == nullptr)
        {
            DEBUG_MSG(COMMON, DEBUG, "malloc error\n");
            break;
        }

        // SCS program version number - Secure Chipset Startup
        convert_String2Hex(PvValue, pxPvValue, size);

        if(csdSetScsPv(CSD_SCS_PV_VERSIONING_REF, nullptr, size, pxPvValue) != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdSetScsPv error\n");
            break;
        }

        if(csdEnableFlashProtection() != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdEnableFlashProtection error\n");
            break;
        }

        if(csdEnableScs() != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdEnableScs error\n");
            break;
        }

        if(csdSelectDebugInterfaceProtectionLevel(CSD_DEBUG_INTERFACE_ACCESS_MODE_PASSWORD_PROTECTED) != CSD_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "csdSelectDebugInterfaceProtectionLevel error\n");
            break;
        }

        result = true;
    }
    while(false);

    free(pxPvValue);
    return result;
}

bool update_PK(char *path, TUnsignedInt8 *clearPK, TUnsignedInt16 pk_len)
{
    TUnsignedInt8 en_csc[MAX_HEX_CSCD_SIZE] = {0};
    const TUnsignedInt8 *pxCscData = nullptr;
    TUnsignedInt8 *pxPairingData = nullptr;
    FILE *fp_pk = nullptr;
    FILE *fp_csc = nullptr;
    char cmd[256] = {0};
    sprintf(cmd, "dd if=/mnt/enc_pk.bin of=%s/boot_total_area.abs seek=%d obs=2048 oflag=seek_bytes conv=notrunc", path, PK_START_ADDR);
    pxCscData = en_csc;
    pxPairingData = clearPK;
    fp_csc = fopen("/mnt/csc.bin", "rb");

    if(fp_csc == nullptr)
    {
        return false;
    }

    memset(en_csc, 0, MAX_HEX_CSCD_SIZE);
    auto en_csc_read = fread(en_csc, 1, MAX_HEX_CSCD_SIZE, fp_csc);
    fclose(fp_csc);

    if(en_csc_read != MAX_HEX_CSCD_SIZE)
    {
        DEBUG_MSG(COMMON, DEBUG, "CSC Read failure: " << MAX_HEX_CSCD_SIZE << " " << strerror(errno) << "\n");
        return false;
    }

    if(dptCertEncryptPairingData(pxCscData, pxPairingData) == 0)
    {
        fp_pk = fopen("/mnt/enc_pk.bin", "wb+");

        if(fp_pk == nullptr)
        {
            return false;
        }

        // Ignore last 2 bytes, crc
        fwrite(clearPK, 1, pk_len - 2, fp_pk);
        fclose(fp_pk);
        usleep(100 * 1000);

        if(system(cmd) != 0)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool update_CSC(char *path, TUnsignedInt8 *csc, TUnsignedInt16 csc_len)
{
    FILE *fp_csc = nullptr;
    char cmd[256] = {0};
    sprintf(cmd, "dd if=/mnt/csc.bin of=%s/boot_total_area.abs seek=%d obs=4096 oflag=seek_bytes conv=notrunc", path, CSCD_START_ADDR);
    fp_csc = fopen("/mnt/csc.bin", "wb+");

    if(fp_csc == nullptr)
    {
        return false;
    }

    fwrite(csc, 1, MAX_HEX_CSCD_SIZE, fp_csc);
    fclose(fp_csc);
    usleep(100 * 1000);

    if(system(cmd) != 0)
    {
        return false;
    }

    return true;
}

bool update_FPK(char *path, char *fpk)
{
    bool ret = false;
    int i = 0;
    TUnsignedInt8 xSize = 16;
    FILE *fp_fpk = nullptr;
    char *ptr = fpk;
    char cmd[256] = {0};
    sprintf(cmd, "dd if=/mnt/enc_fpk.bin of=%s/boot_total_area.abs seek=%d obs=16 oflag=seek_bytes conv=notrunc", path, FPK_START_ADDR);
    TUnsignedInt8 fpk_input[16] = {0};
    TUnsignedInt8 fpk_output[16] = {0};
    memset(fpk_input, 0x00, 16);
    memset(fpk_output, 0x00, 16);

    do
    {
        convert_String2Hex(fpk, fpk_input, xSize);
        TSecFunctionTable *funtable = secGetFunctionTable();

        if(funtable == nullptr)
        {
            DEBUG_MSG(COMMON, DEBUG, "funtable is NULL\n");
            break;
        }

        if(funtable->secEncryptFlashProtKey(fpk_input, fpk_output, xSize) != SEC_NO_ERROR)
        {
            DEBUG_MSG(COMMON, DEBUG, "secEncryptFlashProtKey error\n");
            break;
        }

        DEBUG_MSG(COMMON, DEBUG, "enc fpk:\n" << hex);

        for(i = 0; i < xSize; i++)
        {
            DEBUG_MSG(COMMON, DEBUG, setw(2) << setfill('0') << fpk_output[i]);
        }

        DEBUG_MSG(COMMON, DEBUG, dec << "\n");
        fp_fpk = fopen("/mnt/enc_fpk.bin", "wb+");

        if(fp_fpk == nullptr)
        {
            DEBUG_MSG(COMMON, DEBUG, "fp_fpk fopen error\n");
            break;
        }

        fwrite(fpk_output, 1, xSize, fp_fpk);
        fclose(fp_fpk);
        usleep(100 * 1000);
        DEBUG_MSG(COMMON, DEBUG, "Executando: '" << cmd << "'\n");

        if(system(cmd) != 0)
        {
            DEBUG_MSG(COMMON, DEBUG, "system error\n");
            break;
        }

        ret = true;
    }
    while(false);

    return ret;
}

void convert_String2Hex(const char *src, TUnsignedInt8 *out, TUnsignedInt16 len)
{
    char *ptr = (char *)src;
    char tmp[4] = {0, 0, 0, 0};
    int i = 0;

    for(i = 0; i < len; i++)
    {
        strncpy(tmp, ptr, 2);
        out[i] = strtol(tmp, nullptr, 16);
        ptr += 2;
    }
}

bool is_there_flashdisk()
{
    try
    {
        for(const auto &entry : fs::recursive_directory_iterator("/mnt/usb"))
        {
            // Apenas verifica se existe um pendrive conectado
            break;
        }
    }
    catch(const std::filesystem::filesystem_error &ex)
    {
        std::cerr << "Error accessing directory: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

std::string file_find(std::string path, std::string filter)
{
    std::string result;

    for(const auto &entry : fs::recursive_directory_iterator(path))
    {
        if(fs::is_regular_file(entry) && entry.path().filename() == filter)
        {
            result = entry.path().string();
            break;
        }
    }

    return result;
}

std::string format_partition_by_name(const char *partition, const char *folder_name)
{
    // Faz o parse do número da partição
    std::string command = std::string("cat /proc/mtd | awk '/") + partition + std::string("/{ print $1 }'");
    auto res = exec_system_command(command);
    auto num_str = std::get<0>(res);
    num_str.erase(std::remove_if(num_str.begin(), num_str.end(), [](char c)
    {
        return !std::isdigit(c);
    }), num_str.end());
    DEBUG_MSG(COMMON, DEBUG, "Partition number: " << num_str << "\n");
    std::string folder_name_str = folder_name;
    std::string last_folder_name = folder_name_str.substr(folder_name_str.find_last_of("/") + 1);
    DEBUG_MSG(COMMON, DEBUG, "Last folder name: " << last_folder_name << "\n");
    // Verifica se a partição está montada, e se estiver, desmonta
    command = std::string("mount | grep ") + last_folder_name;
    res = exec_system_command(command);

    if(std::get<1>(res) == 0)
    {
        command = "umount " + std::string(folder_name) + " 2>&1";
        res = exec_system_command(command);

        if(std::get<1>(res) != 0)
        {
            return std::get<0>(res);
        }
    }

    // Apaga a partição
    command = "flash_eraseall -j /dev/mtd" + num_str + " 2>&1";
    res = exec_system_command(command);

    if(std::get<1>(res) != 0)
    {
        return std::get<0>(res);
    }

    // Monta a partição
    command = "mount -t jffs2 /dev/mtdblock" + num_str + " " + folder_name + " 2>&1";
    res = exec_system_command(command);

    if(std::get<1>(res) != 0)
    {
        return std::get<0>(res);
    }

    return "";
}

std::string read_file_by_name(std::string path, std::string filter)
{
    // Raiz onde todos os arquivos de vídeo serão buscados
    std::vector<std::string> files = {};
    std::string file;

    // Iterate over the std::filesystem::directory_entry elements using `auto`
    for(auto const &dir_entry : fs::recursive_directory_iterator(path))
    {
        files.push_back(dir_entry.path());
    }

    for(auto &f : files)
    {
        if(f.find(filter) != std::string::npos)
        {
            file = f;
            break;
        }
    }

    if(file.empty())
    {
        DEBUG_MSG(COMMON, DEBUG, "File not found\n");
    }

    return file;
}

std::tuple<std::string, int> exec_system_command(std::string command)
{
    DEBUG_MSG(COMMON, DEBUG, "Command: " << command << "\n");
    std::string res = "";
    char buff[1024];
    FILE *pipe = popen(command.c_str(), "r");

    while(fgets(buff, sizeof(buff), pipe) != nullptr)
    {
        res += buff;
    }

    auto result_code = pclose(pipe);
    DEBUG_MSG(COMMON, DEBUG, "Result: " << res << ", Code: " << result_code << "\n");
    return {res, result_code};
}

void restore_background()
{
    if(m_main != nullptr)
    {
        lv_obj_clear_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    }
}

void clear_background()
{
    if(m_main != nullptr)
    {
        lv_obj_add_flag(m_main, LV_OBJ_FLAG_HIDDEN);
    }
}

void init_net_config()
{
#if defined(MBGUI_APP_TPM)
    DEBUG_MSG(COMMON, DEBUG, "Versão de software TPM: " << PROJECT_TPM_VERSION << "\n");
    DEBUG_MSG(COMMON, DEBUG, "Data do commit: " << GIT_VERSION << "\n\n\n");
    auto res = tpm::get_tpm_json();

    if(res.find("error") != res.end())
    {
        DEBUG_MSG(COMMON, DEBUG, "Error: " << res["error"] << "\n");
        return;
    }

    std::string ip = res["ip_addr"];
    std::string netmask = res["network_mask"];
    std::string gw = res["logprod_ip_addr"];
    DEBUG_MSG(COMMON, DEBUG, "ip_addr = " << ip << endl);
    DEBUG_MSG(COMMON, DEBUG, "network_mask = " << netmask << endl);
    DEBUG_MSG(COMMON, DEBUG, "logprod_ip_addr = " << gw << endl);
#ifndef NDEBUG
    DEBUG_MSG(COMMON, DEBUG, "ip_addr = " << ip << endl);
    DEBUG_MSG(COMMON, DEBUG, "network_mask = " << netmask << endl);
    DEBUG_MSG(COMMON, DEBUG, "logprod_ip_addr = " << gw << endl);
    // Change ip_addr value
    char cmd[128];
    const char *nwkInf = "eth0"; // Network interfaceBasic_TPM_Response update_cas_vendor_data(connection_context *ctx);
    // Bring the interface down
    snprintf(cmd, sizeof(cmd), "ip link set %s down", nwkInf);
    tpm::exec_system_command(cmd);
    // Set the IP address and netmask
    snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", nwkInf, ip.c_str(), netmask.c_str());
    tpm::exec_system_command(cmd);
    // Set the gateway
    snprintf(cmd, sizeof(cmd), "route add default gw %s %s", gw.c_str(), nwkInf);
    tpm::exec_system_command(cmd);
    // Bring the interface up
    snprintf(cmd, sizeof(cmd), "ip link set %s up", nwkInf);
    tpm::exec_system_command(cmd);
#endif
#endif // MBGUI_APP_TPM
}

void create_background()
{
    auto text_color_int = lv_color_hex(0x00F0F0F0);
    auto bg_color_int = lv_color_hex(0x000F0F0F);
    auto bg_color_danger = lv_color_hex(0x00F00808);
    auto background = bg_color_int;
    std::string result;
    m_main = lv_obj_create(lv_scr_act());
    lv_obj_set_size(m_main, 1280, 720);
    lv_obj_set_style_bg_color(m_main, background, 0);
    m_title_box = create_rect(m_main, 0, 0, 1000, 200, background);
    lv_obj_align(m_title_box, LV_ALIGN_TOP_MID, 0, 100);
    m_title_text = set_label_text(m_title_box, title, 0, 0, font_40_semi, text_color_int);
    lv_obj_align(m_title_text, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_align(m_title_text, LV_TEXT_ALIGN_CENTER, 0);
    // Exibe o conteúdo do arquivo TPM.json
    m_details_box = create_rect(m_main, 0, 0, 600, 360, background);
    lv_obj_align(m_details_box, LV_ALIGN_TOP_MID, 0, 300);
    m_details_text = set_label_text(m_details_box, result.c_str(), 0, 0, font_40_semi, text_color_int);
    lv_obj_align(m_title_text, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_align(m_title_text, LV_TEXT_ALIGN_CENTER, 0);
    bool retry = true;

    while(retry)
    {
        result.clear();
        auto res = get_tpm_json();

        if(res.find("error") != res.end())
        {
            DEBUG_MSG(COMMON, DEBUG, "error = " << res["error"] << "\n");
            background = bg_color_danger;
        }

        for(auto &r : res)
        {
            result += r.first + ": " + r.second + "\n";
        }

        result += "ver.: " + PROJECT_TPM_VERSION + "\n";

        if(res.find("error") == res.end())
        {
            init_net_config();
            retry = false;
            background = bg_color_int;
        }

        lv_obj_set_style_bg_color(m_main, background, 0);
        lv_obj_set_style_bg_color(m_title_box, background, 0);
        lv_obj_set_style_bg_color(m_details_box, background, 0);
        lv_label_set_text(m_details_text, result.c_str());

        for(auto i = 0; i < 100; i++)
        {
            // Draw the screen and retry
            lv_timer_handler();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

std::tuple<std::array<unsigned char, 4>, std::string> get_nuid()
{
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    std::array<unsigned char, 4> nuid;
    csdInitialize(&xInitParameters);

    if(csdGetNuid(nuid.data()) != CSD_NO_ERROR)
    {
        nuid.fill(0);
    }

    csdTerminate(&xTermParameters);
    std::stringstream ss;

    for(int i = 0; i < 4; ++i)
    {
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nuid[i]);
    }

    std::string nuid_str = ss.str();
    return std::make_tuple(nuid, nuid_str);
}

std::tuple<std::array<unsigned char, 4>, std::string, std::string> get_casn()
{
    std::array<unsigned char, 4> casn;
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    csdInitialize(&xInitParameters);

    if(csdGetStbCaSn(casn.data()) != CSD_NO_ERROR)
    {
        casn.fill(0);
    }

    // String Decimal do CAID
    TChar stb_casn_dec_str[17] = {0};
    dptStbCaSnToString(casn.data(), stb_casn_dec_str);
    // String Hexadecimal do CAID
    auto casn_int = dptArrayToUnsignedIntN(casn.data(), 4);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase << std::setw(8) << casn_int;
    auto casn_str = ss.str();
    csdTerminate(&xTermParameters);
    return std::make_tuple(casn, casn_str, stb_casn_dec_str);
}

std::tuple<std::array<unsigned char, 4>, std::string> get_nuid_check_number()
{
    std::array<unsigned char, 4> nuid_check_number;
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    csdInitialize(&xInitParameters);

    if(csdGetNUIDCheckNumber(nuid_check_number.data()) != CSD_NO_ERROR)
    {
        DEBUG_MSG(COMMON, DEBUG, "csdGetNUIDCheckNumber error\n");
        nuid_check_number.fill(0);
    }
    else
    {
        DEBUG_MSG(COMMON, DEBUG, "csdGetNUIDCheckNumber called\n");
    }

    csdTerminate(&xTermParameters);
    std::stringstream ss;

    for(int i = 0; i < 4; ++i)
    {
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nuid_check_number[i]);
    }

    std::string nuid_check_number_str = ss.str();
    DEBUG_MSG(COMMON, DEBUG, "nuid_check_number_str = " << nuid_check_number_str << "\n");
    return std::make_tuple(nuid_check_number, nuid_check_number_str);
}

int get_cscd_check_number(std::array<unsigned char, 16> cscd, std::array<unsigned char, 4> &cscd_check_number)
{
    auto res = CSD_NO_ERROR;
    TCsdInitParameters xInitParameters;
    TCsdTerminateParameters xTermParameters;
    xInitParameters.xReservd = 0;
    xTermParameters.xReservd = 0;
    csdInitialize(&xInitParameters);

    if(csdGetCSCDCheckNumber(cscd.data(), cscd_check_number.data()) != CSD_NO_ERROR)
    {
        res = CSD_ERROR;
    }

    csdTerminate(&xTermParameters);
    return res;
}

std::map<std::string, std::string> get_tpm_json()
{
    std::map<std::string, std::string> result;

    do
    {
        if(is_there_flashdisk() == false)
        {
            result["error"] = "Pendrive não encontrado";
            break;
        }

        auto res = get_device_map();

        for(auto &r : res)
        {
            DEBUG_MSG(COMMON, DEBUG, "device: " << r.first << " - " << r.second << "\n");
        }

        std::string content = "";
        auto file = read_file_by_name("/mnt/usb", "TPM.json"); // exec_system_command

        if(file.empty())
        {
            result["error"] = "Arquivo TPM.json não encontrado";
            break;
        }

        std::ifstream ifs(file);
        content = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        DEBUG_MSG(COMMON, DEBUG, "content = " << content << "\n");
        // Captura os valores do JSON
        cJSON *json = cJSON_Parse(content.c_str());

        if(json == nullptr)
        {
            result["error"] = "jSON inválido";
            break;
        }

        cJSON *item = cJSON_GetObjectItem(json, "ip_addr");

        if(item == nullptr)
        {
            result["error"] = "ip_addr não encontrado";
            break;
        }

        result["ip_addr"] = item->valuestring;
        struct sockaddr_in sa;

        if(inet_pton(AF_INET, item->valuestring, &(sa.sin_addr)) != 1)
        {
            result["error"] = "ip_addr inválido";
            break;
        }

        item = cJSON_GetObjectItem(json, "network_mask");

        if(item == nullptr)
        {
            result["error"] = "network_mask não encontrado";
            break;
        }

        result["network_mask"] = item->valuestring;

        if(inet_pton(AF_INET, item->valuestring, &(sa.sin_addr)) != 1)
        {
            result["error"] = "network_mask inválido";
            break;
        }

        item = cJSON_GetObjectItem(json, "logprod_ip_addr");

        if(item == nullptr)
        {
            result["error"] = "logprod_ip_addr não encontrado";
            break;
        }

        result["logprod_ip_addr"] = item->valuestring;

        if(inet_pton(AF_INET, item->valuestring, &(sa.sin_addr)) != 1)
        {
            result["error"] = "logprod_ip_addr inválido";
            break;
        }

        item = cJSON_GetObjectItem(json, "server_port");

        if(item == nullptr)
        {
            result["error"] = "server_port não encontrado";
            break;
        }

        result["server_port"] = std::to_string(item->valueint);

        if(item->valueint <= 0 || item->valueint > 65535)
        {
            result["error"] = "server_port inválido";
            break;
        }
    }
    while(false);

    return result;
}

} // namespace tpm

} // namespace mb
