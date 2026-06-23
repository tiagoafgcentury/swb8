# Arquitetura Atual do Software B8

## Visão Geral

O MBGUI ? a aplicação central dos receptores MidiaBox. Ele roda em Linux embarcado e orquestra UI, sintonia, demux, player, banco local, CAS/Nagra, configuração de satélites e operações de sistema.

O desenho atual ? baseado em um loop principal com `Task::run_processes()`, eventos internos e IPC por filas POSIX quando há separação entre processos GUI/CAS. A aplicação também usa watchdog de hardware; portanto qualquer handler bloqueante, deadlock ou loop ocupado pode virar travamento seguido de reboot.

## Camadas Principais

| Camada | Diretório | Responsabilidade atual |
|---|---|---|
| Domínio comum | `src/common` | Tipos, lineup, configuração, estado persistido, satélites, hash e utilitários. |
| DVB | `src/dvb` | Parsers de tabelas DVB: PAT, PMT, SDT, NIT, EIT, TOT/TDT e descritores. |
| HAL | `src/hal` | Abstração de hardware ALi/Montage: tuner, demux, display, player, HDMI, DiSEqC, watchdog, remote control. |
| Tasks | `src/tasks` | Orquestração por eventos: application, database, demux, tuner, player, OSD, CAS, EIT, remote control. |
| CAS/Nagra | `src/cas/nagra` | Integração Nagra/CAK, callbacks CAS, descrambling, reset de hardware solicitado pelo CAS e PVR CAS. |
| TPM/API HTTP | `src/tpm` | Servidor HTTP/TPM, endpoints de sistema e operações de configuração/diagnóstico. |
| UI | `ui/lvgl` | Telas LVGL, menus, lista de canais, instalação fácil, satélite, DiSEqC/chave, EPG, mensagens e diagnóstico. |

## Modelo de Execução

O `main()` cria objetos de sistema e tasks, habilita watchdog e roda o loop:

```cpp
while(g_mbgui_keep_running)
{
    wd.ping();
    mb::Task::run_processes();
    std::this_thread::sleep_for(LOOP_TIME);
}
```

Pontos importantes do código atual:

- `src/mb_main.cpp` define flags globais de vida/restart: `g_mbgui_keep_running`, `g_mbgui_reboot_after_exit`, `g_mbgui_restart_on_exit`, `g_mbgui_do_factory_reset`.
- O loop principal usa `LOOP_TIME = 150ns`, o que ? praticamente busy-loop.
- Quando `g_mbgui_restart_on_exit` está ativo, o app reinicia a si mesmo com `execve("/proc/self/exe", argv, nullptr)`.
- Quando CAS solicita reset de hardware, o fluxo pode encerrar o app para permitir reboot real do receptor.

## Tasks e Eventos

`Task::run_processes()` chama os `process()` das tasks registradas por macro e depois processa eventos e IPC:

- `Task_Application`: inicialização, estado global, standby, factory reset, clock, linha macro de aplicação.
- `Task_Database`: SQLite, satélites, transponders, serviços, agenda/PVR, lineup salvo.
- `Task_Demux`: coleta de tabelas DVB, build/update de lineup, PMT/SDT/NIT/EIT.
- `Task_Tuner`: lock fisico, DiSEqC, blind scan, autodetecção de LNBf.
- `Task_Player`: troca de canal, player A/V, CAS descramble, reinício do player.
- `Task_OSD`: roteamento de UI LVGL, telas, mensagens, controle remoto.
- `Task_CAS`: ponte com Nagra/CAK quando compilado como CAS.
- `Task_EIT_Events`: eventos de EPG e tratamento de memória baixa.

## IPC GUI/CAS

Quando `MBGUI_HAS_IPC` está ativo, GUI e CAS se comunicam por filas POSIX (`mqueue`). O código usa:

- `open_local_queue()`
- `open_remote_queue()`
- `mq_send()`
- `mq_receive()`

Riscos atuais documentados:

- Espera indefinida por fila em `open_queue()`.
- Retry de `mq_send()` pode bloquear até cerca de 2 segundos por mensagem quando a fila está cheia.
- Muitos handlers default em `Task` fazem `mb_assert(false)`/`__builtin_unreachable()` se receberem evento não sobrescrito.

## Multi-Satélite e Identidade

O projeto atual suporta multi-satélite e DiSEqC. A regra central e não tratar `service_id` sozinho como identidade de canal.

No código atual, `Transponder_Id` ? um identificador de 64 bits que inclui:

- frequência
- polaridade
- `satellite_id` nos bits altos

Um canal/serviço deve ser identificado, para fins seguros, por:

- `transponder_id`
- `service_id`

ou por uma chave equivalente estável. Evite usar apenas `service_id`, `viewer_channel` ou ponteiro `Service*` em fluxos assíncronos.

Estado atual observado na release:

- A constraint do banco em `srv` confirma a identidade por `(transponder_id, service_id)`.
- Alguns fluxos ainda usam `service_id` isolado, especialmente agenda/PVR/lembrete e reordenação de lista de canais.
- A coluna `agenda.srv_id` existe, mas o código atual grava/carrega nela o valor de `ScheduleEntry::service_id`, não necessariamente o `srv_id` relacional.
- Eventos de canal ainda trafegam `Service*`, que contínua sendo risco quando a lista de serviços e reconstruída.

## DiSEqC e Ordem de Sintonia

Arquiteturalmente, a seleção de porta DiSEqC deve acontecer antes da tentativa de lock do transponder.

Na release atual, isso precisa ser validado por HAL:

- No HAL ALi, a chamada de conexão do tuner ocorre antes de `set_diseqc()`.
- No HAL Montage, o caminho principal de lock não evidência envio de DiSEqC.
- Bugs de "sem sinal" em multi-satélite devem registrar chipset, satélite, porta, transponder e ordem real entre DiSEqC/LNB/lock.

## Pontos de Atenção da Arquitetura Atual

- Eventos de canal ainda trafegam `Service*` em vários pontos, o que pode ser inseguro se o vetor de serviços for alterado.
- Alguns fluxos acessam `services[0]` ou `transponders[0]` sem validar lista vazia.
- O estado de satélite atual vem de `State_File`, `Config`, banco `sat`, lineup e UI; inconsistências entre eles geram bugs de DiSEqC/regionalização.
- Agenda/PVR e ordenação de canais ainda exigem revisao para carregar uma chave estável do canal de ponta a ponta.
- O CAS/Nagra pode solicitar reset de hardware via `CAK_EVT_CHIPSET_HARDWARE_RESET`.
- A pasta `doc/documentacao-mbgui/bugs` deve concentrar PRDs, análises e histórico de bugs por release.
