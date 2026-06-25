# Estrategia de documentacao

## Objetivo

Criar uma base de documentacao tecnica sustentavel para o projeto `mbgui`, sem reorganizar a estrutura do software. A documentacao deve reduzir dependencia de conhecimento oral, acelerar onboarding, diminuir duvidas repetidas e tornar operacoes criticas mais auditaveis.

O diretorio `doc/` existente deve ser ignorado nesta estrategia, pois contem testes de documentacao.

## Diagnostico inicial do repositorio

O repositorio aparenta ser um projeto C/C++ embarcado com CMake, focado no produto MidiaBox GUI.

Componentes observados:

- Build principal em `CMakeLists.txt`, com opcoes de compilacao para recursos como HLS, SAT Monitor, Nagra, animacoes, sanitizers e debug.
- Toolchains MIPS em `toolchain_mips_ali.txt` e `toolchain_mips_montage.txt`.
- Scripts de configuracao e build na raiz, como `configure.sh`, `configure-release.sh`, `configure-nagra.sh`, `configure-dl.sh`, `docker-make.sh` e `generate_release_seed.sh`.
- SDK Montage em `montage_sdk/`, incluindo Dockerfile e scripts de build.
- Codigo principal em `src/`, com modulos `common`, `dvb`, `hal`, `cas`, `tasks`, `tpm` e `dlgui`.
- UI em `ui/lvgl`, com telas, componentes OSD, traducoes e integracao LVGL.
- Recursos visuais em `res/`.
- Dependencias externas em `extern/`, incluindo SQLite, LVGL, rlottie, EFL, fontes e fw_env.
- Scripts operacionais e artefatos auxiliares em `extra/`, incluindo USB/update, assinatura Nagra e colecao Postman.
- Hooks Git em `git-hooks/`.
- Debug auxiliar em `debug/`.

Lacunas iniciais:

- Nao ha README principal fora do diretorio `doc/`.
- Nao ha guia central de build, toolchain ou ambiente.
- Nao ha mapa oficial de arquitetura por modulo.
- Nao ha processo documentado para release, assinatura, OTA e update USB.
- Nao ha registro formal de decisoes arquiteturais.
- Nao ha padrao para manter documentacao atualizada em pull requests.

## Principios

- Documentar o que reduz risco operacional ou acelera trabalho recorrente.
- Manter documentacao em `docs/plano/`, proxima ao repositorio e revisada junto com codigo.
- Adotar docs as code: versionamento, pull request, validacao automatizada e publicacao reproduzivel.
- O processo deve ser leve: usar o menor nivel de documentacao suficiente para cada mudanca.
- Preferir Markdown simples no inicio; adicionar portal apenas quando houver massa critica.
- Separar tutoriais, guias, referencia e explicacoes.
- Usar MkDocs como portal navegavel da documentacao escrita e revisada.
- Usar Doxygen para referencia tecnica extraida do codigo C/C++.
- Usar IA como acelerador de levantamento e rascunho, nunca como fonte final sem revisao humana.
- Tratar documentacao como contexto reutilizavel para desenvolvedores, revisores, QA e ferramentas de IA.
- Toda decisao tecnica relevante deve ter ADR.
- Toda rotina operacional critica deve ter runbook.
- Toda mudanca de comportamento, build, release, API, variavel ou integracao deve atualizar documentacao.

## Publicos-alvo

- Desenvolvedor novo: precisa compilar, executar, entender modulos e fluxo de trabalho.
- Mantenedor: precisa localizar responsabilidades, entender tradeoffs e revisar mudancas.
- Release/operacoes: precisa gerar builds, pacotes, updates, assinaturas e diagnosticar falhas.
- QA: precisa entender variantes, flags, cenarios de validacao e artefatos esperados.
- Produto/suporte tecnico: precisa consultar comportamento de alto nivel e fluxos do dispositivo.

## Estrutura proposta

```text
docs/
  plano/
    README.md
    documentation-strategy.md
    getting-started/
      local-environment.md
      build-debug.md
      build-release.md
    architecture/
      overview.md
      modules.md
      task-model.md
      platform-hal.md
      ui-lvgl.md
      dvb-pipeline.md
      cas-nagra.md
      tpm.md
    development/
      coding-guidelines.md
      cmake-options.md
      adding-ui-screen.md
      adding-dvb-table.md
      working-with-resources.md
    operations/
      debug-device.md
      logs-and-diagnostics.md
      usb-update.md
      postman-tests.md
    release/
      release-process.md
      montage-sdk.md
      signing-nagra.md
      ota-package.md
      versioning.md
    api/
      http-server.md
      postman-collection.md
    decisions/
      0001-record-architecture-decisions.md
    tooling/
      doxygen-mkdocs.md
    process/
      docs-as-code.md
      lightweight-documentation-policy.md
      ai-assisted-documentation.md
    management/
      documentation-management-plan.md
      redmine-wbs-schedule.md
    scope/
      process-functionality-inventory.md
    reference/
      build-flags.md
      scripts.md
      repository-map.md
      environment-variables.md
    document-templates/
      adr.md
      runbook.md
      technical-guide.md
      api-reference.md
```

Crie os arquivos gradualmente. A prioridade nao e preencher tudo de uma vez, e sim transformar conhecimento critico em documentos revisaveis.

## Priorizacao recomendada

### Fase 1: Fundacao

Objetivo: permitir que alguem entenda e compile o projeto sem depender de explicacao oral.

Entregaveis:

- `docs/plano/getting-started/local-environment.md`
- `docs/plano/getting-started/build-debug.md`
- `docs/plano/getting-started/build-release.md`
- `docs/plano/reference/repository-map.md`
- `docs/plano/reference/scripts.md`
- README principal na raiz apontando para `docs/plano/`

### Fase 2: Arquitetura

Objetivo: explicar como o sistema e dividido e como os fluxos principais se conectam.

Entregaveis:

- `docs/plano/architecture/overview.md`
- `docs/plano/architecture/modules.md`
- `docs/plano/architecture/task-model.md`
- `docs/plano/architecture/platform-hal.md`
- `docs/plano/architecture/ui-lvgl.md`
- `docs/plano/architecture/dvb-pipeline.md`
- `docs/plano/architecture/cas-nagra.md`
- Primeiros ADRs em `docs/plano/decisions/`

### Fase 3: Operacao e release

Objetivo: reduzir risco em processos criticos.

Entregaveis:

- `docs/plano/release/release-process.md`
- `docs/plano/release/signing-nagra.md`
- `docs/plano/release/ota-package.md`
- `docs/plano/release/montage-sdk.md`
- `docs/plano/operations/usb-update.md`
- `docs/plano/operations/debug-device.md`

### Fase 4: Qualidade continua

Objetivo: fazer documentacao evoluir junto com o codigo.

Entregaveis:

- Template de pull request com checklist de documentacao.
- Validacao automatica de links Markdown.
- Validacao de geracao MkDocs.
- Validacao de geracao Doxygen.
- Processo documentado para uso de IA com revisao humana.
- Criar biblioteca inicial de prompts reutilizaveis, instructions, skills e hooks candidatos para o fluxo de documentacao.
- Revisao trimestral de paginas criticas.
- Backlog de lacunas documentais.

## Governanca

### Regras de atualizacao

Atualizar documentacao e obrigatorio quando uma mudanca:

- Altera comando, build, toolchain, flag CMake ou ambiente.
- Altera comportamento visivel do usuario.
- Altera fluxo de release, assinatura, OTA ou update.
- Altera contrato de API, mensagem, endpoint ou formato de dados.
- Adiciona modulo, tela, task, dependencia externa ou integracao.
- Introduz decisao arquitetural ou tradeoff relevante.

### Checklist de PR

Sugestao para `.github/pull_request_template.md` ou equivalente:

```md
## Documentacao

- [ ] Esta mudanca nao exige atualizacao de documentacao
- [ ] README ou `docs/plano/` atualizado
- [ ] ADR criado ou atualizado
- [ ] Flags, scripts ou variaveis documentadas
- [ ] Processo operacional ou de release atualizado
- [ ] API, contrato ou colecao Postman atualizada
- [ ] Comentarios Doxygen atualizados quando houve mudanca de interface publica ou contrato interno relevante
- [ ] Conteudo gerado ou apoiado por IA foi revisado por uma pessoa responsavel pelo modulo
```

### Responsabilidades

- Autor da mudanca: atualiza a documentacao afetada.
- Revisor tecnico: valida se a documentacao esta coerente com o codigo.
- Mantenedor do modulo: decide se a mudanca exige ADR ou runbook.
- Responsavel de release: valida documentos de release, assinatura e OTA.
- IA: apoia levantamento, sumarios, primeira versao de textos e identificacao de lacunas.
- Validador humano: confirma fatos, corrige contexto e aprova a publicacao.

## Politica leve

Para nao burocratizar o desenvolvimento, a documentacao deve seguir niveis:

- Nivel 0: nada a documentar quando nao ha mudanca de comportamento, processo ou contrato.
- Nivel 1: nota curta no PR quando a informacao e temporaria ou so ajuda a revisao.
- Nivel 2: atualizacao pequena em documento existente, que deve ser o caso mais comum.
- Nivel 3: novo documento apenas para assuntos recorrentes, criticos ou amplos.

A pergunta principal e: alguem no futuro precisara desta informacao para desenvolver, operar, testar ou manter o projeto? Se nao, nao crie burocracia.
## Docs as Code

A abordagem oficial recomendada e docs as code. A documentacao deve ser tratada como parte versionada do produto:

- Markdown, configuracoes MkDocs e configuracoes Doxygen vivem no Git.
- Mudancas passam por branch, pull request e revisao humana.
- Validacoes automatizadas verificam estrutura, links, MkDocs e Doxygen.
- Publicacao deve ser automatica ou reproduzivel.
- Conteudo gerado por IA e apenas rascunho ate ser validado por uma pessoa responsavel.

Essa abordagem permite historico, rastreabilidade, qualidade e melhoria continua sem criar um processo paralelo desconectado do desenvolvimento.
## Doxygen e MkDocs

O modelo recomendado e separar responsabilidades:

- MkDocs: portal principal, guias, tutoriais, arquitetura, processos, runbooks, ADRs e referencias operacionais.
- Doxygen: referencia tecnica gerada a partir do codigo, principalmente classes, structs, funcoes, interfaces, arquivos e relacoes entre modulos.

O MkDocs deve ser a porta de entrada para humanos. O Doxygen deve ser linkado a partir do MkDocs quando a leitura exigir detalhe de implementacao.

Documentos narrativos nao devem ser substituidos por Doxygen. Comentarios Doxygen devem explicar contrato, responsabilidade, parametros, retorno, efeitos colaterais e invariantes relevantes. Nao devem apenas repetir o nome da funcao.

## Uso de IA com verificacao humana

IA pode ser usada para:

- Mapear arquivos, modulos e dependencias aparentes.
- Gerar primeira versao de mapas de arquitetura.
- Resumir responsabilidades de arquivos ou subsistemas.
- Identificar lacunas de documentacao.
- Sugerir comentarios Doxygen iniciais.
- Criar rascunhos de guias, runbooks e ADRs.

IA nao deve ser usada sem validacao para:

- Afirmar comportamento de negocio.
- Documentar seguranca, assinatura, OTA, Nagra, chaves, criptografia ou release.
- Definir contratos de API.
- Registrar decisoes arquiteturais finais.
- Descrever comportamento de hardware ou plataforma sem confirmacao.

Todo conteudo apoiado por IA deve passar por revisao humana com foco em:

- Correta leitura do codigo.
- Confirmacao por pessoa responsavel pelo modulo.
- Teste ou reproducao dos comandos documentados.
- Remocao de suposicoes nao verificadas.
- Registro de pendencias quando houver incerteza.

## Melhoria continua

Use um ciclo mensal ou por sprint:

```text
Coletar lacunas -> Priorizar -> Documentar -> Revisar -> Medir -> Ajustar processo
```

Fontes de lacunas:

- Duvidas repetidas no time.
- Falhas de build ou release.
- Incidentes em assinatura, update, USB, OTA ou ambiente.
- PRs com discussoes longas por falta de contexto.
- Arquivos/scripts usados com frequencia mas sem explicacao.
- Modulos com alta criticidade e baixa cobertura documental.

Metricas sugeridas:

- Tempo para novo desenvolvedor preparar ambiente e compilar.
- Numero de perguntas recorrentes sobre build, release e debug.
- Quantidade de documentos revisados no mes.
- Quantidade de paginas sem revisao ha mais de 90 dias.
- PRs que alteram comportamento sem atualizar documentacao.
- Falhas de release causadas por procedimento incompleto ou desatualizado.
- Cobertura Doxygen em interfaces publicas ou modulos criticos.
- Numero de paginas MkDocs revisadas dentro do prazo.
- Numero de rascunhos gerados por IA aprovados, corrigidos ou rejeitados.

Cadencia:

- Semanal: registrar lacunas encontradas.
- Por PR: validar checklist de documentacao.
- Por release: revisar documentos de release e operacao.
- Mensal: priorizar 3 a 5 melhorias documentais.
- Trimestral: revisar arquitetura, mapas de modulo e runbooks criticos.

## Padrao de escrita

- Escrever para execucao: comandos, pre-requisitos, entradas, saidas e verificacao.
- Preferir exemplos reais do repositorio.
- Explicar contexto apenas quando ajuda a tomar decisao.
- Usar nomes reais de arquivos, targets, flags e scripts.
- Manter documentos curtos; dividir quando passarem a misturar assuntos.
- Registrar data de revisao em documentos operacionais criticos.

## Ferramentas recomendadas

Inicio simples:

- Markdown no repositorio.
- MkDocs Material para portal interno.
- Doxygen para referencia C/C++ gerada.
- `markdownlint` para estilo.
- `lychee` ou ferramenta equivalente para links quebrados.
- `codespell` ou dicionario tecnico para erros recorrentes.

Quando a documentacao crescer:

- Integracao entre MkDocs e saida HTML do Doxygen.
- Publicacao automatica do portal em ambiente interno.
- Relatorios de cobertura ou warnings do Doxygen no CI.
- Docusaurus apenas se houver necessidade publica ou versionamento mais complexo de produto.
- Backstage se a empresa ja usa catalogo interno de servicos.

## Proximas acoes

1. Criar README principal na raiz apontando para `docs/plano/`.
2. Criar `docs/plano/reference/repository-map.md` com o mapa dos diretorios atuais.
3. Criar `docs/plano/getting-started/build-debug.md` usando `configure.sh` e toolchain Montage.
4. Criar `docs/plano/getting-started/build-release.md` usando `configure-release.sh` e toolchain ALi.
5. Criar `docs/plano/release/signing-nagra.md` consolidando o conteudo de `extra/signed/Readme.md`.
6. Criar primeiro ADR em `docs/plano/decisions/0001-use-repository-docs.md`.
7. Adicionar checklist de documentacao ao template de PR.
8. Criar `docs/plano/process/docs-as-code.md` e usar como processo oficial.
9. Criar `docs/plano/management/documentation-management-plan.md` como plano de gestao.
10. Definir configuracao inicial de MkDocs.
11. Definir configuracao inicial de Doxygen.
12. Executar um piloto de documentacao assistida por IA em um modulo de baixo risco, com revisao humana registrada.











