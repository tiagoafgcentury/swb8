# Plano de gestao da documentacao

## Objetivo

Gerenciar a estruturacao da documentacao como um projeto continuo, usando docs as code, MkDocs, Doxygen, apoio de IA e revisao humana.

Este plano deve orientar backlog, prioridades, cadencia, responsabilidades e criterios de qualidade.

## Resultado esperado

Ao final da estruturacao inicial, o projeto deve ter:

- Portal MkDocs navegavel.
- Referencia Doxygen gerada a partir do codigo C/C++.
- Estrutura de documentos por audiencia e tipo de uso.
- Processo de PR com checklist de documentacao.
- Fluxo de melhoria continua.
- Politica de uso de IA com verificacao humana.
- Backlog vivo de lacunas documentais.

## Politica leve de execucao

Para manter o ritmo do desenvolvimento:

- Nem todo PR precisa alterar documentacao.
- Atualizacao pequena em documento existente deve ser preferida a criar documento novo.
- ADR deve ser reservado para decisoes duradouras.
- Validacoes comecam simples: MkDocs para portal e Doxygen quando referencia de codigo for afetada.
- Markdownlint, spellcheck e link checker podem entrar como recomendacao antes de virarem bloqueio.

Use os niveis definidos em `docs/plano/process/lightweight-documentation-policy.md`.
## Controle no Redmine

O Redmine sera usado para planejar e acompanhar o projeto de documentacao.

Regras praticas:

- Criar epicos para as frentes da WBS.
- Criar tarefas para documentos ou grupos pequenos de documentos.
- Linkar cada tarefa ao PR correspondente.
- Usar os itens do inventario de processos como backlog inicial.
- Priorizar por risco, recorrencia e dependencia de conhecimento oral.
- Manter o Git como fonte da verdade do conteudo.

Documentos de apoio:

- `docs/plano/management/redmine-wbs-schedule.md`.
- `docs/plano/management/redmine-task-backlog.md`.
## Frentes de trabalho

### Frente 1: Fundacao docs as code

Objetivo: criar a infraestrutura minima para documentar com controle e revisao.

Entregaveis:

- `docs/plano/process/docs-as-code.md`.
- `mkdocs.yml` inicial.
- `Doxyfile` inicial.
- `docs/ppt-plano/src/index.md` como entrada gerada do portal.
- Checklist de PR para documentacao.
- Comandos locais de validacao.

Criterio de aceite:

- `mkdocs build --strict` executa com sucesso.
- `doxygen Doxyfile` executa com warnings conhecidos e registrados.
- Portal lista os documentos existentes.

### Frente 2: Inventario e mapa do repositorio

Objetivo: transformar a estrutura atual do projeto em mapa navegavel.

Entregaveis:

- `docs/plano/reference/repository-map.md`.
- `docs/plano/reference/scripts.md`.
- `docs/plano/reference/build-flags.md`.
- `docs/plano/reference/external-dependencies.md`.

Criterio de aceite:

- Cada diretorio relevante tem responsabilidade descrita.
- Scripts principais tem objetivo, entrada, saida e risco documentados.
- Flags CMake principais estao explicadas.

### Frente 3: Onboarding e build

Objetivo: permitir que um novo desenvolvedor prepare ambiente e compile.

Entregaveis:

- `docs/plano/getting-started/local-environment.md`.
- `docs/plano/getting-started/build-debug.md`.
- `docs/plano/getting-started/build-release.md`.
- `docs/plano/release/montage-sdk.md`.

Criterio de aceite:

- Uma pessoa consegue seguir o guia sem apoio oral.
- Comandos foram testados ou pendencias foram marcadas.
- Diferencas entre ALi, Montage, debug e release estao claras.

### Frente 4: Arquitetura

Objetivo: documentar a visao tecnica dos modulos principais.

Entregaveis:

- `docs/plano/architecture/overview.md`.
- `docs/plano/architecture/modules.md`.
- `docs/plano/architecture/task-model.md`.
- `docs/plano/architecture/platform-hal.md`.
- `docs/plano/architecture/dvb-pipeline.md`.
- `docs/plano/architecture/ui-lvgl.md`.
- `docs/plano/architecture/cas-nagra.md`.
- `docs/plano/architecture/tpm.md`.

Criterio de aceite:

- Cada modulo tem responsabilidade, entradas, saidas e dependencias principais.
- Inferencias geradas por IA foram revisadas por dono tecnico.
- Lacunas nao confirmadas estao marcadas.

### Frente 5: Operacao e release

Objetivo: reduzir risco em atividades criticas.

Entregaveis:

- `docs/plano/release/release-process.md`.
- `docs/plano/release/signing-nagra.md`.
- `docs/plano/release/ota-package.md`.
- `docs/plano/operations/usb-update.md`.
- `docs/plano/operations/debug-device.md`.
- `docs/plano/operations/logs-and-diagnostics.md`.

Criterio de aceite:

- Procedimentos tem pre-requisitos, passos, validacao e rollback quando aplicavel.
- Informacoes sensiveis foram revisadas.
- Responsavel de release aprovou os documentos criticos.

### Frente 6: Referencia Doxygen assistida por IA

Objetivo: revisar arquivos `.h` e `.cpp` com apoio de IA para inserir comentarios Doxygen padronizados, priorizando contratos tecnicos reais e mantendo revisao humana obrigatoria.

Escopo inicial recomendado:

- Onda 1: revisar e comentar headers `.h` de `src/common`.
- Onda 2: revisar e comentar headers `.h` de `src/hal`, `src/dvb`, `src/tasks` e `src/tpm`.
- Onda 3: revisar `.cpp` com logica relevante, efeitos colaterais ou contratos implicitos.
- Onda 4: gerar Doxygen, corrigir warnings e linkar a referencia no MkDocs.
- Onda 5: avaliar `ui/lvgl` e `src/cas/nagra`, com revisao autorizada quando necessario.

Criterio de aceite:

- Doxygen gera referencia navegavel.
- Interfaces criticas tem comentarios sobre contrato, parametros, retorno e efeitos colaterais.
- Codigo de terceiros em `extern/` nao polui a referencia principal.

## Backlog inicial

| Prioridade | Item | Tipo | Responsavel | Status |
| --- | --- | --- | --- | --- |
| Alta | Criar `mkdocs.yml` inicial | Infra | A definir | Pendente |
| Alta | Criar `Doxyfile` inicial | Infra | A definir | Pendente |
| Alta | Criar mapa do repositorio | Referencia | A definir | Pendente |
| Alta | Documentar build debug | Guia | A definir | Pendente |
| Alta | Documentar build release | Guia | A definir | Pendente |
| Media | Documentar scripts principais | Referencia | A definir | Pendente |
| Media | Documentar SDK Montage | Release | A definir | Pendente |
| Media | Documentar assinatura Nagra | Runbook | A definir | Pendente |
| Media | Criar overview de arquitetura | Arquitetura | A definir | Pendente |
| Media | Piloto Doxygen em `src/hal` | Doxygen | A definir | Pendente |
| Baixa | Automatizar markdownlint | Qualidade | A definir | Pendente |
| Baixa | Automatizar link checker | Qualidade | A definir | Pendente |

## Cadencia de gestao

### Semanal

- Revisar novas lacunas.
- Priorizar documentos bloqueadores.
- Checar PRs de documentacao abertos.
- Registrar documentos com pendencias de validacao humana.

### Por sprint ou ciclo de desenvolvimento

- Planejar 2 a 5 melhorias documentais.
- Garantir que mudancas de codigo relevantes atualizaram docs.
- Revisar metricas basicas.

### Por release

- Revisar guias de release, assinatura, OTA e update.
- Confirmar comandos e artefatos esperados.
- Registrar mudancas de processo.

### Trimestral

- Revisar documentos criticos.
- Arquivar ou atualizar paginas obsoletas.
- Reavaliar regras de CI.
- Medir evolucao da cobertura Doxygen.

## Indicadores

Indicadores simples para acompanhar:

- Numero de paginas oficiais no MkDocs.
- Numero de runbooks criticos revisados.
- Numero de warnings Doxygen.
- Numero de PRs com checklist de documentacao preenchido.
- Tempo para novo desenvolvedor executar build inicial.
- Numero de duvidas repetidas que viraram documentos.
- Numero de documentos com status `Pendente` ou `Inferido`.

## Riscos e mitigacoes

| Risco | Mitigacao |
| --- | --- |
| Documentacao virar copia desatualizada do codigo | Usar Doxygen para referencia de codigo e MkDocs para contexto |
| IA introduzir informacao incorreta | Exigir revisao humana e marcadores de confiabilidade |
| Processo ficar pesado demais | Comecar com checklist simples e poucos gates |
| Doxygen gerar muito ruido | Comecar por modulos criticos e excluir terceiros |
| Portal nao ser usado | Tornar MkDocs a entrada oficial e linkar no README raiz |
| Documentacao nao acompanhar PRs | Checklist obrigatorio e revisao pelo dono do modulo |

## Etapa Doxygen assistida por IA

Sequencia recomendada:

1. Definir `Doxyfile` inicial.
2. Rodar baseline Doxygen sem alterar codigo.
3. Selecionar primeiro modulo piloto.
4. Usar IA para gerar rascunhos de comentarios nos arquivos `.h`.
5. Revisar tecnicamente cada comentario antes de aplicar.
6. Aplicar comentarios aprovados em branch/MR pequena.
7. Rodar Doxygen e registrar warnings.
8. Avancar para `.cpp` apenas onde ha valor tecnico.
9. Publicar/linkar referencia no MkDocs.

Documento de apoio: `docs/plano/tooling/doxygen-commenting-guidelines.md`.

## Primeiros 30 dias

1. Fechar decisao de docs as code, MkDocs e Doxygen em ADR.
2. Criar configuracao minima de MkDocs.
3. Criar configuracao minima de Doxygen.
4. Publicar primeira versao local do portal.
5. Criar mapa do repositorio.
6. Criar guias de build debug e release.
7. Fazer piloto de IA em um modulo de baixo risco.
8. Fazer piloto Doxygen em uma area pequena, preferencialmente `src/common` ou `src/hal`.

## Primeiros 60 dias

1. Documentar arquitetura geral e modulos principais.
2. Documentar scripts principais.
3. Documentar processo de release.
4. Integrar build MkDocs e Doxygen no CI ou script local padronizado.
5. Criar rotina de revisao mensal de lacunas.

## Primeiros 90 dias

1. Expandir Doxygen para interfaces criticas.
2. Revisar runbooks operacionais.
3. Medir reducao de duvidas recorrentes.
4. Endurecer validacoes automatizadas.
5. Publicar portal em ambiente interno ou pipeline oficial.







