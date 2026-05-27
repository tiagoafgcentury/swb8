# 01 - Arquitetura Atual do Sistema MBGUI

## Visao Geral

O MBGUI e a aplicacao central dos receptores MidiaBox. Ele roda em Linux embarcado e orquestra UI, sintonia, demux, player, banco local, CAS/Nagra, configuracao de satelites e operacoes de sistema.

O desenho atual e baseado em um loop principal com `Task::run_processes()`, eventos internos e IPC por filas POSIX quando ha separacao entre processos GUI/CAS. A aplicacao tambem usa watchdog de hardware; portanto qualquer handler bloqueante, deadlock ou loop ocupado pode virar travamento seguido de reboot.

## Camadas Principais

| Camada | Diretorio | Responsabilidade atual |
|---|---|---|
| Dominio comum | `src/common` | Tipos, lineup, configuracao, estado persistido, satelites, hash e utilitarios. |
| DVB | `src/dvb` | Parsers de tabelas DVB: PAT, PMT, SDT, NIT, EIT, TOT/TDT e descritores. |
| HAL | `src/hal` | Abstracao de hardware ALi/Montage: tuner, demux, display, player, HDMI, DiSEqC, watchdog, remote control. |
| Tasks | `src/tasks` | Orquestracao por eventos: application, database, demux, tuner, player, OSD, CAS, EIT, remote control. |
| CAS/Nagra | `src/cas/nagra` | Integracao Nagra/CAK, callbacks CAS, descrambling, reset de hardware solicitado pelo CAS e PVR CAS. |
| TPM/API HTTP | `src/tpm` | Servidor HTTP/TPM, endpoints de sistema e operacoes de configuracao/diagnostico. |
| UI | `ui/lvgl` | Telas LVGL, menus, lista de canais, instalacao facil, satelite, DiSEqC/chave, EPG, mensagens e diagnostico. |

## Modelo de Execucao

O `main()` cria objetos de sistema e tasks, habilita watchdog e roda o loop:

```cpp
while(g_mbgui_keep_running)
{
    wd.ping();
    mb::Task::run_processes();
    std::this_thread::sleep_for(LOOP_TIME);
}
```

Pontos importantes do codigo atual:

- `src/mb_main.cpp` define flags globais de vida/restart: `g_mbgui_keep_running`, `g_mbgui_reboot_after_exit`, `g_mbgui_restart_on_exit`, `g_mbgui_do_factory_reset`.
- O loop principal usa `LOOP_TIME = 150ns`, o que e praticamente busy-loop.
- Quando `g_mbgui_restart_on_exit` esta ativo, o app reinicia a si mesmo com `execve("/proc/self/exe", argv, nullptr)`.
- Quando CAS solicita reset de hardware, o fluxo pode encerrar o app para permitir reboot real do receptor.

## Tasks e Eventos

`Task::run_processes()` chama os `process()` das tasks registradas por macro e depois processa eventos e IPC:

- `Task_Application`: inicializacao, estado global, standby, factory reset, clock, linha macro de aplicacao.
- `Task_Database`: SQLite, satelites, transponders, servicos, agenda/PVR, lineup salvo.
- `Task_Demux`: coleta de tabelas DVB, build/update de lineup, PMT/SDT/NIT/EIT.
- `Task_Tuner`: lock fisico, DiSEqC, blind scan, autodeteccao de LNBf.
- `Task_Player`: troca de canal, player A/V, CAS descramble, reinicio do player.
- `Task_OSD`: roteamento de UI LVGL, telas, mensagens, controle remoto.
- `Task_CAS`: ponte com Nagra/CAK quando compilado como CAS.
- `Task_EIT_Events`: eventos de EPG e tratamento de memoria baixa.

## IPC GUI/CAS

Quando `MBGUI_HAS_IPC` esta ativo, GUI e CAS se comunicam por filas POSIX (`mqueue`). O codigo usa:

- `open_local_queue()`
- `open_remote_queue()`
- `mq_send()`
- `mq_receive()`

Riscos atuais documentados:

- Espera indefinida por fila em `open_queue()`.
- Retry de `mq_send()` pode bloquear ate cerca de 2 segundos por mensagem quando a fila esta cheia.
- Muitos handlers default em `Task` fazem `mb_assert(false)`/`__builtin_unreachable()` se receberem evento nao sobrescrito.

## Multi-Satelite e Identidade

O projeto atual suporta multi-satelite e DiSEqC. A regra central e nao tratar `service_id` sozinho como identidade de canal.

No codigo atual, `Transponder_Id` e um identificador de 64 bits que inclui:

- frequencia
- polaridade
- `satellite_id` nos bits altos

Um canal/servico deve ser identificado, para fins seguros, por:

- `transponder_id`
- `service_id`

ou por uma chave equivalente estavel. Evite usar apenas `service_id`, `viewer_channel` ou ponteiro `Service*` em fluxos assincronos.

## Pontos de Atencao da Arquitetura Atual

- Eventos de canal ainda trafegam `Service*` em varios pontos, o que pode ser inseguro se o vetor de servicos for alterado.
- Alguns fluxos acessam `services[0]` ou `transponders[0]` sem validar lista vazia.
- O estado de satelite atual vem de `State_File`, `Config`, banco `sat`, lineup e UI; inconsistencias entre eles geram bugs de DiSEqC/regionalizacao.
- O CAS/Nagra pode solicitar reset de hardware via `CAK_EVT_CHIPSET_HARDWARE_RESET`.
- A pasta `doc/documentacao-mbgui/bugs` deve concentrar PRDs, analises e historico de bugs por release.

