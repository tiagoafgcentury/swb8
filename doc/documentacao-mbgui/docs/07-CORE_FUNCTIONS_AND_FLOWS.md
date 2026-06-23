# Funções Principais e Fluxos Atuais

## Inicialização

Fluxo principal:

1. `main()` imprime versão/build.
2. Lê/grava variável de ambiente de firmware `software_version`.
3. Cria diretorios de storage/cache.
4. Inicializa `System`, `Watchdog` e tasks.
5. Habilita watchdog.
6. Entra no loop `Task::run_processes()`.

Arquivos:

- `src/mb_main.cpp`
- `src/tasks/mb_task.cpp`
- `src/tasks/mb_task_application.cpp`

## Encerramento, Restart e Reboot

Flags globais:

- `g_mbgui_keep_running`
- `g_mbgui_reboot_after_exit`
- `g_mbgui_restart_on_exit`
- `g_mbgui_do_factory_reset`

Fluxos:

- `SIGTERM`: encerra sem restart/reboot.
- `Task_Application::system_exit()`: encerra e solicita restart do app.
- CAS `system_need_reset`: OSD mostra contagem e encerra permitindo reboot.
- Factory reset done: seta `g_mbgui_do_factory_reset` e chama `system_exit()`.
- TPM `/system/reboot`: executa comandos de reboot.

## Carregamento de Lineup

Fluxo macro:

1. `Task_Application` solicita `post_event_lineup_load()`.
2. `Task_Database` lê `tp` e `srv`.
3. `Lineup` ? populado em memória.
4. `post_event_lineup_ready()` notifica tasks.
5. Player/OSD passam a usar serviço atual/listas.

Cuidados:

- `services` pode estar vazio.
- Evitar `services[0]` sem guarda.
- Evitar guardar `Service*` para uso assíncrono prolongado.

## Build/Atualização de Lineup

Fluxo usado por Instala Fácil e busca:

1. UI posta `post_event_lineup_build()`.
2. `Task_Demux::handle_event_lineup_build()` cria/reusa `m_demux_lineup`.
3. Demux coleta tabelas e monta lista.
4. Callback da UI deve ser chamado com progresso/finalização.

Pontos atuais importantes:

- Callback e `current_satellite_id` precisam ser atualizados a cada build.
- Se satélite/policy mudar, a subclasse de demux pode precisar ser recriada.
- UI deve ter timeout/cancelamento para não ficar presa se callback falhar.

## Criar e Editar Satélite

Fluxo atual:

1. UI abre `OSD_Edit_Satellite`.
2. Usuário altera nome, tipo LNBF, banda, polaridade ou chave.
3. Se `id == 0`, UI posta `post_event_add_satellite()`.
4. Se `id != 0`, UI posta `post_event_update_satellite()`.
5. Config recarrega lista de satélites.

Riscos:

- ID gerado para novo satélite não retorna imediatamente para UI.
- Segunda edição pode operar em ID inválido/antigo.
- Estado de DiSEqC da tela precisa ser resetado.
- Relock após update deve validar se há transponders.

## Sintonia e Zapping

Fluxo:

1. UI/Lineup solicita troca de canal.
2. `Task_Player::handle_event_channel_change()` recebe `Service*`.
3. Config seleciona satélite por ID do serviço.
4. Tuner tenta travar o transponder.
5. Demux coleta PMT se necessário.
6. CAS/Nagra descramble se canal protegido.
7. Player inicia A/V.

Cuidados:

- `Service*` aponta para item de vetor e pode ficar inválido se lineup mudar.
- Chave segura para canal e `transponder_id + service_id`.
- `service_id` isolado não ? suficiente.
- A ordem DiSEqC antes do lock deve ser validada por HAL; no caminho ALi atual, o comando aparece depois da chamada de conexão do tuner.

## Regionalização

Fluxos envolvidos:

- Lineup Sky/Claro/Generic.
- Zone ID / Bouquet ID.
- CAS/Nagra.
- `Task_Application::handle_event_zone_id_changed()`.
- `post_event_lineup_save_zone_id()`.
- Diagnóstico OSD.

Riscos:

- Estado persistido de operadora anterior.
- Cache/banco não invalidado em factory reset/troca de operadora.
- `network_id`, `network_policies`, `zone_id` e `bouquet_id` divergentes.

## Factory Reset

Fluxo atual observado:

1. evento de factory reset chega ao app/CAS.
2. `g_mbgui_do_factory_reset` ? setado ao concluir.
3. `main()` chama `erase_all_files()` e regrava `State_File`.

Ponto de atenção:

- `erase_all_files()` remove arquivos regulares em `MBGUI_CACHE_PATH`.
- Nem todo storage possivelmente usado por CAS/Nagra ou configuração fica necessariamente coberto.

## PVR e Agenda

Agenda/PVR deve resolver canal por chave estável. Em multi-satélite, qualquer uso de `service_id` isolado pode gravar ou lembrar canal errado.

Estado atual observado:

- A tabela `agenda` possui coluna `srv_id`, mas o código grava e carrega `ScheduleEntry::service_id` nela.
- Telas de agenda e lembrete procuram serviço por `service_id` isolado.
- `OSD_Program_Reminder::change_channel()` pode selecionar o primeiro canal com o mesmo `service_id`, mesmo que seja de outro transponder/satélite.
- Esse fluxo deve ser tratado como ponto de risco ativo da release, não apenas como recomendação futura.

Ao mexer nesse fluxo, validar:

- criacao de agenda
- edição
- disparo
- canal com `service_id` repetido em satélite diferente
- persistência em SQLite
- gravação/lembrete usando `ServiceKey` ou `srv_id` real de ponta a ponta

## Fluxo de Bugs e Releases

Documentação de bug deve ficar em:

```text
doc/documentacao-mbgui/bugs
```

Cada bug/release deve documentar:

- cenário reproduzido
- comportamento esperado
- comportamento atual
- hipóteses ligadas ao código
- arquivos envolvidos
- logs necessários
- critérios de aceite
- plano de teste de bancada
