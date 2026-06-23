# PRD - Correcao de bouquet segment SKY apos transicao Claro para SKY

Data: 2026-06-08

## Resumo

Durante a transicao do receptor do satelite Claro para o satelite SKY, o CAK pode atualizar o zipcode antes de atualizar o bouquet segment. O MBGUI atual reage ao primeiro callback de smartcard que contem zipcode SKY e usa imediatamente o segmento retornado naquele momento. Quando o segmento ainda e o antigo da Claro, a lista de canais SKY e reconstruida para a zona errada.

O comportamento observado em campo e:

- ativacao inicial em Claro retorna zipcode Claro e segmento Claro;
- ao migrar para SKY, o zipcode muda primeiro;
- o segmento permanece temporariamente com valor antigo;
- o MBGUI salva esse valor intermediario e dispara rebuild da lista SKY;
- alguns minutos depois o segmento correto chega e a lista e reconstruida novamente.

## Impacto

- Lista de canais SKY pode ser montada com bouquet/regiao incorreta.
- Usuario pode ver canais de uma zona diferente da selecionada/contratada.
- O problema persiste mesmo apos reboot imediato, porque a primeira leitura apos boot ainda pode receber o segmento antigo.
- A lista so se corrige depois que o CAK recebe/processa a EMM correta do segmento.

## Evidencia do problema

Leitura correta em Claro:

```text
Got Seg Code: 56 0 '02790000000000000000000000000000000000000000000000000000000000000000000c0004000200000004000000000000000000000000'
Got Zip Code: 6 0 '000000000079'
```

Leitura intermediaria apos transicao para SKY:

```text
Got Seg Code: 56 0 '02790000000000000000000000000000000000000000000000000000000000000000000c0004000200000004000000000000000000000000'
Got Zip Code: 6 0 '051434ef7d34'
```

Leitura correta apos alguns minutos:

```text
Got Seg Code: 56 0 '02800000000000000000000000000000000000000000000000000000000000000000000c0004000200000004000000000000000000000000'
Got Zip Code: 6 0 '051434ef7d34'
```

## Causa provavel

O fornecedor CAS informou que zipcode e bouquet segment podem ser atualizados em ciclos diferentes de callback, pois o headend envia diferentes EMMs. O MBGUI atual trata o primeiro callback com zipcode SKY como estado final e dispara a persistencia/rebuild antes de confirmar que o segmento SKY tambem foi atualizado.

Fluxo atual no MBGUI:

1. `CAK_EVT_SMARTCARDS` e processado em `src/cas/nagra/mb_nagra.cpp`.
2. `Nagra::check_smartcard()` le `caSmartcardGetZipCode()` e `caSmartcardGetSegments()`.
3. Se `zip_code[0]` existe, o codigo assume SKY.
4. O bouquet SKY e calculado a partir dos 10 primeiros bits do segmento.
5. `Task::post_event_lineup_save_zone_id()` e chamado imediatamente.
6. `Zone_ID::set_zone_id()` dispara `handle_event_zone_id_changed()`.
7. Para SKY, o OSD inicia `OSD_Channel_List_Update` e a lista e reconstruida.

## Comparacao com EitvCaMain

Foi comparado o diretorio `EitvCaMain`, informado como uma implementacao que nao apresenta o mesmo problema.

Principais diferencas:

- `EitvCaMain` registra `CA_LISTENER_TYPE_SMARTCARDS` e `CA_LISTENER_TYPE_PURCHASE_HISTORY`.
- O callback `smartcardNotificationCb()` nao le e notifica zipcode/segmento imediatamente.
- O callback reinicia uma thread de notificacao de regiao.
- A thread aguarda antes de ler o smartcard:
  - 30 segundos no fluxo normal;
  - 5 segundos quando `purchaseHistoryNotificationCb()` marca `shouldSendSegmentation = true`.
- Somente depois do atraso chama `fillSmartcardInformation()`.
- `fillSmartcardInformation()` le `caSmartcardGetZipCode()` e `caSmartcardGetSegments()`.
- Depois envia `ZipCodeNotification` e `SegmentBouquetNotification`.

Conclusao: o `EitvCaMain` possui um debounce temporal antes de propagar zipcode/segmento. Esse atraso evita que a aplicacao reaja ao estado parcial onde zipcode ja mudou, mas segmento ainda nao.

## Arquivos envolvidos no MBGUI

- `src/cas/nagra/mb_nagra.cpp`
  - `Nagra::process_ready_events()`
  - `Nagra::check_smartcard()`
  - `Nagra::check_for_smatcards()`
- `src/cas/nagra/mb_nagra.h`
  - estado interno para debounce/confirmacao de segmento SKY
- `src/common/mb_types.h`
  - `Zone_ID_t` atualmente e `uint8_t`
- `src/mb_zone_id.cpp`
  - persistencia de zone id Claro/SKY em `zone.id`
- `src/tasks/mb_task_osd.cpp`
  - rebuild de lista SKY em `handle_event_zone_id_changed()`
- `src/mb_demux_lineup.cpp`
  - uso de `BOUQUET_ID + my_zone_id` para filtrar BAT SKY

## Requisitos funcionais

### RF-01 - Nao reconstruir lista SKY com segmento parcial

O MBGUI nao deve salvar o zone/bouquet SKY nem reconstruir a lista de canais enquanto houver indicio de que o callback CAS representa estado parcial de transicao.

### RF-02 - Usar `PURCHASE_HISTORY` como gatilho preferencial para SKY

Quando `CAK_EVT_PURCHASE_HISTORY` for recebido, o MBGUI deve tratar esse evento como indicio de que a atualizacao de direitos/EMM avancou e deve reler o smartcard apos curto atraso.

### RF-03 - Aplicar debounce para leituras de smartcard SKY

Para SKY, a leitura de zipcode/segmento deve ser confirmada antes de chamar `Task::post_event_lineup_save_zone_id()`.

Politicas aceitas:

- esperar alguns segundos apos `PURCHASE_HISTORY` e entao reler;
- exigir duas leituras consecutivas iguais do mesmo segmento SKY;
- combinar as duas abordagens.

### RF-04 - Preservar comportamento Claro

O fluxo Claro, baseado em zipcode, nao deve sofrer atraso desnecessario nem regressao de ativacao.

### RF-05 - Evitar notificacoes duplicadas

Se a leitura confirmada retornar o mesmo operador e zone/bouquet ja salvo, o MBGUI nao deve disparar novo rebuild da lista.

## Requisitos tecnicos

### RT-01 - Separar leitura bruta de notificacao da aplicacao

Refatorar `Nagra::check_smartcard()` para separar:

- leitura de zipcode/segmento/vUA;
- decisao de operador e zone/bouquet;
- validacao/debounce;
- notificacao via `Task::post_event_lineup_save_zone_id()`.

### RT-02 - Adicionar estado de debounce SKY

Adicionar estado interno em `Nagra`, por exemplo:

```cpp
std::optional<Zone_ID_t> m_pending_sky_zone_id;
uint8_t m_pending_sky_confirmations = 0;
std::chrono::steady_clock::time_point m_last_sky_purchase_history;
bool m_sky_waiting_purchase_history = false;
```

Os nomes finais podem variar, mas o comportamento deve ser rastreavel por logs.

### RT-03 - Corrigir comparacao com `zone.id`

O codigo atual compara byte bruto do arquivo com `segment_code[1]`. Para SKY, a comparacao deve ser feita contra o bouquet/city code calculado, nao contra byte bruto do segmento.

### RT-04 - Rever largura de `Zone_ID_t`

O bouquet SKY usa 10 bits. `Zone_ID_t` atualmente e `uint8_t`, o que pode truncar valores acima de 255.

Avaliar uma destas opcoes:

- alterar `Zone_ID_t` para `uint16_t`;
- criar tipo separado para bouquet SKY;
- manter compatibilidade do arquivo `zone.id` com migracao de formato.

Se a mudanca de tipo for considerada arriscada para esta release, documentar a limitacao e garantir ao menos que os valores usados atualmente nao passem de 255.

### RT-05 - Logs de diagnostico

Adicionar logs para:

- evento que iniciou a leitura (`SMARTCARDS`, `PURCHASE_HISTORY`, leitura agendada);
- zipcode bruto;
- segmento bruto;
- bouquet calculado;
- decisao tomada: pendente, confirmado, ignorado por duplicidade ou invalido;
- tempo entre `PURCHASE_HISTORY` e leitura confirmada.

## Proposta de solucao

Implementar no MBGUI um fluxo semelhante ao `EitvCaMain`, mas adaptado ao modelo de eventos existente.

### Fluxo proposto para SKY

1. Ao receber `CAK_EVT_SMARTCARDS`, ler smartcard somente para diagnostico ou iniciar debounce.
2. Se for detectado zipcode SKY, calcular bouquet a partir do segmento.
3. Se houver transicao recente Claro -> SKY, nao salvar imediatamente.
4. Marcar leitura como pendente.
5. Ao receber `CAK_EVT_PURCHASE_HISTORY`, agendar nova leitura de smartcard apos aproximadamente 5 segundos.
6. Na leitura agendada, confirmar o bouquet.
7. Se o bouquet for diferente do valor salvo e valido, chamar `Task::post_event_lineup_save_zone_id(Satellite_Operator::Sky, bouquet)`.
8. O rebuild da lista SKY acontece somente apos a confirmacao.

### Fluxo proposto para Claro

1. Se `zip_code[0] == 0`, manter regra atual de Claro.
2. Usar o ultimo byte do zipcode como zone id.
3. Postar alteracao somente se o valor mudou.

## Criterios de aceite

- Ao migrar de Claro para SKY, o MBGUI nao deve reconstruir a lista usando segmento antigo da Claro.
- Durante a janela em que zipcode SKY ja mudou mas segmento ainda e antigo, o log deve mostrar que a leitura foi mantida como pendente/ignorada.
- Quando o segmento SKY correto chegar, o MBGUI deve salvar o bouquet correto e reconstruir a lista uma unica vez.
- A lista SKY deve ser montada com o bouquet correspondente ao segmento final.
- Reiniciar o receptor durante a janela de transicao nao deve fixar lista SKY em bouquet antigo.
- Ativacao Claro deve continuar salvando zone id e montando lista como antes.
- Nao deve haver loop de rebuild de lista quando o mesmo bouquet for recebido varias vezes.

## Plano de teste

### Teste 1 - Transicao Claro para SKY com EMM atrasada

1. Ativar receptor no satelite Claro.
2. Confirmar logs de zipcode Claro e segmento Claro.
3. Migrar para SKY.
4. Capturar logs de `SMARTCARDS` e `PURCHASE_HISTORY`.
5. Verificar caso intermediario em que zipcode SKY aparece antes do segmento SKY.
6. Confirmar que o MBGUI nao reconstruiu lista com segmento Claro.
7. Aguardar chegada do segmento SKY correto.
8. Confirmar rebuild unico com bouquet correto.

### Teste 2 - Reboot durante janela de transicao

1. Reproduzir transicao Claro -> SKY.
2. Reiniciar o receptor enquanto zipcode SKY ja mudou e segmento ainda nao.
3. Confirmar que o MBGUI nao persiste bouquet antigo.
4. Confirmar que a lista e reconstruida somente apos segmento confirmado.

### Teste 3 - SKY ja ativado

1. Inicializar receptor ja em SKY.
2. Confirmar que a lista e carregada normalmente.
3. Confirmar que callbacks repetidos de smartcard nao disparam rebuild desnecessario.

### Teste 4 - Claro

1. Ativar receptor em Claro.
2. Confirmar que zipcode Claro atualiza zone id.
3. Confirmar que nao houve atraso perceptivel causado pelo debounce SKY.

### Teste 5 - Regressao de lista e BAT

1. Com bouquet SKY confirmado, validar que `BOUQUET_ID + zone_id` coleta a BAT correta.
2. Confirmar canais regionais esperados.
3. Confirmar que canais de outra zona nao aparecem indevidamente.

## Riscos

- Atraso excessivo pode fazer a UI parecer travada durante ativacao SKY.
- Se o headend demorar mais que o debounce configurado, ainda pode ser necessario esperar uma segunda confirmacao.
- Alterar `Zone_ID_t` para `uint16_t` pode impactar persistencia em `zone.id`, IPC, banco e formatacao de logs.
- Ignorar callbacks iniciais sem logs claros pode dificultar diagnostico de campo.

## Dependencias externas

- Validar com suporte CAS/Headend:
  - sequenciamento das EMMs de zipcode e segmento;
  - tempo de carousel;
  - se `PURCHASE_HISTORY` e o melhor evento para indicar conclusao de atualizacao SKY;
  - traces CAK com biblioteca de engenharia quando disponivel.

## Sugestao de resposta ao suporte CAS

```text
We identified that the application currently reacts immediately to the SMARTCARDS callback. During the Claro to SKY transition, the zipcode may already be updated while the segment still contains the previous value, so the client can rebuild the SKY channel list using a transient segment.

We are planning to add a client-side debounce/confirmation step similar to the EitvCaMain implementation: wait for PURCHASE_HISTORY and/or confirm the same SKY segment in consecutive smartcard reads before saving the bouquet and rebuilding the channel list.

We will still collect CAK debug traces to confirm the EMM sequencing and carousel timing from the headend.
```

## Status

Aberto para implementacao.

