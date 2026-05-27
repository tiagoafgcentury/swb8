#pragma once

#include "common/mb_globals.h"
#include "mb_events.h"
#include "mb_menu_resources.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "tasks/mb_task_cas.h"

#include <lvgl.h>

#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <map>

#define MAX_NUM_LINES 5

using namespace std::chrono_literals;

namespace mb {

class OSD_Popup_Message :
    public OSD,
    public Remote_Control_Handler,
    public std::enable_shared_from_this<OSD_Popup_Message>
{
private:

    static constexpr auto width  = DISPLAY_WIDTH  / 1.2;
    static constexpr auto heigth = DISPLAY_HEIGHT / 1.6;
    lv_obj_t *m_caid { nullptr };
    lv_obj_t *m_scua { nullptr };
    lv_obj_t *text_label { nullptr };

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_scua_cb;

    std::array<lv_obj_t *, static_cast<int>(Message_Categories::COUNT)> m_messages {};
    std::array<lv_timer_t *, static_cast<int>(Message_Categories::COUNT)> m_popup_timers {};

    static std::shared_ptr<OSD_Popup_Message> s_instance;
    static void timer_cb(lv_timer_t *_timer);

    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);
    void hide_popup_message(Message_Categories _category, bool _do_cleanup);

    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    std::map<int, std::string> m_nagra_code_map =
    {
        {0,    "Canal não codificado.\tNon Scrambled Channel."},
        {1,    "Canal liberado pelo smartcard.\tChannel Unlocked by Smart Card."},
        {2,    "Canal liberado pelo smartcard.\tChannel Unlocked by Smart Card."},
        {100,  "Acesso negado pelo smartcard.\tAccess Denied by Smart Card."},
        {101,  "Acesso negado, smartcard ausente ou incompatível.\tAccess Denied, Smart Card Missing or Incompatible."},
        {102,  "Acesso negado: o smartcard não é válido.\tAccess Denied: Invalid Smart Card."},
        {103,  "O smartcard está suspenso. Acesso não foi autorizado.\tSmart Card Suspended. Access Not Authorized."},
        {104,  "Este programa não está disponível para esta região.\tThis Program is Not Available in This Region."},
        {105,  "Acesso não autorizado: saldo insuficiente.\tAccess Not Authorized: Insufficient Balance."},
        {106,  "Acesso não autorizado: o programa é protegido contra cópia.\tAccess Not Authorized: Program is Copy-Protected."},
        {107,  "Acesso não autorizado devido às configurações de controle parental.\tAccess Not Authorized Due to Parental Control Settings."},
        {108,  "Acesso negado.\tAccess Denied."},
        {109,  "Acesso não autorizado: o smartcard não está pareado.\tAccess Not Authorized: Smart Card Not Paired."},
        {110,  "Acesso não autorizado: o smartcard não está pareado.\tAccess Not Authorized: Smart Card Not Paired."},
        {111,  "O programa é incompatível com este receptor.\tProgram is Incompatible with this Receiver."},
        {112,  "Acesso negado: este programa não é suportado pelo receptor.\tAccess Denied: This Program is Not Supported by the Receiver."},
        {113,  "Acesso será liberado. Por favor, aguarde alguns instantes\tAccess Will Be Granted. Please Wait a Moment."},
        {114,  "Acesso ao programa não foi autorizado devido a uma violação das regras de uso.\tAccess to the Program Was Not Authorized Due to a Violation of the Usage Rules."},
        {115,  "Acesso negado: é necessário ativar o receptor.\tAccess Denied: Activation Required."},
        {116,  "Acesso negado.\tAccess Denied."},
        {117,  "Acesso negado. O software do aparelho precisa ser atualizado.\tAccess Denied. Software Upgrade Required."},
        {118,  "Acesso ao programa não é permitido devido a novas regras de uso.\tAccess to the Program is Not Authorized Due to New Usage Rules."},
        {1000, "Acesso é concedido pelo smartcard para um serviço PPT.\tAccess Granted by Smart Card for a PPT Service."},
        {1001, "Acesso bloqueado pelo controle parental.\tAccess Blocked by Parental Control."},
        {1002, "Acesso negado: direito de acesso suspenso.\tAccess Denied: Access Right Suspended."},
        {1003, "Acesso negado: nenhum período ativo para o conteúdo Pay Per Time.\tAccess Denied: No Active Period for Pay Per Time Content."}
    };

public:

    static std::shared_ptr<OSD_Popup_Message> get_instance();
    OSD_Popup_Message();
    virtual ~OSD_Popup_Message();

    void hide_popup_message(Message_Categories _category)
    {
        hide_popup_message(_category, true);
    }

    void show_popup_message(const Event_Display_Message &_message);
    void process_nagra_message(std::string &_message);
};

} // namespace mb

