# Standby - Riscos e Checklist

## Pontos de atencao

### Estado default em arquivo

`State_File::App_State_File` define `stand_by` com default `true`.

Impacto:

- arquivo ausente, corrompido ou mal lido pode fazer o objeto nascer assumindo standby
- um primeiro boot ou falha de persistencia pode parecer entrada indevida em espera

Ao investigar:

- confirmar se o arquivo de estado existe
- confirmar se leitura/escrita terminou sem erro
- registrar valores de `stand_by` e `stand_by_in_production_mode`

### Fonte de verdade da transicao

`System::stand_by()` decide a transicao olhando o estado salvo em arquivo, nao apenas um estado RAM local.

Impacto:

- divergencia entre arquivo, hardware e expectativa do usuario pode parecer alternancia invertida
- o comportamento sobrevive ao reinicio da aplicacao, mas fica dependente da persistencia

Ao investigar:

- comparar estado antes/depois de `System::stand_by()`
- registrar se o fluxo chamou `fake_power_off()` ou `fake_power_on()`

### Retorno depende de canal e transponder

O retorno do standby depende de `handle_event_application_state_loaded()` encontrar canal e transponder corretos.

Impacto:

- se o canal salvo nao existir mais, o backend minimo pode nao ser restaurado
- se o lineup mudou, o retorno visual pode ocorrer, mas EMM/CAS/OTA podem ficar degradados

Ao investigar:

- validar `current_channel`
- validar `current_satellite_id`
- confirmar se o canal salvo existe no lineup atual
- confirmar se o transponder foi resolvido e travado

### Standby e novo boot do app

O fluxo mistura "retorno do standby" com "novo boot do app em estado persistido".

Impacto:

- problemas de retorno podem ser problemas de startup
- testes precisam observar inicializacao, restauracao de contexto e reexecucao de tasks

Ao investigar:

- separar falha visual de falha de backend
- verificar logs desde o inicio do processo, nao apenas a tecla de retorno

### Fake standby, PMU standby e poweroff real

Existem caminhos diferentes no repositorio:

- fake standby do MBGUI
- standby/power de baixo nivel via TPM/AUI
- poweroff real por endpoint HTTP

Impacto:

- discussoes de produto, QA e suporte podem misturar comportamentos diferentes
- consumo, wakeup, persistencia e retorno podem variar conforme o caminho usado

## Checklist de reproducao

### Entrada por POWER

- Pressionar `POWER` sem media player/PVR ativo.
- Confirmar publicacao de `post_event_toggle_power()`.
- Confirmar entrada em `Task_Application::handle_event_toggle_power()`.
- Confirmar chamada de `System::stand_by()`.
- Confirmar `State_File` atualizado.
- Confirmar `system_exit()` e reinicio do app.

### Entrada por sleep timer

- Configurar sleep timer pela UI.
- Confirmar criacao do `lv_timer`.
- Confirmar decremento ate zero.
- Confirmar chamada de `OSD_Menu_Plus_Sleep::start_standby()`.
- Confirmar convergencia para `post_event_toggle_power()`.

### Retorno do standby

- Confirmar leitura de `State_File::App_State_File`.
- Confirmar chamada de `Task_Application::is_in_stand_by()`.
- Confirmar transicao para `ST_STAND_BY_MODE` quando aplicavel.
- Confirmar `System::check_standby_mode()`.
- Confirmar `post_event_application_state_load()`.
- Confirmar restauracao de volume, mute, canal e satelite.
- Confirmar tentativa de lock do transponder.
- Confirmar inicio de EMM/CAS quando aplicavel.

### OTA e backend

- Confirmar se `Task_Demux::process()` identifica standby.
- Confirmar ciclo aproximado de checagem de OTA.
- Confirmar se `check_for_otas()` foi chamado.
- Confirmar se o fluxo Sky/operadora aplicavel travou transponder de referencia.

## Perguntas para bug report

- O receptor entrou em standby por POWER, painel frontal, sleep timer ou outro gatilho?
- O comportamento observado foi fake standby ou poweroff real?
- O app reiniciou?
- O receptor inteiro reiniciou?
- O estado salvo indicava standby antes do evento?
- O estado salvo indicava standby depois do evento?
- O ultimo canal salvo ainda existe no lineup?
- O transponder do ultimo canal foi encontrado?
- Houve lock de transponder no retorno?
- Houve filtragem EMM/CAS?
- Houve checagem OTA?

## Recorte recomendado para futuras analises

Se o processo precisar ser aprofundado, dividir a investigacao nestes blocos:

1. entrada por controle remoto e painel frontal
2. persistencia de estado e reinicio do app
3. retorno operacional em `Task_Application` + `Task_Demux`
4. diferenca entre fake standby, PMU standby e poweroff real

