# UI, OSD e LVGL

## Visão Geral

A UI fica em `ui/lvgl` e ? executada via `Task_OSD`. Ela renderiza menus, mensagens, listas, diagnóstico, EPG, instalação fácil, edição de satélite, chave/DiSEqC, busca manual/automática e recursos multimídia.

Regra arquitetural importante:

> A UI deve refletir estado e postar eventos; ela não deve fazer operações pesadas ou acessar hardware/banco diretamente fora dos fluxos de `Task`.

Na prática, algumas telas atuais ainda disparam eventos que podem provocar persistência, tuning, lineup rebuild ou player restart. Por isso ? essencial manter callbacks e estados internos limpos.

## Roteamento de Controle Remoto

Cada tela implementa `handle_event_remote_control()`. O controle remoto trafega como `Event_Remote_Control`, com teclas como:

- `KEY_OK`
- `KEY_VOLTAR`
- `KEY_CHUP`
- `KEY_CHDOWN`
- `KEY_VOLUP`
- `KEY_VOLDOWN`
- teclas numericas

Cuidados atuais:

- Tratar `KEY_OK` explicitamente em telas com ações especiais.
- Limpar sequencias internas ao abrir/fechar telas.
- Não deixar modal sem `VOLTAR`/cancelamento quando uma operação pode travar.

## Telas Críticas

### Lista de canais

Arquivos:

- `ui/lvgl/mb_osd_channel_list.cpp`
- `ui/lvgl/mb_osd_channel_detail.cpp`
- `ui/lvgl/mb_osd_guide_channel.cpp`

Riscos:

- Usar `viewer_channel` ou `service_id` isolado em multi-satélite.
- Guardar `Service*` entre eventos.
- Mostrar banda/satélite global em vez da origem real do canal.

### Instalação Fácil e Atualização de Lista

Arquivos:

- `ui/lvgl/mb_osd_instala_facil.cpp`
- `ui/lvgl/mb_osd_fast_install.cpp`
- `ui/lvgl/mb_osd_channel_list_update.cpp`
- `ui/lvgl/mb_osd_auto_search_channel_list.cpp`

Riscos:

- Callback de build de lineup antigo.
- `m_demux_lineup` reaproveitado com satélite/policy antiga.
- UI ficar em estado `Start` sem callback final.
- Reset parcial de objetos LVGL/timers.

### Edição de Satélite e Chave

Arquivos:

- `ui/lvgl/mb_osd_edit_satellite.cpp`
- `ui/lvgl/mb_osd_select_satellite.cpp`
- `ui/lvgl/mb_osd_select_switch.cpp`

Riscos atuais observados:

- Satélite novo (`id == 0`) ? salvo sem retorno imediato do ID gerado.
- Segunda edição pode operar sobre ID antigo/inválido.
- Estado interno de DiSEqC (`m_diseq_c_active`, `m_diseq_c_selected`) precisa ser resetado entre aberturas.
- `update_satellite()` acessa `current_lineup->transponders[0]` sem validar lista vazia.
- O fluxo de salvar satélite novo posta `post_event_add_satellite()` e fecha a tela sem receber `last_insert_rowid()` ou novo `sat_id`.
- O update de satélite atual dispara relock diretamente pela UI; antes disso deve validar lista de transponders e idealmente delegar a decisão para a task apropriada.

### Agenda, Lembrete e Gravação

Arquivos:

- `ui/lvgl/mb_osd_scheduled_edit.cpp`
- `ui/lvgl/mb_osd_scheduled_list.cpp`
- `ui/lvgl/mb_osd_program_reminder.cpp`
- `ui/lvgl/mb_osd_guide_channel.cpp`

Riscos atuais observados:

- A agenda exibe, edita e dispara canais usando `service_id` isolado em alguns fluxos.
- `OSD_Program_Reminder::change_channel()` procura o primeiro serviço com o mesmo `service_id`, sem comparar `transponder_id`.
- Em multi-satélite, um lembrete ou gravação pode apontar para canal errado se houver IDs repetidos.
- Ao corrigir agenda, levar também `transponder_id`/`srv_id` até a UI, não apenas ao banco.

### Mensagens de Sistema

Arquivo:

- `ui/lvgl/mb_osd_message_system_restart.cpp`

Essa tela ? usada quando o sistema precisa reiniciar por solicitação CAS/Nagra. Ela seta flags globais de encerramento/reboot após contagem.

## LVGL: Objetos e Timers

Macros comuns:

- `DELETE_OBJ(obj)`
- `DELETE_TIMER(tmr)`

Boas práticas:

- Usar `lv_obj_null_on_delete()` quando o ponteiro precisa ser zerado por LVGL.
- Deletar timers no destrutor da tela.
- Evitar callbacks LVGL capturando `this` quando o objeto pode ser destruido antes do callback.
- Resetar estados internos no destrutor e no método que fecha opções.

## Identidade Visual x Identidade Lógica

Em multi-satélite:

- O número exibido (`viewer_channel`) ? apenas identidade visual.
- O canal real precisa ser resolvido por `transponder_id + service_id`.
- O satélite exibido na UI deve vir da origem do canal, não da configuração global.

## Checklist para Novas Telas

- A tela tem caminho de saída com `KEY_VOLTAR`?
- Eventos pesados são postados para `Task` apropriada?
- Nenhum ponteiro para item de `std::vector` ? guardado para uso futuro?
- Timers LVGL são deletados no fechamento?
- Estado interno e resetado ao abrir e fechar?
- Em multi-satélite, a tela usa `transponder_id + service_id`?
- Labels de satélite/banda/policy usam a origem real do canal/satélite?
- Se a tela trabalha com agenda/PVR, ela preserva uma chave estável do canal e não apenas `service_id`?
