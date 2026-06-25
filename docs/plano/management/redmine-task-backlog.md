# Backlog de tarefas para Redmine

## Objetivo

Este arquivo lista tarefas iniciais para cadastrar no Redmine e controlar o projeto de documentacao do Software B8.

Use este backlog como ponto de partida. Ajuste responsaveis, estimativas, datas e prioridades conforme disponibilidade do time.

## Padrao sugerido de cadastro

Campos recomendados no Redmine:

- Tracker: Epic, Task, Review, Improvement ou Bug.
- Prioridade: Alta, Media, Baixa.
- Tipo Diataxis: Tutorial, Guia, Referencia, Explicacao.
- Documento alvo: caminho esperado em `docs/`.
- Status de validacao: A validar, Inferido, Confirmado, Publicado.
- Responsavel tecnico.
- Revisor humano.
- Link do PR.
- Marco.

Fluxo sugerido:

```text
Novo -> Em levantamento -> Em redacao -> Em revisao -> Publicado -> Fechado
```

## Marcos sugeridos

| Marco | Objetivo |
| --- | --- |
| M1 - Fundacao | Estrutura docs as code, MkDocs, Doxygen e processo leve |
| M2 - Onboarding e build | Guias para ambiente, build debug, build release e mapa do repositorio |
| M3 - Arquitetura | Visao tecnica dos modulos principais |
| M4 - Operacao e release | Atualizacao, assinatura, release, TPM e procedimentos criticos |
| M5 - Funcionalidades B8 | Processos de usuario, receptor, canais, operadoras e diagnostico |
| M6 - Referencia tecnica | Doxygen, referencias tecnicas e melhoria continua |

## Epicos

### EPIC-001 - Fundacao docs as code

Objetivo: criar a base minima para documentacao versionada, revisada e publicavel.

Tarefas:

| ID sugerido | Titulo | Prioridade | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- |
| DOC-001 | Criar configuracao inicial do MkDocs | Alta | `mkdocs.yml`, `docs/plano/index.md` | `mkdocs build --strict` executa com sucesso |
| DOC-002 | Criar configuracao inicial do Doxygen | Alta | `Doxyfile` | Doxygen gera HTML inicial com escopo controlado |
| DOC-003 | Definir checklist de PR para documentacao | Alta | `.github/pull_request_template.md` ou equivalente | Checklist inclui docs, IA, MkDocs e Doxygen quando aplicavel |
| DOC-004 | Definir publicacao local/interna do portal | Media | `docs/plano/management/redmine-wbs-schedule.md` | Fluxo de publicacao documentado |
| DOC-005 | Criar ADR da decisao MkDocs + Doxygen + docs as code | Alta | `docs/plano/decisions/0001-docs-as-code-mkdocs-doxygen.md` | ADR revisado e aceito |

### EPIC-002 - Inventario e planejamento

Objetivo: validar escopo, converter processos em backlog e preparar cronograma.

Tarefas:

| ID sugerido | Titulo | Prioridade | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- |
| DOC-010 | Validar inventario de processos e funcionalidades | Alta | `docs/plano/scope/process-functionality-inventory.md` | Itens revisados por desenvolvimento, QA, produto e release |
| DOC-011 | Priorizar processos por criticidade | Alta | `docs/plano/scope/process-functionality-inventory.md` | Prioridade ajustada e justificada |
| DOC-012 | Criar WBS no Redmine | Alta | Redmine | Epicos cadastrados conforme WBS aprovada |
| DOC-013 | Criar cronograma inicial no Redmine | Alta | Redmine | Marcos, datas e dependencias cadastrados |
| DOC-014 | Definir responsaveis e revisores por area | Alta | Redmine | Cada frente possui responsavel e revisor |

### EPIC-003 - Onboarding e build

Objetivo: permitir que um desenvolvedor prepare ambiente e compile o projeto.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-020 | Documentar ambiente de desenvolvimento | Alta | Tutorial | `docs/plano/getting-started/local-environment.md` | Pre-requisitos e ferramentas documentados |
| DOC-021 | Documentar build debug | Alta | Guia | `docs/plano/getting-started/build-debug.md` | Comandos de debug testados ou pendencias marcadas |
| DOC-022 | Documentar build release | Alta | Guia | `docs/plano/getting-started/build-release.md` | Comandos de release, saidas e pre-requisitos documentados |
| DOC-023 | Documentar toolchains ALi e Montage | Alta | Referencia | `docs/plano/reference/toolchains.md` | Toolchains, uso e diferencas documentados |
| DOC-024 | Documentar SDK Montage | Media | Guia | `docs/plano/release/montage-sdk.md` | Dockerfile e scripts do SDK explicados |
| DOC-025 | Criar mapa do repositorio | Alta | Referencia | `docs/plano/reference/repository-map.md` | Diretorios e responsabilidades descritos |
| DOC-026 | Documentar scripts principais | Media | Referencia | `docs/plano/reference/scripts.md` | Scripts principais com objetivo, entrada, saida e risco |
| DOC-027 | Documentar flags CMake | Alta | Referencia | `docs/plano/reference/build-flags.md` | Flags principais descritas com padrao e efeito |

### EPIC-004 - Arquitetura tecnica

Objetivo: explicar os modulos principais e seus relacionamentos.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-030 | Criar visao geral de arquitetura | Alta | Explicacao | `docs/plano/architecture/overview.md` | Visao revisada por responsavel tecnico |
| DOC-031 | Documentar mapa de modulos | Alta | Explicacao | `docs/plano/architecture/modules.md` | Modulos, responsabilidades e dependencias descritos |
| DOC-032 | Documentar modelo de tasks | Alta | Explicacao | `docs/plano/architecture/task-model.md` | Tasks principais e comunicacao documentadas |
| DOC-033 | Documentar HAL e plataformas | Alta | Explicacao | `docs/plano/architecture/platform-hal.md` | ALi/Montage e contratos HAL descritos |
| DOC-034 | Documentar pipeline DVB | Alta | Explicacao | `docs/plano/architecture/dvb-pipeline.md` | NIT, SDT, BAT e fluxo de canais descritos |
| DOC-035 | Documentar UI LVGL | Media | Explicacao | `docs/plano/architecture/ui-lvgl.md` | Organizacao de telas e recursos explicada |
| DOC-036 | Documentar CAS/Nagra | Alta | Explicacao | `docs/plano/architecture/cas-nagra.md` | Conteudo revisado por responsavel autorizado |
| DOC-037 | Documentar TPM | Alta | Explicacao | `docs/plano/architecture/tpm.md` | Funcionamento geral e limites documentados |

### EPIC-005 - Operacao, atualizacao e release

Objetivo: reduzir risco em processos criticos de release, update e producao.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-040 | Documentar processo de release | Alta | Runbook | `docs/plano/release/release-process.md` | Passos, artefatos, validacao e responsavel documentados |
| DOC-041 | Documentar geracao de `.b8` | Alta | Runbook | `docs/plano/release/generate-b8.md` | Entradas, saidas e validacao documentadas |
| DOC-042 | Documentar geracao de `.bin` | Alta | Runbook | `docs/plano/release/generate-bin.md` | Fluxo e uso documentados |
| DOC-043 | Documentar assinatura Nagra | Alta | Runbook | `docs/plano/release/signing-nagra.md` | Revisao do responsavel de release/seguranca concluida |
| DOC-044 | Documentar scripts de release | Alta | Referencia | `docs/plano/release/release-scripts.md` | Scripts e parametros descritos |
| DOC-045 | Documentar atualizacao via USB | Alta | Runbook | `docs/plano/operations/usb-update.md` | Procedimento e rollback documentados |
| DOC-046 | Documentar atualizacao via satelite | Alta | Runbook | `docs/plano/operations/satellite-update.md` | Fluxo e validacoes documentados |
| DOC-047 | Documentar atualizacao compulsoria | Alta | Explicacao | `docs/plano/operations/forced-update.md` | Regras e comportamento documentados |
| DOC-048 | Documentar TPM para producao | Alta | Runbook | `docs/plano/operations/tpm-production.md` | Procedimento validado por responsavel tecnico |
| DOC-049 | Documentar protocolo HTTP/TPM | Alta | Referencia | `docs/plano/api/tpm-http.md` | Endpoints, entradas, saidas e erros documentados |

### EPIC-006 - Funcionalidades do receptor

Objetivo: documentar comportamento funcional do B8 para desenvolvimento, QA e suporte.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-060 | Documentar stand-by | Alta | Explicacao | `docs/features/standby.md` | Entrada, estado e retorno documentados para Claro/Sky |
| DOC-061 | Documentar lista de canais TV/Radio | Alta | Guia | `docs/features/channel-list.md` | Edicao e favoritos documentados |
| DOC-062 | Documentar busca automatica de canais | Alta | Guia | `docs/features/auto-channel-search.md` | Sequencia e criterios documentados |
| DOC-063 | Documentar busca manual de canais | Alta | Guia | `docs/features/manual-channel-search.md` | Parametros e validacao documentados |
| DOC-064 | Documentar Instala Facil | Alta | Tutorial | `docs/features/instala-facil.md` | Sequencia funcional validada |
| DOC-065 | Documentar padrao de fabrica | Alta | Guia | `docs/features/factory-reset.md` | Procedimentos Claro/Sky documentados |
| DOC-066 | Documentar LNBF | Alta | Referencia | `docs/features/lnbf.md` | Tipos e configuracoes documentados |
| DOC-067 | Documentar chave DiSEqC | Alta | Guia | `docs/features/diseqc.md` | Configuracao e busca documentadas |
| DOC-068 | Documentar relogio e fuso horario | Alta | Explicacao | `docs/features/clock-timezone.md` | Fonte de data/hora e regras documentadas |
| DOC-069 | Documentar transicao Claro/Sky | Alta | Explicacao | `docs/features/operator-transition.md` | Regras e cenarios documentados |
| DOC-070 | Documentar multimidia | Media | Referencia | `docs/features/multimedia.md` | Fotos, videos, musicas e codecs documentados |
| DOC-071 | Documentar gravador | Alta | Guia | `docs/features/recorder.md` | Pendrive, opcoes e validacao documentados |
| DOC-072 | Documentar agendamentos | Alta | Guia | `docs/features/schedules.md` | Gravacao, assistir e stand-by documentados |
| DOC-073 | Documentar Closed Caption | Alta | Referencia | `docs/features/closed-caption.md` | CEA 608, CEA 708, ARIB B-37 e DVB Subtitle documentados |
| DOC-074 | Documentar audio L/R e audiodescricao | Alta | Referencia | `docs/features/audio-rules.md` | Portugues, Ingles e AUD revisados por norma |
| DOC-075 | Documentar tela de informacoes do canal | Media | Referencia | `docs/features/channel-info.md` | Info 1x e Info 2x documentados |
| DOC-076 | Documentar teclas e atalhos | Media | Referencia | `docs/features/remote-keys.md` | LAST, Mais, Sleep e TV/Radio documentados |
| DOC-077 | Documentar tela de diagnostico | Alta | Referencia | `docs/features/diagnostics.md` | Campos e interpretacao documentados |
| DOC-078 | Documentar mensagens gerais | Alta | Referencia | `docs/features/messages.md` | Tipos e formatos Sky/Nagra/Century documentados |
| DOC-079 | Documentar informacoes do receptor | Media | Referencia | `docs/features/receiver-info.md` | Campos documentados |
| DOC-080 | Documentar acesso condicional | Alta | Referencia | `docs/features/conditional-access-info.md` | Informacoes e regras documentadas |
| DOC-081 | Documentar suporte | Media | Guia | `docs/features/support.md` | Fluxos e informacoes documentados |

### EPIC-007 - Operadoras e regras satelitais

Objetivo: documentar regras especificas de Claro/SATHDR e Sky/Nova Parabolica.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-090 | Documentar Claro SATHDR | Alta | Explicacao | `docs/operators/claro-sathdr.md` | BAT, canais e regras revisadas |
| DOC-091 | Documentar canal de servico Claro | Alta | Referencia | `docs/operators/claro-service-channel.md` | 12120 MHz, OTA e DVB-SI documentados |
| DOC-092 | Documentar regras BAT Claro | Alta | Referencia | `docs/operators/claro-bat-rules.md` | Regras validadas com codigo/norma |
| DOC-093 | Documentar Sky Nova Parabolica | Alta | Explicacao | `docs/operators/sky-nova-parabolica.md` | Canais livres, recarga e lista documentados |
| DOC-094 | Documentar regras de recarga Sky | Alta | Referencia | `docs/operators/sky-recharge.md` | Regras validadas |

### EPIC-008 - Estruturas internas e dados

Objetivo: documentar dados persistidos, arquivos e estruturas relevantes.

Tarefas:

| ID sugerido | Titulo | Prioridade | Tipo Diataxis | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- | --- |
| DOC-100 | Documentar BD de canais | Alta | Referencia | `docs/plano/reference/channel-database.md` | Estrutura e uso documentados |
| DOC-101 | Documentar BD de configuracao STB | Alta | Referencia | `docs/plano/reference/stb-configuration-database.md` | Estrutura e uso documentados |
| DOC-102 | Documentar arquivos ZoneID e PersonalBit | Alta | Referencia | `docs/plano/reference/device-configuration-files.md` | Arquivos e regras documentados |
| DOC-103 | Documentar arquivos/configuracoes Nagra | Alta | Referencia | `docs/plano/reference/nagra-configuration-files.md` | Revisao autorizada concluida |

### EPIC-009 - Referencia Doxygen assistida por IA

Objetivo: revisar arquivos `.h` e `.cpp` com apoio de IA, inserir comentarios Doxygen padronizados e gerar referencia tecnica revisada.

Tarefas:

| ID sugerido | Titulo | Prioridade | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- |
| DOC-110 | Criar `Doxyfile` inicial | Alta | `Doxyfile` | Doxygen gera HTML inicial com escopo controlado |
| DOC-111 | Gerar baseline Doxygen sem comentarios novos | Alta | `build/doxygen` | Warnings iniciais registrados |
| DOC-112 | Definir padrao de comentarios Doxygen com IA | Alta | `docs/plano/tooling/doxygen-commenting-guidelines.md` | Padrao revisado e aprovado |
| DOC-113 | Piloto: revisar headers `.h` de modulo pequeno | Alta | Modulo piloto | Comentarios em headers revisados por responsavel tecnico |
| DOC-114 | Revisar headers `.h` de `src/common` | Alta | `src/common` | Interfaces relevantes comentadas e Doxygen validado |
| DOC-115 | Revisar headers `.h` de `src/hal` | Alta | `src/hal` | Contratos HAL comentados e revisados |
| DOC-116 | Revisar headers `.h` de `src/dvb` | Alta | `src/dvb` | Tipos, structs e contratos DVB comentados |
| DOC-117 | Revisar headers `.h` de `src/tasks` | Media | `src/tasks` | Contratos principais de tasks comentados |
| DOC-118 | Revisar headers `.h` de `src/tpm` | Alta | `src/tpm` | Interfaces criticas TPM/API comentadas |
| DOC-119 | Revisar `.cpp` relevantes apos headers | Media | `src/**/*.cpp` | Apenas logica relevante e efeitos colaterais comentados |
| DOC-120 | Avaliar `ui/lvgl` para comentarios Doxygen | Media | `ui/lvgl` | Escopo definido e comentarios relevantes revisados |
| DOC-121 | Avaliar `src/cas/nagra` com revisao autorizada | Media | `src/cas/nagra` | Escopo aprovado por responsavel autorizado |
| DOC-122 | Corrigir warnings e ruido Doxygen | Media | `Doxyfile`, codigo | Warnings novos analisados/corrigidos |
| DOC-123 | Integrar links Doxygen no MkDocs | Media | `docs/plano/reference/code-reference.md` | Portal aponta para referencia gerada |

### EPIC-010 - Qualidade e melhoria continua

Objetivo: manter a documentacao util e atualizada com baixo atrito.

Tarefas:

| ID sugerido | Titulo | Prioridade | Documento alvo | Criterio de aceite |
| --- | --- | --- | --- | --- |
| DOC-130 | Automatizar build MkDocs | Alta | CI/script | Build reproduzivel documentado |
| DOC-131 | Automatizar build Doxygen | Media | CI/script | Build reproduzivel documentado |
| DOC-132 | Criar rotina mensal de revisao de lacunas | Media | `docs/plano/management/documentation-management-plan.md` | Rotina aprovada |
| DOC-133 | Criar rotina trimestral de revisao critica | Media | `docs/plano/management/documentation-management-plan.md` | Documentos criticos definidos |
| DOC-134 | Avaliar markdownlint | Baixa | CI/script | Relatorio gerado sem bloquear inicialmente |
| DOC-135 | Avaliar checagem de links | Baixa | CI/script | Relatorio gerado sem bloquear inicialmente |
| DOC-136 | Avaliar spellcheck tecnico | Baixa | CI/script | Dicionario inicial definido |

## Tarefas de revisao humana

Estas tarefas podem ser criadas como subtarefas ou tracker `Review`.

| ID sugerido | Titulo | Prioridade | Relacionado a |
| --- | --- | --- | --- |
| REV-001 | Revisar inventario de processos com QA | Alta | DOC-010 |
| REV-002 | Revisar inventario de processos com desenvolvimento | Alta | DOC-010 |
| REV-003 | Revisar processos de release com responsavel de release | Alta | DOC-040 a DOC-044 |
| REV-004 | Revisar conteudo Nagra/CAS com responsavel autorizado | Alta | DOC-036, DOC-043, DOC-103 |
| REV-005 | Revisar regras Claro/Sky com especialista de produto | Alta | DOC-090 a DOC-094 |
| REV-006 | Revisar guias de build com desenvolvedor que nao escreveu o documento | Media | DOC-020 a DOC-027 |

## Observacoes para cadastro

- Nao cadastrar todas as tarefas de uma vez se isso gerar ruido. Comece por epicos e tarefas de prioridade alta.
- Use as tarefas de prioridade media e baixa como backlog futuro.
- Para cada documento, prefira uma tarefa pequena e revisavel.
- Quando um item for grande demais, divida por subtarefas de levantamento, redacao e revisao.
- Vincule PRs do Git aos IDs Redmine para manter rastreabilidade.



