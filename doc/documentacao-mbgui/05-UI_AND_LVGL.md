# 05 - UI, OSD e LVGL

## Visao Geral

A UI fica em `ui/lvgl` e e executada via `Task_OSD`. Ela renderiza menus, mensagens, listas, diagnostico, EPG, instalacao facil, edicao de satelite, chave/DiSEqC, busca manual/automatica e recursos multimidia.

Regra arquitetural importante:

> A UI deve refletir estado e postar eventos; ela nao deve fazer operacoes pesadas ou acessar hardware/banco diretamente fora dos fluxos de `Task`.

Na pratica, algumas telas atuais ainda disparam eventos que podem provocar persistencia, tuning, lineup rebuild ou player restart. Por isso e essencial manter callbacks e estados internos limpos.

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

- Tratar `KEY_OK` explicitamente em telas com acoes especiais.
- Limpar sequencias internas ao abrir/fechar telas.
- Nao deixar modal sem `VOLTAR`/cancelamento quando uma operacao pode travar.

## Telas Criticas

### Lista de canais

Arquivos:

- `ui/lvgl/mb_osd_channel_list.cpp`
- `ui/lvgl/mb_osd_channel_detail.cpp`
- `ui/lvgl/mb_osd_guide_channel.cpp`

Riscos:

- Usar `viewer_channel` ou `service_id` isolado em multi-satelite.
- Guardar `Service*` entre eventos.
- Mostrar banda/satelite global em vez da origem real do canal.

### Instalacao Facil e Atualizacao de Lista

Arquivos:

- `ui/lvgl/mb_osd_instala_facil.cpp`
- `ui/lvgl/mb_osd_fast_install.cpp`
- `ui/lvgl/mb_osd_channel_list_update.cpp`
- `ui/lvgl/mb_osd_auto_search_channel_list.cpp`

Riscos:

- Callback de build de lineup antigo.
- `m_demux_lineup` reaproveitado com satelite/policy antiga.
- UI ficar em estado `Start` sem callback final.
- Reset parcial de objetos LVGL/timers.

### Edicao de Satelite e Chave

Arquivos:

- `ui/lvgl/mb_osd_edit_satellite.cpp`
- `ui/lvgl/mb_osd_select_satellite.cpp`
- `ui/lvgl/mb_osd_select_switch.cpp`

Riscos atuais observados:

- Satelite novo (`id == 0`) e salvo sem retorno imediato do ID gerado.
- Segunda edicao pode operar sobre ID antigo/invalido.
- Estado interno de DiSEqC (`m_diseq_c_active`, `m_diseq_c_selected`) precisa ser resetado entre aberturas.
- `update_satellite()` acessa `current_lineup->transponders[0]` sem validar lista vazia.

### Mensagens de Sistema

Arquivo:

- `ui/lvgl/mb_osd_message_system_restart.cpp`

Essa tela e usada quando o sistema precisa reiniciar por solicitacao CAS/Nagra. Ela seta flags globais de encerramento/reboot apos contagem.

## LVGL: Objetos e Timers

Macros comuns:

- `DELETE_OBJ(obj)`
- `DELETE_TIMER(tmr)`

Boas praticas:

- Usar `lv_obj_null_on_delete()` quando o ponteiro precisa ser zerado por LVGL.
- Deletar timers no destrutor da tela.
- Evitar callbacks LVGL capturando `this` quando o objeto pode ser destruido antes do callback.
- Resetar estados internos no destrutor e no metodo que fecha opcoes.

## Identidade Visual x Identidade Logica

Em multi-satelite:

- O numero exibido (`viewer_channel`) e apenas identidade visual.
- O canal real precisa ser resolvido por `transponder_id + service_id`.
- O satelite exibido na UI deve vir da origem do canal, nao da configuracao global.

## Checklist para Novas Telas

- A tela tem caminho de saida com `KEY_VOLTAR`?
- Eventos pesados sao postados para `Task` apropriada?
- Nenhum ponteiro para item de `std::vector` e guardado para uso futuro?
- Timers LVGL sao deletados no fechamento?
- Estado interno e resetado ao abrir e fechar?
- Em multi-satelite, a tela usa `transponder_id + service_id`?
- Labels de satelite/banda/policy usam a origem real do canal/satelite?

