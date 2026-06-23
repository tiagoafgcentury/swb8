# Troubleshooting e Diagnóstico

## Objetivo

Este guia descreve como investigar travamentos, reboots, falhas de sintonia, regionalização incorreta, problemas de DiSEqC e bugs de UI no MBGUI atual.

Para bugs documentados por release, use a pasta:

```text
doc/documentacao-mbgui/bugs
```

## Classificacao Rápida de Sintomas

| Sintoma | Hipóteses comuns |
|---|---|
| Receptor reinicia inteiro | watchdog, CAS hardware reset, endpoint TPM `/system/reboot`, supervisor externo, crash nativo. |
| App reinicia mas receptor não | `g_mbgui_restart_on_exit=true` e `execve("/proc/self/exe", ...)`. |
| UI congela | callback não chamado, deadlock, fila IPC cheia, task bloqueada, modal sem saída. |
| Canal errado em multi-satélite | uso de `service_id`/`viewer_channel` isolado, DiSEqC errado, `satellite_id` incorreto. |
| Sem sinal após troca | DiSEqC não comutou, satélite não encontrado, fallback de porta, transponder sem `satellite_id`. |
| Regionalização errada/blackout | `network_id`, `zone_id`, `bouquet_id`, CAS/Nagra ou cache antigo divergente. |
| Reboot ao editar UI | ponteiro inválido, lista vazia acessada por `[0]`, timer LVGL usando objeto destruido. |

## Logs Essenciais

Para qualquer bug, registrar:

- build/versionamento
- data/hora
- fluxo exato de teclas
- se reiniciou app ou hardware inteiro
- ?ltimo canal/satélite
- estado de standby/factory reset
- logs imediatamente antes do reboot

Para multi-satélite/DiSEqC:

- `satellite_id`
- nome do satélite
- `switch_type`
- `switch_pos`
- `transponder_id`
- frequência/polaridade/symbol rate
- resultado de lock
- `network_policies`

Para regionalização:

- `network_id`
- operador/policy atual
- `zone_id`
- `bouquet_id`
- origem do valor aplicado
- eventos CAS/Nagra proximos

Para banco:

- IDs da tabela `sat`
- `transponder_id`
- `service_id`
- `srv_id`
- quantidade de linhas afetadas em updates
- se listas carregadas estavam vazias

## Flags e Telemetria

Documentos anteriores citam:

```sh
export MBGUI_ENABLE_MULTISAT_DIAGNOSTICS=1
export MBGUI_ENABLE_SCHEDULE_LEGACY_MIGRATION=1
```

Antes de depender dessas flags, confirme no código do release se ainda estão implementadas no ponto investigado. Se não houver telemetria ativa, adicionar logs pontuais nos arquivos envolvidos.

Estado nesta release:

- Essas flags não aparecem implementadas no código fonte atual.
- Para diagnóstico real, use flags CMake existentes como `MBGUI_USE_EXTRA_DEBUGGING`, `MBGUI_PERIODIC_DUMP` e `MBD_DEFAULT_LOG_LEVEL=LOG_LEVEL_DEBUG`, ou adicione logs pontuais no fluxo investigado.

## Investigando Reboots

Verificar em ordem:

1. O log mostra `execve restarting app`?
2. Algum fluxo chamou `Task_Application::system_exit()`?
3. CAS/Nagra recebeu `CAK_EVT_CHIPSET_HARDWARE_RESET`?
4. OSD mostrou mensagem de sistema precisa reiniciar?
5. Endpoint `/system/reboot` foi chamado?
6. Watchdog deixou de ser pingado por mais de 10só
7. Houve acesso inválido a vetor/lista vazia?
8. Houve `mb_assert(false)` em debug?

Pontos de código:

- `src/mb_main.cpp`
- `src/tasks/mb_task_application.cpp`
- `src/cas/nagra/mb_nagra.cpp`
- `ui/lvgl/mb_osd_message_system_restart.cpp`
- `src/hal/*/mb_watchdog.cpp`
- `src/tpm/mb_tpm.cpp`

## Investigando Travamentos

Verificar:

- `Task::run_processes()` está rodando?
- Algum `process()` de task demora demaisó
- `mq_send()` está em retry por fila cheia?
- `open_queue()` está aguardando fila indefinidamente?
- A tela LVGL espera callback que nunca chega?
- Algum lock de lineup ficou preso?

Adicionar medicao de tempo em:

- cada `PROFILE_TASK`
- `Task::process_events()`
- callbacks de UI
- handlers de banco/demux/tuner

## Investigando DiSEqC e Sintonia

Checklist:

1. O canal/serviço possui `transponder_id` com `satellite_id` correto?
2. O satélite existe na tabela `sat`?
3. `Config::get_diseqc_type(satellite_id)` retorna valor esperado?
4. `Config::get_diseqc_port(satellite_id)` retorna porta fisica correta?
5. DiSEqC foi enviado antes do lock? Na release atual isso precisa ser confirmado no HAL, porque o ALi chama `set_diseqc()` após iniciar `aui_nim_connect()`.
6. O lock ocorreu no transponder correto?
7. O demux salvou transponders com `satellite_id` correto?

## Investigando Agenda/PVR em Multi-Satélite

Checklist:

1. A agenda carregada do banco representa `srv_id` real ou apenas `service_id`?
2. A tela de agenda preservou `transponder_id` junto do canal selecionado?
3. `OSD_Program_Reminder` encontrou o canal por chave estável ou pegou o primeiro `service_id` igualê
4. `Task_Player::handle_event_start_recording()` recebeu identificador suficiente para achar o canal certo?
5. Existem canais com mesmo `service_id` em satélites diferentes no banco?
6. O teste inclui lembrete e gravação em canal de outro satélite?

## Investigando Bugs de Edição de Satélite

Para novo satélite:

- verificar se `id == 0` antes do primeiro save
- obter `last_insert_rowid()` após insert
- confirmar se UI recebeu o novo ID
- confirmar se a segunda edição entra em fluxo de add ou update
- verificar `sqlite3_changes()` no update
- validar `current_lineup->transponders.empty()` antes de relock

Arquivos:

- `ui/lvgl/mb_osd_edit_satellite.cpp`
- `src/tasks/mb_task_database.cpp`
- `src/common/mb_config.cpp`

## Checklist de Regressão

- Zapping rápido por CH+/CH- em 1 satélite.
- Zapping entre 2 ou mais satélites DiSEqC.
- Criar satélite novo e editar chave várias vezes.
- Instala Fácil D2 -> Sky -> D2 sem reset.
- Factory reset e reativação por operadora.
- Abrir/fechar Info 2x e pressionar OK repetidamente.
- Lineup vazio ou com apenas 1 canal.
- EPG/agenda em canais com mesmo `service_id` em satélites diferentes.
- Reordenação de canais quando há `service_id`/`viewer_channel` repetidos em satélites diferentes.
