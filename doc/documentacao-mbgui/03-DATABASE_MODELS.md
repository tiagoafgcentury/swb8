# 03 - Dominio de Dados e Banco SQLite

## Visao Geral

O MBGUI usa SQLite para persistir satelites, transponders, servicos/canais, agenda/PVR e configuracoes derivadas. A task responsavel por I/O e `Task_Database`, em `src/tasks/mb_task_database.cpp`.

O banco local e parte critica do suporte multi-satelite. A principal regra atual e:

> Nunca use somente `service_id` para identificar, atualizar, ordenar, favoritar, agendar ou sintonizar canais. Use `transponder_id + service_id` ou uma chave estavel equivalente.

## Identidade Atual de Canal

O codigo atual define `Transponder_Id` em `src/common/mb_types.h`.

`Transponder_Id::set_frequency()` monta um valor de 64 bits contendo:

- frequencia
- polaridade
- `satellite_id` deslocado para os bits altos

Portanto, a identidade pratica de um servico e:

```text
ServiceKey = { transponder_id, service_id }
```

Importante: documentos antigos mencionavam `unique_id = (transponder_id << 16) | service_id`. Essa formulacao nao deve ser usada cegamente porque `transponder_id` ja e 64-bit no codigo atual. Para novas analises e correcoes, prefira falar em `ServiceKey` ou em `srv.srv_id` se for escolhida uma chave primaria relacional.

## Tabela `sat`

Criada em `Task_Database::create_tables()`.

Campos principais:

- `sat_id`: chave primaria do satelite.
- `name`: nome exibido na UI.
- `band`: banda C/Ku.
- `type`: tipo de LNBF.
- `position`: polaridade/posicao LNBF.
- `switch_type`: tipo de chave, por exemplo `None`, `DiseqC_1_0`, `DiseqC_1_1`.
- `switch_pos`: porta da chave DiSEqC.
- `orbital`: posicao orbital.
- `is_mandatory`: satelite obrigatorio.
- `network_policies`: politica da rede/operadora, usada para selecionar comportamento Sky, TVRO/Claro ou Generic.

O cadastro inicial e feito por `Task_Database::populate_satellite()`.

Ponto atual de atencao:

- A insercao de novo satelite pela UI usa `insert into sat (...)` sem retornar automaticamente o `sat_id` gerado para a tela de edicao.
- Isso e relevante para bugs ao criar satelite novo e editar chave/DiSEqC repetidas vezes.

## Tabela `tp`

Armazena transponders.

Campos relevantes:

- `transponder_id`: identificador de 64 bits.
- `symbol_rate`
- `dvb_mode`
- `transport_stream_id`
- `original_network_id`
- `network_id`
- `is_home_channel`
- `satellite_id`

Mesmo havendo coluna `satellite_id`, o `transponder_id` tambem carrega `satellite_id` no tipo `Transponder_Id`. Ao inserir ou carregar transponders, manter esses dois dados coerentes.

## Tabela `srv`

Armazena servicos/canais.

Campos relevantes:

- `srv_id`: chave primaria relacional.
- `transponder_id`
- `service_id`
- `service_type`
- `name`
- `epg_pid`
- `regionalizacao`
- `viewer_channel`
- `bouquet_id`
- `is_favorite`
- `order_in_full`
- `order_in_favorite`

O schema atual possui constraint unica:

```sql
unique (transponder_id, service_id)
```

Essa constraint confirma que `service_id` sozinho nao identifica canal no sistema.

## Tabela `agenda`

Usada para agenda/PVR/lembretes.

Ponto de atencao atual:

- A documentacao antiga tratava agenda como se guardasse `unique_id`.
- O codigo deve ser revisado sempre que mexer em agenda para garantir que o canal agendado seja resolvido por chave estavel, nao apenas por `service_id`.
- Em cenarios multi-satelite, agenda deve apontar para `ServiceKey` ou para `srv_id`, conforme decisao tecnica do release.

## Configuracao em Arquivo de Estado

`State_File::App_State_File` armazena dados como:

- `network_id`
- `current_satellite_id`
- `band`
- `lnbf_type`
- `lnbf_inverted`
- flags de standby/producao/configuracao

Esse estado interage com `Config::set_config()`, `Config::select_satellite_by_id()` e carregamento de lista de satelites.

## Fluxos de Persistencia Criticos

### Criar satelite

Fluxo UI atual:

1. `OSD_Edit_Satellite::save_changes()`
2. se `m_current_satellite.id == 0`, chama `Task::post_event_add_satellite(m_current_satellite)`
3. `Task_Database::handle_event_add_satellite()` insere no banco
4. UI chama `Config::load_satellite_list()`

Risco conhecido:

- O ID gerado no banco nao volta imediatamente para `m_current_satellite`.

### Editar satelite/chave

Fluxo:

1. UI altera `switch_type`/`switch_pos`
2. `save_changes()` chama `update_satellite()` se o ID for diferente de zero
3. `Task_Database::handle_event_update_satellite()` executa `update sat ... where sat_id = :id`

Recomendacao:

- Verificar `sqlite3_changes()` apos update.
- Nunca assumir que o update afetou uma linha se o ID veio de estado recem-criado.

### Salvar lineup

`Task_Database` grava transponders e servicos encontrados. Todo transponder/servico deve manter satelite correto para DiSEqC, CAS, regionalizacao e UI.

## Checklist para Mudancas no Banco

- Preservar constraint `(transponder_id, service_id)`.
- Nao criar queries de update/delete por `service_id` isolado.
- Ao adicionar satelites default, pensar em migracao para bancos existentes.
- Ao trocar operadora/satelite, invalidar ou recarregar estado derivado.
- Validar listas vazias antes de acessar `[0]`.
- Logar `satellite_id`, `transponder_id`, `service_id`, `network_policies`, `switch_type` e `switch_pos` em bugs de sintonia/regionalizacao.

