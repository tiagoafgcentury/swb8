# HAL, Sintonia e DiSEqC

## Visão Geral

A camada HAL fica em `src/hal` e abstrai diferenças entre chipsets ALi e Montage. Ela integra tuner, demux, player, display, HDMI, remote control, DiSEqC e watchdog.

As tasks de maior contato com HAL são:

- `Task_Tuner`
- `Task_Demux`
- `Task_Player`
- `Task_Application`

## Fluxo Atual de Sintonia

O fluxo de lock em alto nível e:

1. UI/Player/Demux solicita lock de um `Transponder`.
2. `Task::post_event_transponder_lock()` entrega evento.
3. `Task_Tuner::handle_event_transponder_lock()` seleciona satélite/porta quando necessário.
4. HAL/Tuner deve aplicar DiSEqC, configurar LNB e tentar lock, mas a ordem real precisa ser conferida por chipset.
5. `Task_Tuner` posta `handle_event_transponder_locked`.
6. `Task_Demux` coleta tabelas DVB ou `Task_Player` inicia fluxo A/V.

## DiSEqC

DiSEqC ? parte crítica em multi-satélite. O comportamento desejado ? enviar o comando de porta antes do lock do transponder.

Estado observado no código atual:

- No HAL ALi, `Tuner::lock()` chama `aui_nim_connect()` e só depois chama `set_diseqc(_tp->satellite_id)`.
- No HAL Montage, o fluxo de lock não mostra aplicação equivalente de DiSEqC no caminho principal.
- Portanto, a frase "DiSEqC antes do lock" deve ser tratada como requisito técnico esperado, não como garantia comprovada pela release atual.

Dados relevantes:

- `sat.switch_type`
- `sat.switch_pos`
- `sat.id`
- `Transponder_Id::satellite_id()`
- `Config::get_diseqc_port()`
- `Config::get_diseqc_type()`

Pontos atuais de atenção:

- Fallback silencioso para satélite não encontrado pode mandar porta errada.
- Bugs de satélite novo/chave podem deixar `switch_pos` inconsistente.
- `Config::select_satellite_by_id()` e `Config::set_satellite_config_by_id()` devem sempre manter `m_current_satellite` coerente.
- Verificar a ordem real entre comando DiSEqC, configuração de LNB/22KHz e tentativa de lock em cada HAL antes de fechar bugs de "sem sinal".

## Tuner

Arquivos principais:

- `src/tasks/mb_task_tuner.cpp`
- `src/hal/ALi/mb_tuner.cpp`
- `src/hal/Montage/mb_tuner.cpp`

Riscos conhecidos:

- Operações de lock/blind scan podem ser demoradas.
- Esperas em nanossegundos no código atual indicam busy-wait e devem ser revisadas em correções de estabilidade.
- Não deve haver lock de transponder sem satélite/DiSEqC correto.
- A implementação ALi aplica DiSEqC após iniciar a conexão do tuner; isso pode ser incompatível com chaves que exigem seleção de porta antes do lock.

## Demux

Arquivos principais:

- `src/tasks/mb_task_demux.cpp`
- `src/mb_demux_lineup.cpp`
- `src/mb_demux_lineup_sky.cpp`
- `src/mb_demux_lineup_claro.cpp`
- `src/mb_demux_lineup_generic.cpp`

O build de lineup depende da subclasse correta para a política/operadora atual e do `current_satellite_id` correto.

Bug histórico importante:

- Reusar `m_demux_lineup` sem atualizar callback/satélite pode travar a tela de atualização de lista.

Estado atual:

- `Task_Demux::handle_event_lineup_build()` recria `m_demux_lineup` antes de escolher Sky/Claro/Generic e atualiza callback e `current_satellite_id`.
- Ainda assim, fluxos alternativos de demux devem ser revisados quando operador/satélite mudar durante uma busca.

## Watchdog

O watchdog está implementado por chipset:

- `src/hal/ALi/mb_watchdog.cpp`
- `src/hal/Montage/mb_watchdog.cpp`

Timeout observado no código:

- aproximadamente 10 segundos.

Como `wd.ping()` ocorre no loop principal, qualquer handler que bloqueie o loop por tempo suficiente pode causar reboot.

Fontes comuns de risco:

- deadlock em mutex de lineup
- IPC cheio
- espera indefinida por fila
- blind scan/lock chamado no contexto errado
- callback de UI fazendo operação pesada

## CAS/Nagra e Reset de Hardware

O CAS pode solicitar reset de hardware:

- `CAK_EVT_CHIPSET_HARDWARE_RESET`
- `Nagra::process()` seta flag de reset
- callback chama `post_event_system_need_reset()`
- OSD exibe contagem e encerra app para permitir reboot

Esse fluxo ? esperado em algumas condições CAS, mas deve ser logado com contexto completo para não parecer reboot aleatório.

## Telemetria Recomendada

Para diagnóstico de sintonia/DiSEqC, logar:

- `satellite_id`
- nome do satélite
- `switch_type`
- `switch_pos`
- frequência/polaridade/symbol rate
- resultado de lock
- tempo de lock
- origem do evento de lock
- network policy

Em bugs de regionalização, logar também:

- `network_id`
- `zone_id`
- `bouquet_id`
- operador atual
- origem do dado aplicado
