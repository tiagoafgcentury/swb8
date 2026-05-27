# 06 - Troubleshooting e Diagnostico

## Objetivo

Este guia descreve como investigar travamentos, reboots, falhas de sintonia, regionalizacao incorreta, problemas de DiSEqC e bugs de UI no MBGUI atual.

Para bugs documentados por release, use a pasta:

```text
doc/documentacao-mbgui/bugs
```

## Classificacao Rapida de Sintomas

| Sintoma | Hipoteses comuns |
|---|---|
| Receptor reinicia inteiro | watchdog, CAS hardware reset, endpoint TPM `/system/reboot`, supervisor externo, crash nativo. |
| App reinicia mas receptor nao | `g_mbgui_restart_on_exit=true` e `execve("/proc/self/exe", ...)`. |
| UI congela | callback nao chamado, deadlock, fila IPC cheia, task bloqueada, modal sem saida. |
| Canal errado em multi-satelite | uso de `service_id`/`viewer_channel` isolado, DiSEqC errado, `satellite_id` incorreto. |
| Sem sinal apos troca | DiSEqC nao comutou, satelite nao encontrado, fallback de porta, transponder sem `satellite_id`. |
| Regionalizacao errada/blackout | `network_id`, `zone_id`, `bouquet_id`, CAS/Nagra ou cache antigo divergente. |
| Reboot ao editar UI | ponteiro invalido, lista vazia acessada por `[0]`, timer LVGL usando objeto destruido. |

## Logs Essenciais

Para qualquer bug, registrar:

- build/versionamento
- data/hora
- fluxo exato de teclas
- se reiniciou app ou hardware inteiro
- ultimo canal/satelite
- estado de standby/factory reset
- logs imediatamente antes do reboot

Para multi-satelite/DiSEqC:

- `satellite_id`
- nome do satelite
- `switch_type`
- `switch_pos`
- `transponder_id`
- frequencia/polaridade/symbol rate
- resultado de lock
- `network_policies`

Para regionalizacao:

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

Antes de depender dessas flags, confirme no codigo do release se ainda estao implementadas no ponto investigado. Se nao houver telemetria ativa, adicionar logs pontuais nos arquivos envolvidos.

## Investigando Reboots

Verificar em ordem:

1. O log mostra `execve restarting app`?
2. Algum fluxo chamou `Task_Application::system_exit()`?
3. CAS/Nagra recebeu `CAK_EVT_CHIPSET_HARDWARE_RESET`?
4. OSD mostrou mensagem de sistema precisa reiniciar?
5. Endpoint `/system/reboot` foi chamado?
6. Watchdog deixou de ser pingado por mais de 10s?
7. Houve acesso invalido a vetor/lista vazia?
8. Houve `mb_assert(false)` em debug?

Pontos de codigo:

- `src/mb_main.cpp`
- `src/tasks/mb_task_application.cpp`
- `src/cas/nagra/mb_nagra.cpp`
- `ui/lvgl/mb_osd_message_system_restart.cpp`
- `src/hal/*/mb_watchdog.cpp`
- `src/tpm/mb_tpm.cpp`

## Investigando Travamentos

Verificar:

- `Task::run_processes()` esta rodando?
- Algum `process()` de task demora demais?
- `mq_send()` esta em retry por fila cheia?
- `open_queue()` esta aguardando fila indefinidamente?
- A tela LVGL espera callback que nunca chega?
- Algum lock de lineup ficou preso?

Adicionar medicao de tempo em:

- cada `PROFILE_TASK`
- `Task::process_events()`
- callbacks de UI
- handlers de banco/demux/tuner

## Investigando DiSEqC e Sintonia

Checklist:

1. O canal/servico possui `transponder_id` com `satellite_id` correto?
2. O satelite existe na tabela `sat`?
3. `Config::get_diseqc_type(satellite_id)` retorna valor esperado?
4. `Config::get_diseqc_port(satellite_id)` retorna porta fisica correta?
5. DiSEqC foi enviado antes do lock?
6. O lock ocorreu no transponder correto?
7. O demux salvou transponders com `satellite_id` correto?

## Investigando Bugs de Edicao de Satelite

Para novo satelite:

- verificar se `id == 0` antes do primeiro save
- obter `last_insert_rowid()` apos insert
- confirmar se UI recebeu o novo ID
- confirmar se a segunda edicao entra em fluxo de add ou update
- verificar `sqlite3_changes()` no update
- validar `current_lineup->transponders.empty()` antes de relock

Arquivos:

- `ui/lvgl/mb_osd_edit_satellite.cpp`
- `src/tasks/mb_task_database.cpp`
- `src/common/mb_config.cpp`

## Checklist de Regressao

- Zapping rapido por CH+/CH- em 1 satelite.
- Zapping entre 2 ou mais satelites DiSEqC.
- Criar satelite novo e editar chave varias vezes.
- Instala Facil D2 -> Sky -> D2 sem reset.
- Factory reset e reativacao por operadora.
- Abrir/fechar Info 2x e pressionar OK repetidamente.
- Lineup vazio ou com apenas 1 canal.
- EPG/agenda em canais com mesmo `service_id` em satelites diferentes.

