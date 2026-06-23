# Standby - Funcao a Funcao

## Entrada por controle remoto ou painel

### `Task_Remote_Control::Task_Remote_Control()`

Arquivo:

- `src/tasks/mb_task_remote_control.cpp`

Responsabilidade:

- registrar o handler de teclas vindo do HAL
- decidir se `KEY_POWER` vai direto para toggle de energia ou passa pela pilha normal de OSD

Regra observada:

- se media player ou PVR estiverem ativos, publica `Event_Remote_Control`
- caso contrario, chama `post_event_toggle_power()`

### `Remote_Control::read_keys()`

Arquivos:

- `src/hal/ALi/mb_remote_control.cpp`
- `src/hal/Montage/mb_remote_control.cpp`

Responsabilidade:

- converter evento de IR em `Remote_Control_Key`
- encaminhar `KEY_POWER` para a camada de task

### `Remote_Control::read_keys_front_panel()`

Arquivo:

- `src/hal/ALi/mb_remote_control.cpp`

Responsabilidade:

- monitorar botoes fisicos do painel frontal
- converter botao de energia em `Remote_Control_Key::KEY_POWER`

## Evento global de standby

### `Task::post_event_toggle_power()`

Arquivo:

- `src/tasks/mb_task.cpp`

Responsabilidade:

- publicar o evento global `handle_event_toggle_power`

Na pratica, o handler relevante para este fluxo e o de `Task_Application`.

## Orquestracao principal

### `Task_Application::handle_event_toggle_power()`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- centralizar a transicao de standby

Sequencia:

1. chama `System::stand_by(g_production_final_test)`
2. recebe o novo estado booleano de standby
3. carrega `State_File::App_State_File`
4. grava `file.stand_by` ou `file.stand_by_in_production_mode`
5. executa `file.write()`
6. chama `system_exit()`

Observacao:

- a funcao nao apenas apaga a saida de video; ela tambem prepara o proximo boot persistindo o estado

### `Task_Application::system_exit()`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- encerrar o MBGUI de forma controlada

Efeito:

- `g_mbgui_keep_running = false`
- `g_mbgui_restart_on_exit = true`
- o loop principal termina
- o app volta a subir

## HAL e fake standby

### `System::stand_by(bool)`

Arquivo:

- `src/hal/ALi/mb_system.cpp`

Responsabilidade:

- alternar o estado de fake standby

Logica:

- le `State_File::App_State_File`
- se o estado salvo indica ligado, executa `fake_power_off()` e seta `s_fake_standby_mode = true`
- se o estado salvo indica standby, executa `fake_power_on()` e seta `s_fake_standby_mode = false`

### `System::fake_power_off()`

Arquivo:

- `src/hal/ALi/mb_system.cpp`

Responsabilidade:

- desligar as saidas visiveis do receptor

Chamadas:

- `m_hdmi->hdmi_output_off()`
- `Display::set_cvbs_off()`

### `System::fake_power_on()`

Arquivo:

- `src/hal/ALi/mb_system.cpp`

Responsabilidade:

- reabilitar as saidas visiveis do receptor

Chamadas:

- `m_hdmi->hdmi_output_on()`
- `Display::set_cvbs_on()`

## Sleep timer

### `Task_Application::handle_event_remote_control(KEY_SLEEP)`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- quando em `ST_IDLE`, abrir o menu de sleep via `post_event_osd_menu_plus(false, true, ...)`

### `Task_OSD::handle_event_osd_menu_plus(...)`

Arquivo:

- `src/tasks/mb_task_osd.cpp`

Responsabilidade:

- instanciar `OSD_Menu_Plus`
- permitir abertura da variante de sleep

### `OSD_Menu_Plus_Sleep::set_sleep_timer_value()`

Arquivo:

- `ui/lvgl/mb_osd_menu_plus_sleep.cpp`

Responsabilidade:

- armar ou cancelar um `lv_timer` em minutos

### `OSD_Menu_Plus_Sleep::process_sleep_timer()`

Arquivo:

- `ui/lvgl/mb_osd_menu_plus_sleep.cpp`

Responsabilidade:

- decrementar o contador
- quando chega a zero, apagar o timer e chamar `start_standby()`

### `OSD_Menu_Plus_Sleep::start_standby()`

Arquivo:

- `ui/lvgl/mb_osd_menu_plus_sleep.cpp`

Responsabilidade:

- chamar `Task::post_event_toggle_power()`

Conclusao:

- o sleep timer nao tem fluxo proprio de desligamento; ele agenda o mesmo evento do `POWER`

## Startup e retorno

### `Task_Application::is_in_stand_by()`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- consultar o arquivo de estado no boot
- decidir se o app deve iniciar em `ST_STAND_BY_MODE`

### `System::check_standby_mode(bool)`

Arquivo:

- `src/hal/ALi/mb_system.cpp`

Responsabilidade:

- reaplicar fake standby logo que a HAL sobe

Efeito:

- se arquivo indica standby, aplica `fake_power_off()` e LED vermelho
- caso contrario, aplica `fake_power_on()` e LED verde

### `Task_Application::process()`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- no estado `ST_STARTING`, decidir o caminho inicial da aplicacao

Ramo relevante:

- se `is_in_stand_by()` retornar `true`, muda para `ST_WAITING_FOR_APP_STATE` e chama `post_event_application_state_load()`

## Persistencia e restauracao

### `Task_Application::application_state_save()`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- montar `Event_Save_Application_State` com canal atual, mute, volume, standby, tipo de lista e satelite atual

### `Task_Database::handle_event_application_state_save(...)`

Arquivo:

- `src/tasks/mb_task_database.cpp`

Responsabilidade:

- gravar os campos recebidos em `State_File::App_State_File`

### `Task_Database::handle_event_application_state_load()`

Arquivo:

- `src/tasks/mb_task_database.cpp`

Responsabilidade:

- ler `State_File::App_State_File`
- montar `Event_Save_Application_State`
- publicar `post_event_application_state_loaded(...)`

### `Task_Application::handle_event_application_state_loaded(...)`

Arquivo:

- `src/tasks/mb_task_application.cpp`

Responsabilidade:

- restaurar estado apos leitura do arquivo

Comportamento em standby:

- restaura volume e mute
- tenta localizar canal salvo
- localiza o transponder desse canal
- chama `post_event_cas_start_emm_filtering(tp)`
- chama `post_event_transponder_lock(tp)`

## Demux, EMM e OTA

### `Task_Demux::handle_event_cas_start_emm_filtering(const Transponder *)`

Arquivo:

- `src/tasks/mb_task_demux.cpp`

Responsabilidade:

- colocar o demux em `ST_START_EMM_FILTERING`
- pedir lock do transponder para iniciar filtragem CAS

### `Task_Demux::handle_event_transponder_locked(const Event_Tuner_Lock&)`

Arquivo:

- `src/tasks/mb_task_demux.cpp`

Responsabilidade:

- quando o tuner trava em contexto relevante, pedir CAT e manter fluxo CAS/EMM

### `Task_Demux::process()`

Arquivo:

- `src/tasks/mb_task_demux.cpp`

Responsabilidade durante standby:

- verificar periodicamente se o app esta em standby
- a cada ciclo definido, pode chamar `check_for_otas()`

### `Task_Demux::check_for_ota_sky(uint32_t)`

Arquivo:

- `src/tasks/mb_task_demux.cpp`

Responsabilidade:

- tratar caminho especifico de OTA Sky
- quando em standby e frequencia atual e zero, interpretar o contexto como retorno de power on e tentar travar transponder de referencia

## Fluxos alternativos que nao sao o standby normal

### `tpm_power_off()`

Arquivo:

- `src/tpm/tpm_api.c`

Responsabilidade:

- implementar caminho de standby/power em baixo nivel com `aui_standby_set_state(setting)`

Observacao:

- parece ligado a TPM/teste/baixo nivel, nao ao fluxo normal de standby do usuario no MBGUI

### `system_power_off(connection_context *)`

Arquivo:

- `src/tpm/mb_tpm.cpp`

Responsabilidade:

- endpoint HTTP `/system/power-off`

Comportamento:

- desmonta jffs2
- executa `poweroff`

Conclusao:

- e poweroff do sistema operacional, nao o fake standby do fluxo comum do receptor

