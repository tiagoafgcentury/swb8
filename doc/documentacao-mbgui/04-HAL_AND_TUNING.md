# 04 - HAL, Sintonia e DiSEqC

## Visao Geral

A camada HAL fica em `src/hal` e abstrai diferencas entre chipsets ALi e Montage. Ela integra tuner, demux, player, display, HDMI, remote control, DiSEqC e watchdog.

As tasks de maior contato com HAL sao:

- `Task_Tuner`
- `Task_Demux`
- `Task_Player`
- `Task_Application`

## Fluxo Atual de Sintonia

O fluxo de lock em alto nivel e:

1. UI/Player/Demux solicita lock de um `Transponder`.
2. `Task::post_event_transponder_lock()` entrega evento.
3. `Task_Tuner::handle_event_transponder_lock()` seleciona satelite/porta quando necessario.
4. HAL/Tuner aplica DiSEqC, configura LNB e tenta lock.
5. `Task_Tuner` posta `handle_event_transponder_locked`.
6. `Task_Demux` coleta tabelas DVB ou `Task_Player` inicia fluxo A/V.

## DiSEqC

DiSEqC e parte critica em multi-satelite. O comando de porta precisa ocorrer antes do lock do transponder.

Dados relevantes:

- `sat.switch_type`
- `sat.switch_pos`
- `sat.id`
- `Transponder_Id::satellite_id()`
- `Config::get_diseqc_port()`
- `Config::get_diseqc_type()`

Pontos atuais de atencao:

- Fallback silencioso para satelite nao encontrado pode mandar porta errada.
- Bugs de satelite novo/chave podem deixar `switch_pos` inconsistente.
- `Config::select_satellite_by_id()` e `Config::set_satellite_config_by_id()` devem sempre manter `m_current_satellite` coerente.

## Tuner

Arquivos principais:

- `src/tasks/mb_task_tuner.cpp`
- `src/hal/ALi/mb_tuner.cpp`
- `src/hal/Montage/mb_tuner.cpp`

Riscos conhecidos:

- Operacoes de lock/blind scan podem ser demoradas.
- Esperas em nanossegundos no codigo atual indicam busy-wait e devem ser revisadas em correcoes de estabilidade.
- Nao deve haver lock de transponder sem satelite/DiSEqC correto.

## Demux

Arquivos principais:

- `src/tasks/mb_task_demux.cpp`
- `src/mb_demux_lineup.cpp`
- `src/mb_demux_lineup_sky.cpp`
- `src/mb_demux_lineup_claro.cpp`
- `src/mb_demux_lineup_generic.cpp`

O build de lineup depende da subclasse correta para a politica/operadora atual e do `current_satellite_id` correto.

Bug historico importante:

- Reusar `m_demux_lineup` sem atualizar callback/satelite pode travar a tela de atualizacao de lista.

## Watchdog

O watchdog esta implementado por chipset:

- `src/hal/ALi/mb_watchdog.cpp`
- `src/hal/Montage/mb_watchdog.cpp`

Timeout observado no codigo:

- aproximadamente 10 segundos.

Como `wd.ping()` ocorre no loop principal, qualquer handler que bloqueie o loop por tempo suficiente pode causar reboot.

Fontes comuns de risco:

- deadlock em mutex de lineup
- IPC cheio
- espera indefinida por fila
- blind scan/lock chamado no contexto errado
- callback de UI fazendo operacao pesada

## CAS/Nagra e Reset de Hardware

O CAS pode solicitar reset de hardware:

- `CAK_EVT_CHIPSET_HARDWARE_RESET`
- `Nagra::process()` seta flag de reset
- callback chama `post_event_system_need_reset()`
- OSD exibe contagem e encerra app para permitir reboot

Esse fluxo e esperado em algumas condicoes CAS, mas deve ser logado com contexto completo para nao parecer reboot aleatorio.

## Telemetria Recomendada

Para diagnostico de sintonia/DiSEqC, logar:

- `satellite_id`
- nome do satelite
- `switch_type`
- `switch_pos`
- frequencia/polaridade/symbol rate
- resultado de lock
- tempo de lock
- origem do evento de lock
- network policy

Em bugs de regionalizacao, logar tambem:

- `network_id`
- `zone_id`
- `bouquet_id`
- operador atual
- origem do dado aplicado

