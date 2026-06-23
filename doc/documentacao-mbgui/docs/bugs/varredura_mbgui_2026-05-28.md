# Varredura de bugs potenciais - mbgui

Data: 2026-05-28

Escopo analisado:
- Codigo principal em `src`, `ui`, `extern`, `extra` e arquivos de build relevantes.
- Nesta revisao, o escopo inclui tambem `src/cas`, `src/tpm` e `src/tasks/mb_task_cas.*`.
- Foram ignorados artefatos gerados da documentacao (`doc/documentacao-mbgui/site` e `site_build`) para evitar ruido.
- A varredura foi feita no ambiente local disponivel, sem uso de WSL.

Resumo da varredura:
- Arquivos considerados no escopo: 552.
- Ocorrencias de `while(true)`/`while(1)` no escopo: 16.
- Ocorrencias de `mb_assert(...)` no escopo: 349.
- Ocorrencias de chamadas de shell (`system`, `popen`, `exec_system_command`) no escopo principal/CAS/TPM/UI: 87.
- Ocorrencias de `cJSON_Parse` em TPM/CAS/tasks/UI avaliadas: 8.
- Ocorrencias diretas de `cJSON_Delete` nesses mesmos caminhos: 2.

## Achados prioritarios

### 1. Rotas TPM montam comandos shell com dados recebidos por HTTP

Severidade: Critica

Arquivos:
- `src/tpm/mb_tpm.cpp:229-262`
- `src/tpm/mb_tpm.cpp:384-455`
- `src/tpm/mb_tpm.cpp:1774-1861`
- `src/tpm/mb_tpm.cpp:2046-2060`

Evidencia:
- `update_hwcn` monta comando com valor vindo do JSON: `echo <value> > <path>`.
- `copy_bootloader` monta comandos com `files_path` recebido por JSON, incluindo `cp`, `cd`, `ls`, `sha256sum` e remocao de arquivos.
- `update_PK`, `update_CSC` e `update_FPK` montam comandos `dd`/`dumpimage` em buffers fixos usando caminho vindo da USB.
- `exec_system_command` executa string arbitraria via `popen`.

Impacto possivel:
- Command injection por parametro HTTP/USB.
- Escrita/leitura fora do diretorio esperado.
- Travamento ou corrupcao de sistema se comandos longos, malformados ou destrutivos forem montados.
- Crash se `popen` falhar e retornar `nullptr`, pois o codigo usa `fgets(..., pipe)` sem validar `pipe`.

Recomendacao:
- Remover shell sempre que possivel e usar APIs nativas de arquivo/processo.
- Validar parametros com allowlist estrita de caracteres e tamanho.
- Tratar caminhos com canonicalizacao e conferir se continuam dentro do diretorio permitido.
- Validar retorno de `popen` antes de ler.

### 2. TPM aceita upload HTTP sem limite efetivo de tamanho

Severidade: Alta

Arquivos:
- `src/tpm/mb_task_http_server.cpp:116-167`
- `src/tpm/mb_task_http_server.cpp:227-290`
- `src/tpm/mb_task_http_server.cpp:316-385`

Evidencia:
- As rotas acumulam `ctx->upload_data.append(upload_data, *upload_data_size)`.
- Existe `POSTBUFFERSIZE = 32 * 1024`, mas ele nao limita o tamanho total acumulado.
- O dado acumulado e repassado para parsers JSON e handlers sem teto global.

Impacto possivel:
- Uma requisicao POST grande pode consumir memoria e travar o processo.
- Varias requisicoes simultaneas podem amplificar o problema.
- JSON grande pode gerar alta latencia e fragmentacao de memoria.

Recomendacao:
- Definir limite maximo por rota.
- Rejeitar upload com HTTP 413 quando o acumulado passar do limite.
- Aplicar timeout/limite tambem no numero de conexoes abertas.

### 3. Parsers JSON do TPM vazam memoria por requisicao

Severidade: Alta

Arquivos:
- `src/tpm/mb_http_utils.cpp:65-99`
- `src/tpm/mb_http_utils.cpp:102-134`
- `src/tpm/mb_task_http_server.cpp:316-385`

Evidencia:
- `parse_json_int` chama `cJSON_Parse`, le campos e retorna sem `cJSON_Delete`.
- `parse_json` faz o mesmo para strings.
- `rc_key_send` chama `cJSON_Parse` diretamente e tambem nao libera o objeto raiz.

Impacto possivel:
- Vazamento por request HTTP.
- Em uso continuo, o servidor TPM pode degradar ate travar ou ser encerrado por falta de memoria.

Recomendacao:
- Encapsular `cJSON*` em RAII ou garantir `cJSON_Delete(root)` em todos os caminhos.
- Adicionar testes/valgrind/sanitizer para rotas HTTP mais usadas.

### 4. TPM nao valida falha ao iniciar servidor HTTP

Severidade: Alta

Arquivo:
- `src/tpm/mb_task_http_server.cpp:68-77`

Evidencia:
- O retorno de `MHD_start_daemon` e guardado, mas nao ha verificacao de `nullptr`.
- O objeto pode continuar vivo como se o servidor estivesse ativo.

Impacto possivel:
- Falha silenciosa do HTTP TPM.
- Outras partes do sistema podem assumir que o canal de controle esta disponivel quando nao esta.

Recomendacao:
- Verificar `m_daemon == nullptr`.
- Logar erro claro, sinalizar falha de inicializacao e impedir uso parcial.

### 5. Rotas TPM de reboot, poweroff, formatacao e update parecem sem autenticacao local

Severidade: Critica

Arquivos:
- `src/tpm/mb_tpm.cpp:275-289`
- `src/tpm/mb_tpm.cpp:524-538`
- `src/tpm/mb_tpm.cpp:1060-1111`
- `src/tpm/mb_tpm.cpp:1774-1861`

Evidencia:
- Existem handlers HTTP para reiniciar, desligar, formatar particoes e atualizar areas sensiveis.
- Na camada inspecionada nao apareceu verificacao de autenticacao/autorizacao antes da execucao.

Impacto possivel:
- Qualquer cliente com acesso ao endpoint pode derrubar o aparelho ou acionar operacoes destrutivas.
- Pode haver travamento por reboot/poweroff remoto inesperado.

Recomendacao:
- Centralizar autenticacao/autorizacao antes de handlers destrutivos.
- Restringir origem, metodo e token.
- Adicionar logs auditaveis e confirmacao em operacoes irreversiveis.

### 6. Acesso USB no TPM permite inconsistencias de path e excecoes nao tratadas

Severidade: Alta

Arquivos:
- `src/tpm/mb_tpm.cpp:1366-1510`
- `src/tpm/mb_tpm.cpp:1512-1575`
- `src/tpm/mb_tpm.cpp:1641-1649`

Evidencia:
- `get_path_by_device_id` acessa `args[2]`; URL curta pode causar acesso fora do vetor.
- Operacoes de arquivo usam o ultimo segmento da URL como nome, sem sanitizacao suficiente de `..`, nome vazio, barras codificadas ou caracteres especiais.
- `recursive_directory_iterator(path)` e usado sem `std::error_code` e pode lancar excecao se o diretorio sumir, estiver inacessivel ou contiver links problematicos.

Impacto possivel:
- Crash em request malformado.
- Falha intermitente quando USB e removida durante listagem.
- Escrita/delecao fora do alvo esperado dependendo de como o path final for resolvido.

Recomendacao:
- Validar quantidade de segmentos antes de acessar indices.
- Normalizar e restringir paths ao mount point.
- Usar APIs com `std::error_code` e responder erro HTTP em vez de propagar excecao.

### 7. `mb_assert` nao interrompe a execucao e some em release

Severidade: Alta

Arquivos:
- `src/common/mb_assert.h:6-10`
- `src/cas/nagra/mb_nagra.cpp:61`
- `src/tasks/mb_task_database.cpp:27-43`
- `src/tasks/mb_task.cpp:195-270`

Evidencia:
- Em debug, `mb_assert` apenas imprime no `stderr`.
- Em release (`NDEBUG`), o macro vira bloco vazio.
- Varias rotinas usam `mb_assert(false)` ou `mb_assert(condicao)` como barreira de erro.
- No CAS/Nagra, `NAGRA_EXEC` depende de `mb_assert(CA_NO_ERROR == ret)`.

Impacto possivel:
- Em release, erros de CA, SQLite, demux, IPC e parsing podem seguir adiante como se nada tivesse acontecido.
- Isso pode gerar crash posterior, dados corrompidos, commit parcial ou comportamento inconsistente dificil de reproduzir.

Recomendacao:
- Separar invariantes de debug de validacoes de runtime.
- Para erro recuperavel, retornar falha e abortar o fluxo atual.
- Para erro irrecuperavel, usar caminho explicito de parada/restart controlado.

### 8. CAS/Nagra pode ficar em loop infinito se o descritor de evento falhar

Severidade: Alta

Arquivo:
- `src/cas/nagra/mb_nagra.cpp:94-125`

Evidencia:
- `nagra_get_char` roda `while(true)` e chama `poll`.
- Se `poll` falhar, o codigo loga erro e continua.
- Nao ha limite de tentativas, backoff relevante ou saida por encerramento.

Impacto possivel:
- Thread presa em loop de erro.
- Alto volume de log.
- Travamento no desligamento se o descritor for fechado enquanto o loop ainda esta ativo.

Recomendacao:
- Encerrar o loop em erros permanentes (`EBADF`, descritor fechado, flag de shutdown).
- Usar backoff e sinal de parada.
- Testar fluxo de destruicao/desligamento do CAS.

### 9. Callback assincrono do Nagra usa singleton sem checagem de vida

Severidade: Alta

Arquivos:
- `src/cas/nagra/mb_nagra.cpp:500-525`
- `src/cas/nagra/mb_nagra.cpp:1601-1605`
- `src/tasks/mb_task_cas.cpp:13-38`

Evidencia:
- O construtor usa `mb_assert(s_instance == nullptr)` para proteger singleton.
- Em release, singleton duplicado nao e impedido por essa assertiva.
- `request_is_ready_callback` acessa `s_instance` sem checar se ele ainda existe.
- `Task_CAS` configura callbacks que podem continuar chegando de camadas assincronas.

Impacto possivel:
- Crash por callback chegando depois do destrutor.
- Evento direcionado a instancia errada em cenarios de reinicializacao.
- Estado CAS inconsistente apos factory reset ou reinicio parcial.

Recomendacao:
- Trocar singleton cru por ownership explicito ou registro/desregistro seguro de callbacks.
- Checar `s_instance` e estado de shutdown antes de usar.
- Garantir que callbacks nativos sejam cancelados antes da destruicao.

### 10. Requests CA podem vazar em caminhos de erro

Severidade: Media/Alta

Arquivo:
- `src/cas/nagra/mb_nagra.cpp:1607-1645`

Evidencia:
- `send_generic_nagra_request` cria `caRequest`.
- Em alguns erros depois da criacao/configuracao, a funcao retorna sem chamar `caRequestDispose(request)`.

Impacto possivel:
- Vazamento de objetos CA.
- Degradacao ao longo do tempo em operacoes CAS repetidas.

Recomendacao:
- Encapsular request CA em RAII com deleter apropriado.
- Garantir dispose em todos os retornos de erro.

### 11. Fila de eventos CAS/Nagra nao tem limite aparente

Severidade: Media/Alta

Arquivo:
- `src/cas/nagra/mb_nagra.cpp:706-715`

Evidencia:
- `push_event` insere em `m_events` sem limite visivel.
- Eventos CAK/CA podem chegar em rajadas.

Impacto possivel:
- Crescimento de memoria se o produtor superar o consumidor.
- Latencia crescente e sensacao de travamento.

Recomendacao:
- Definir limite e politica de descarte/coalescencia para eventos repetidos.
- Medir backlog e logar quando aproximar do limite.

### 12. CAS/PVR usa copias de string sem limite confiavel

Severidade: Alta

Arquivos:
- `src/cas/nagra/mb_nagra.cpp:2348-2405`
- `src/cas/nagra/mb_pvr.cpp:159-165`
- `src/cas/nagra/mb_pvr.cpp:249-252`
- `src/cas/nagra/mb_pvr.cpp:461-474`

Evidencia:
- `request_cas_pvr_play_start` usa `sprintf(buf, "%s", url.c_str())` em `char buf[255]`.
- `pvr_record_open` usa `sprintf(st_arp.folder_name, "/%s", param->filename.c_str())`.
- `pvr_play_open` usa `STRCPY` para copiar path.
- `strncpy` em mount name pode nao terminar com `\0` se o tamanho for exatamente o limite.

Impacto possivel:
- Overflow de buffer com URL/nome de arquivo grande.
- Corrupcao de memoria e crash em operacoes PVR.
- Nome sem terminador causando leitura alem do buffer.

Recomendacao:
- Usar `snprintf` com checagem de truncamento.
- Validar tamanho maximo de filename/URL antes de chamar HAL/CA.
- Garantir terminador nulo apos `strncpy`.

### 13. ACS reencrypt copia lista de PIDs sem validar quantidade

Severidade: Alta

Arquivo:
- `src/cas/nagra/mb_acs_reencrypt.cpp:363-365`
- `src/cas/nagra/mb_acs_reencrypt.cpp:621-624`

Evidencia:
- O codigo faz `memcpy(..., sizeof(...) * _p_param->pid_num)`.
- Nao apareceu checagem de `pid_num` contra a capacidade do array destino.

Impacto possivel:
- Overflow de buffer se `pid_num` vier maior que o esperado.
- Corrupcao de memoria em fluxos de reencrypt.

Recomendacao:
- Validar `pid_num` antes do `memcpy`.
- Usar `std::array`/span quando possivel ou copiar com limite explicito.

### 14. Eventos temporizados podem ser perdidos para sempre

Severidade: Alta

Arquivo:
- `src/tasks/mb_task.cpp:713-790`
- `src/tasks/mb_task.cpp:806-827`

Evidencia:
- Eventos postados durante o processamento entram em `s_timed_events_alt`.
- O merge de `s_timed_events_alt` para `s_timed_events` so acontece dentro de `if(!s_timed_events.empty())`.
- Se nao houver evento temporizado ativo no momento, eventos novos podem ficar presos em `s_timed_events_alt`.

Impacto possivel:
- Timer que nunca dispara.
- Fluxos de UI, CAS, aplicacao ou banco esperando timeout/evento que nao chega.
- Sintoma de travamento intermitente sem crash.

Recomendacao:
- Sempre mesclar `s_timed_events_alt` antes de testar se a lista principal esta vazia.
- Adicionar teste unitario cobrindo postagem de timer enquanto a lista principal esta vazia.

### 15. Message queue faz busy wait sem timeout real

Severidade: Alta

Arquivo:
- `src/tasks/private/mb_task_mqueue_helpers.cpp:31-45`
- `src/tasks/private/mb_task_mqueue_helpers.cpp:58-83`

Evidencia:
- `open_queue()` roda `while(true)` ate conseguir abrir a fila.
- O sleep e de `std::chrono::nanoseconds(50)`, muito pequeno para backoff real.
- Em erro, a assertiva `mb_assert(result == -1)` parece invertida e nao protege em release.

Impacto possivel:
- CPU alta se a fila nao aparecer.
- Thread presa para sempre em inicializacao ou reconexao.
- Erros de IPC mascarados em release.

Recomendacao:
- Usar timeout configuravel e backoff progressivo.
- Retornar erro claro para o chamador.
- Corrigir assertiva e logar `errno`.

### 16. `Lineup_Mutex_Ref` pode esconder deadlock em release

Severidade: Media/Alta

Arquivo:
- `src/common/mb_lineup.cpp:348-365`

Evidencia:
- Em debug, tenta `try_lock_for(10s)` e faz `mb_assert(false)` se falhar.
- Mesmo apos falha, marca `m_is_locked = true`.
- Em release, usa `lock()` sem timeout.

Impacto possivel:
- Deadlock real vira travamento sem diagnostico em release.
- Destrutor pode tentar unlock em estado inconsistente se a logica de debug falhar.

Recomendacao:
- Se `try_lock_for` falhar, nao marcar como locked.
- Padronizar timeout/log tambem em release ou isolar lock bloqueante apenas onde for inevitavel.

### 17. Timers LVGL podem sobreviver ao dono

Severidade: Alta

Arquivos:
- `ui/lvgl/mb_osd_select_switch.cpp:37-44`
- `ui/lvgl/mb_osd_select_switch.cpp:700-708`
- `ui/lvgl/mb_osd_select_switch.h:149-152`
- `src/tasks/mb_task_application.cpp:106-110`
- `src/tasks/mb_task_application.cpp:718-721`
- `src/tasks/mb_task_application.h:60`

Evidencia:
- `Select_Switch` deleta `m_refresh_timer`, mas nao deleta `m_detection_timer`.
- Callback de timer captura/usa `this`.
- `Task_Application` cria `m_tmr_evt`, mas o destrutor nao o remove.

Impacto possivel:
- Use-after-free quando timer dispara depois da destruicao do objeto.
- Crash ou comportamento aleatorio na UI.
- Vazamento de timer em troca de telas ou reinicializacao parcial.

Recomendacao:
- Cancelar/deletar todos os timers no destrutor.
- Zerar ponteiros apos delete.
- Preferir wrappers RAII para timers LVGL.

### 18. Parser de lineup limpa estado antes de validar JSON

Severidade: Alta

Arquivo:
- `src/tasks/mb_task_application.cpp:929-1022`

Evidencia:
- `parse_json` limpa a lineup antes de validar completamente o payload.
- O codigo acessa campos `cJSON` sem checagem consistente de tipo/nulo.

Impacto possivel:
- Payload corrompido pode apagar canais antes de falhar.
- Crash por ponteiro nulo ou tipo inesperado.
- Estado persistido pode ficar inconsistente.

Recomendacao:
- Parsear e validar em estrutura temporaria.
- So substituir a lineup atual apos validacao completa.
- Rejeitar payload com erro sem mutar estado.

### 19. Arquivo de estado tem campo nao inicializado e vazamento de FILE*

Severidade: Media/Alta

Arquivo:
- `src/common/mb_state_file.h:19-60`
- `src/common/mb_state_file.h:78-95`
- `src/common/mb_state_file.h:119`

Evidencia:
- `current_channel` nao aparece inicializado no construtor.
- Alguns caminhos de erro de `fread`/`fwrite` retornam antes de `fclose`.

Impacto possivel:
- Canal inicial aleatorio.
- Vazamento de descritor se erro de leitura/escrita se repetir.
- Estado salvo/lido de forma inconsistente.

Recomendacao:
- Inicializar todos os campos.
- Usar RAII para `FILE*`.
- Validar tamanho lido/escrito e versionar o arquivo de estado.

### 20. Transacoes SQLite podem seguir apos erro

Severidade: Alta

Arquivo:
- `src/tasks/mb_task_database.cpp:27-43`
- `src/tasks/mb_task_database.cpp:561-715`
- `src/tasks/mb_task_database.cpp:1005-1076`

Evidencia:
- `SQL_EXEC` apenas loga erro e chama `mb_assert(false)`.
- Em release, a assertiva some e o fluxo pode continuar.
- Ha transacoes longas e multiplos statements dependentes.

Impacto possivel:
- Commit parcial.
- Dados duplicados/incompletos.
- Uso de statement invalido depois de falha.

Recomendacao:
- Fazer macros/funcoes retornarem erro e abortarem transacao.
- Centralizar rollback em falhas.
- Testar falhas de `sqlite3_prepare`, `step`, `bind` e `exec`.

### 21. ALi media player copia URL sem limite

Severidade: Alta

Arquivo:
- `src/hal/ALi/mb_media_player.cpp:185-200`

Evidencia:
- `strcpy((char *)m_p->attr_mp.uc_file_name, url.c_str())`.
- Nao ha validacao visivel do tamanho de `url` contra o buffer de destino.

Impacto possivel:
- Overflow se URL/path exceder o buffer da HAL.
- Crash ao abrir midia ou arquivo grande.

Recomendacao:
- Usar copia limitada e checar truncamento.
- Definir limite maximo de URL aceito no nivel de API.

### 22. REST server pode deixar thread viva em alguns fluxos de parada

Severidade: Media

Arquivo:
- `src/mb_rest_server.cpp:47-59`

Evidencia:
- `stop()` faz join da thread apenas dentro de `if(m_server)`.
- Se `m_server` estiver nulo mas a thread tiver sido criada, o join pode nao acontecer.

Impacto possivel:
- Thread pendurada no encerramento.
- Travamento no shutdown ou destrutor.

Recomendacao:
- Separar controle do servidor e controle da thread.
- Sempre verificar `m_thread.joinable()` no caminho de parada.

### 23. Parser de termos usa schema inconsistente e vaza JSON

Severidade: Media

Arquivo:
- `ui/lvgl/mb_terms_of_use.cpp:30-89`

Evidencia:
- O codigo verifica `cJSON_IsArray(terms)`, mas depois acessa `terms` como objeto por idioma (`cJSON_GetObjectItemCaseSensitive(terms, lang)`).
- O objeto retornado por `cJSON_Parse` nao e liberado no fluxo analisado.

Impacto possivel:
- Termos de uso podem falhar silenciosamente ou carregar texto errado.
- Vazamento de memoria ao recarregar termos.

Recomendacao:
- Definir schema esperado: array ou objeto por idioma.
- Corrigir validacao.
- Liberar raiz com `cJSON_Delete`.

### 24. `Task_CAS` faz flush de processos por numero fixo de iteracoes

Severidade: Media

Arquivo:
- `src/tasks/mb_task_cas.cpp:169-208`
- `src/tasks/mb_task_cas.cpp:212-229`

Evidencia:
- Em fluxos de saida/factory reset, o codigo chama `Task::run_processes()` em loop fixo (`MBGUI_SOCKET_MAX_MESSAGES + 5`).
- Nao ha confirmacao explicita de que todas as mensagens necessarias foram realmente processadas/enviadas.

Impacto possivel:
- Evento CAS ou IPC pode ficar pendente em carga alta.
- Reset/exit pode acontecer com estado parcialmente propagado.

Recomendacao:
- Usar ACK/estado observavel para confirmar esvaziamento.
- Trocar loop fixo por timeout com condicao real de conclusao.

### 25. Loops infinitos precisam de contrato de shutdown

Severidade: Media

Arquivos:
- `src/cas/nagra/mb_nagra.cpp:94-125`
- `src/tasks/private/mb_task_mqueue_helpers.cpp:31-45`
- `src/tpm/tpm_init.c:913`
- `src/cas/nagra/dpt/ca_dpt.c:2217-2225`

Evidencia:
- Ha loops `while(true)`/`while(1)` usados tanto para espera real quanto como bloco tecnico com `break`.
- Os loops de espera precisam de sinal de parada, timeout e tratamento de erro permanente.

Impacto possivel:
- Travamento no desligamento.
- CPU alta em falhas permanentes.
- Dificuldade de diagnostico quando uma dependencia externa nao responde.

Recomendacao:
- Classificar cada loop como: bloco tecnico, loop de servico ou espera de dependencia.
- Para loops de servico/espera, exigir flag de shutdown, timeout e log com throttling.

## Pontos que merecem revisao manual adicional

- `src/cas/nagra/mb_nagra.cpp:749-819`: uso de `m_pvr` em callbacks/fluxos CAS deve ser revisado contra o modelo de thread; nao ficou claro se todos os acessos sao serializados.
- `src/cas/nagra/mb_nagra.cpp:1647-1676`: revisar ownership dos objetos retornados por `caRequestGetObjects`; se a API exigir liberacao explicita, pode haver vazamento adicional.
- `src/tpm/tpm_init.c`: ha varios caminhos de inicializacao de baixo nivel; vale auditar retornos de HAL/NIM e contratos de timeout com o hardware.
- `extern` e bibliotecas de terceiros nao foram tratadas como codigo de propriedade do projeto, exceto quando chamadas pelo codigo principal.

## Recomendacao de ordem de correcao

1. Bloquear command injection e rotas destrutivas TPM sem autenticacao.
2. Adicionar limites de upload e corrigir vazamentos de JSON no TPM.
3. Corrigir callbacks/singleton/loops de shutdown do CAS/Nagra.
4. Remover copias inseguras (`sprintf`, `strcpy`, `STRCPY`) em CAS/PVR/HAL.
5. Corrigir timers LVGL e eventos temporizados.
6. Trocar `mb_assert` usado como tratamento de erro por validacao real de runtime.
7. Reforcar transacoes SQLite, state file e parser de lineup.

## Observacao final

Os achados acima priorizam riscos de travamento, inconsistencia de estado, crash, vazamento de memoria e execucao indevida de comandos. Alguns pontos, especialmente CAS/Nagra e TPM, dependem de contrato com SDK/HAL externo; por isso, onde o ownership da API nao ficou evidente, o item foi marcado como revisao manual adicional em vez de bug conclusivo.
