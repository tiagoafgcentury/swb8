# Boas praticas para arquiteto ou analista responsavel por documentacao de software

## Objetivo

Orientar o profissional responsavel por levantar, organizar, revisar e manter a documentacao tecnica do software.

Este papel nao deve apenas "escrever documentos". A responsabilidade principal e transformar conhecimento tecnico disperso em informacao confiavel, revisavel e util para desenvolvimento, QA, operacao, suporte, release e gestao.

## Papel esperado

O arquiteto ou analista de documentacao atua como ponte entre:

- Desenvolvedores e arquitetos.
- QA e validacao.
- Operacao, suporte e release.
- Gestao tecnica e gestao executiva.
- Codigo-fonte, processos reais e documentacao oficial.

O foco deve ser reduzir dependencia de conhecimento oral, acelerar onboarding, diminuir retrabalho e tornar decisoes tecnicas rastreaveis.

## Responsabilidades principais

- Identificar quais conhecimentos precisam virar documentacao permanente.
- Priorizar documentos por risco, recorrencia, criticidade e impacto no time.
- Entrevistar desenvolvedores sem interromper excessivamente o fluxo de trabalho.
- Validar informacoes tecnicas com donos dos modulos.
- Separar fato confirmado, inferencia e pendencia.
- Manter documentacao proxima do codigo e do processo real.
- Criar modelos reutilizaveis para guias, runbooks, ADRs e referencias.
- Garantir que mudancas relevantes de codigo atualizem a documentacao.
- Evitar burocracia documental desnecessaria.

## Principios de trabalho

### Documentar para uso real

Cada documento deve responder a uma necessidade concreta:

- Como compilar?
- Como testar?
- Como diagnosticar problema?
- Como um modulo funciona?
- Qual contrato uma interface garante?
- Por que uma decisao foi tomada?
- O que muda em uma release?

Se o documento nao ajuda alguem a executar, decidir, revisar ou manter, ele deve ser simplificado ou removido.

### Nao substituir o codigo

Documentacao nao deve repetir linha a linha o que o codigo ja mostra. Ela deve explicar:

- Responsabilidades.
- Fluxos.
- Contratos.
- Premissas.
- Dependencias.
- Riscos.
- Efeitos colaterais.
- Decisoes e tradeoffs.

Referencia detalhada de funcoes deve ficar preferencialmente em Doxygen ou comentarios tecnicos proximos do codigo.

### Trabalhar com niveis de confiabilidade

Use marcadores claros:

- `Confirmado`: validado por dono tecnico ou por teste.
- `Inferido`: deduzido a partir do codigo, ainda sem validacao.
- `Pendente`: informacao incompleta ou aguardando confirmacao.
- `Obsoleto`: conteudo conhecido como desatualizado.

Isso permite publicar conhecimento util sem fingir certeza onde ela ainda nao existe.

### Menos documento novo, mais documento certo

Antes de criar um novo arquivo, verifique se o assunto cabe em documento existente. Muitos documentos pequenos e desconectados reduzem a utilidade da base.

Crie novo documento quando o tema for recorrente, critico, extenso ou tiver audiencia propria.

## Skills tecnicas necessarias

### Leitura de codigo

O responsavel nao precisa ser o principal desenvolvedor do sistema, mas deve conseguir ler codigo o suficiente para:

- Identificar modulos, entradas e saidas.
- Entender chamadas principais.
- Reconhecer contratos de interfaces.
- Localizar configuracoes, flags e scripts.
- Diferenciar comportamento real de intencao declarada.

Para este projeto, e importante ter familiaridade com:

- C e C++.
- CMake.
- Shell scripts.
- Sistemas embarcados.
- Logs, build e debug em ambiente alvo.
- Conceitos de HAL, tasks, DVB, CAS, UI e update de software.

### Arquitetura de software

Skills esperadas:

- Modelagem de componentes.
- Separacao de responsabilidades.
- Identificacao de dependencias.
- Leitura de fluxo entre camadas.
- Registro de decisoes arquiteturais.
- Capacidade de explicar tradeoffs tecnicos.


### Engenharia de contexto

Com IA no fluxo de desenvolvimento, uma skill essencial e saber montar contexto de qualidade. O profissional deve transformar codigo, conversas, PRs, tickets e decisoes em blocos de contexto claros, rastreaveis e verificaveis.

Skills esperadas:

- Selecionar os arquivos e documentos certos para cada pergunta tecnica.
- Separar contexto essencial de ruido.
- Explicitar premissas, restricoes e pendencias.
- Preparar prompts com objetivo, publico-alvo, fonte e nivel de confianca.
- Criar documentos que possam ser reutilizados por humanos e assistentes de IA.
- Validar respostas geradas por IA contra codigo, testes e donos tecnicos.

### Documentacao como codigo

Skills esperadas:

- Markdown.
- Git e fluxo de pull request.
- MkDocs ou ferramenta equivalente.
- Doxygen para referencia de codigo.
- Revisao por pares.
- Organizacao por taxonomia, como Diataxis.

### Analise de requisitos e processos

Skills esperadas:

- Levantamento de informacoes.
- Entrevistas tecnicas.
- Mapeamento de fluxos.
- Escrita de criterios de aceite.
- Identificacao de excecoes e cenarios de erro.
- Separacao entre regra de negocio, regra tecnica e detalhe de implementacao.

### Qualidade e verificacao

Skills esperadas:

- Conferir comandos documentados.
- Validar exemplos.
- Reproduzir procedimentos.
- Identificar lacunas.
- Criar checklists objetivos.
- Trabalhar com QA para transformar comportamento esperado em cenarios verificaveis.

### Comunicacao tecnica

Skills esperadas:

- Fazer perguntas objetivas.
- Resumir assuntos complexos sem distorcer.
- Escrever para diferentes publicos.
- Evitar ambiguidade.
- Registrar decisoes de forma neutra.
- Negociar tempo com desenvolvedores sem criar atrito.

### Facilitacao com desenvolvedores

Esta e uma skill central. O responsavel precisa conseguir abordar desenvolvedores de forma objetiva, respeitando contexto e tempo.

Boas praticas:

- Chegar com contexto previo lido.
- Fazer perguntas especificas.
- Evitar reunioes longas para temas simples.
- Levar um rascunho para o desenvolvedor corrigir, em vez de pedir que ele escreva tudo do zero.
- Pedir confirmacao de pontos tecnicos criticos.
- Registrar pendencias imediatamente.
- Transformar respostas em documento revisavel.


### Investigar antes de documentar ou implementar

Uma boa abordagem com IA e com desenvolvedores e comecar pela compreensao do sistema, nao pela solucao. Antes de pedir codigo, texto final ou correcao, o responsavel deve levantar:

- Onde o fluxo acontece.
- Quais arquivos, scripts, tasks, telas ou servicos participam.
- Quais testes ou roteiros de QA ja cobrem o comportamento.
- Quais documentos existentes precisam ser atualizados.
- Quais impactos laterais podem existir.
- Quais pontos exigem validacao humana.

Essa postura melhora a qualidade da documentacao porque evita registrar uma solucao isolada sem entender o contexto real.

### Registrar aprendizado reutilizavel

Ao final de uma investigacao, bug, release ou revisao, avalie se algo deve virar conhecimento permanente:

- Uma pergunta repetida vira prompt reutilizavel.
- Uma regra de revisao vira diretriz ou instruction.
- Um fluxo tecnico vira runbook, skill ou template.
- Uma validacao critica vira checklist ou gate.
- Uma decisao recorrente vira ADR.

O papel do arquiteto ou analista de documentacao inclui decidir o que merece virar aprendizado reutilizavel e o que deve permanecer apenas como nota temporaria.

## Como abordar desenvolvedores

### Antes da conversa

Prepare-se:

- Leia arquivos relevantes.
- Identifique o modulo e seus donos provaveis.
- Liste duvidas objetivas.
- Verifique documentos existentes.
- Separe o que voce ja sabe do que precisa confirmar.

Evite perguntas genericas como:

```text
Como esse modulo funciona?
```

Prefira perguntas especificas:

```text
Qual task e responsavel por iniciar a sintonia?
Quais eventos fazem esse fluxo sair do estado de espera?
Essa funcao pode ser chamada antes da inicializacao do HAL?
O que acontece se a tabela DVB vier incompleta?
Esse erro deve gerar retry, fallback ou falha fatal?
```

### Durante a conversa

Conduza a conversa por blocos:

- Objetivo do modulo.
- Entradas e saidas.
- Fluxo principal.
- Estados e excecoes.
- Dependencias externas.
- Logs e diagnostico.
- Riscos conhecidos.
- Testes ou validacoes existentes.
- Mudancas recentes.
- Pontos ainda incertos.

Confirme sempre:

- O que e comportamento atual.
- O que e comportamento desejado.
- O que e workaround.
- O que e legado.
- O que esta planejado para mudar.

### Depois da conversa

Transforme a conversa em artefato:

- Atualize ou crie o documento.
- Marque pontos pendentes.
- Abra PR ou tarefa.
- Solicite revisao do dono tecnico.
- Linke documento ao Redmine, PR ou decisao relacionada.

O desenvolvedor deve revisar fatos tecnicos, nao ser obrigado a reescrever o documento inteiro.

## Perguntas-chave para desenvolvedores

### Sobre modulo

- Qual problema este modulo resolve?
- Quem chama este modulo?
- Quais modulos ele chama?
- Quais dados entram e quais dados saem?
- Quais estados internos sao relevantes?
- Existem efeitos colaterais importantes?
- O modulo depende de ordem de inicializacao?

### Sobre contrato

- O que a funcao ou classe garante?
- Quais parametros podem ser nulos, vazios ou invalidos?
- Qual e o comportamento em erro?
- Ha timeout, retry ou fallback?
- A chamada e sincrona ou assincrona?
- A funcao e thread-safe?
- Existe dependencia de hardware, plataforma ou configuracao?

### Sobre fluxo

- Qual e o caminho feliz?
- Quais sao os principais caminhos de erro?
- Como o sistema se recupera?
- Quais logs indicam sucesso?
- Quais logs indicam falha?
- Como reproduzir manualmente?
- Existe teste automatizado ou roteiro de QA?

### Sobre decisao tecnica

- Que alternativas foram consideradas?
- Por que esta abordagem foi escolhida?
- Quais tradeoffs foram aceitos?
- Que risco ficou pendente?
- Quando essa decisao deve ser revista?

### Sobre release e operacao

- O que precisa ser validado antes da entrega?
- Quais arquivos ou artefatos sao gerados?
- O que pode quebrar em campo?
- Existe rollback?
- O que suporte precisa saber?
- Quais logs ou comandos ajudam no diagnostico?

## Tipos de documentos recomendados

### Guia tecnico

Usado para ensinar uma tarefa recorrente, como build, debug, configuracao de ambiente ou integracao.

Deve conter:

- Objetivo.
- Pre-requisitos.
- Passos.
- Saida esperada.
- Problemas comuns.
- Como validar.

### Referencia tecnica

Usada para consulta rapida de parametros, scripts, flags, estruturas, interfaces ou comandos.

Deve conter:

- Nome.
- Descricao.
- Entrada.
- Saida.
- Exemplo.
- Restricoes.
- Dono ou fonte.

### Runbook

Usado para operacao, release, update, diagnostico ou resposta a incidente.

Deve conter:

- Quando usar.
- Pre-requisitos.
- Passos.
- Validacao.
- Rollback.
- Escalonamento.
- Riscos.

### ADR

Usado para decisoes arquiteturais relevantes.

Deve conter:

- Contexto.
- Decisao.
- Alternativas.
- Consequencias.
- Status.
- Data.

### Visao de arquitetura

Usada para explicar estrutura, responsabilidades e fluxos de alto nivel.

Deve conter:

- Diagrama ou mapa textual.
- Componentes.
- Responsabilidades.
- Dependencias.
- Fluxos principais.
- Riscos e pontos de extensao.

## Checklist de qualidade de um documento

Antes de considerar pronto, valide:

- O publico-alvo esta claro.
- O documento tem objetivo explicito.
- O conteudo esta no lugar certo.
- Comandos e caminhos foram conferidos.
- Informacoes incertas estao marcadas.
- O dono tecnico revisou fatos criticos.
- Nao ha copia desnecessaria do codigo.
- Ha links para documentos relacionados.
- O documento informa como validar o resultado.
- O texto e direto, sem ambiguidade.

## Checklist para revisao com desenvolvedor

Use este roteiro em revisoes tecnicas:

```md
## Revisao tecnica

- [ ] O objetivo do modulo/processo esta correto
- [ ] O fluxo principal esta correto
- [ ] Os fluxos de erro relevantes foram cobertos
- [ ] Os nomes de arquivos, funcoes e scripts estao corretos
- [ ] As dependencias externas foram citadas
- [ ] Os comandos ou passos foram validados
- [ ] As limitacoes conhecidas foram registradas
- [ ] As pendencias estao marcadas
- [ ] O conteudo pode ser publicado para o publico-alvo definido
```

## Antipadroes a evitar

- Criar documento sem dono.
- Documentar suposicao como se fosse fato.
- Pedir para o desenvolvedor escrever tudo do zero.
- Fazer reunioes longas sem pauta.
- Escrever documentacao que apenas repete o codigo.
- Criar documentos isolados sem link com a estrutura oficial.
- Usar IA sem revisao humana em assuntos criticos.
- Manter documentos obsoletos sem marcacao.
- Bloquear desenvolvimento por exigencia documental desproporcional.
- Documentar apenas depois que o conhecimento ja foi perdido.

## Cadencia recomendada

### Semanal

- Revisar lacunas novas.
- Conversar com donos de modulos criticos.
- Atualizar documentos afetados por PRs recentes.
- Fechar pendencias pequenas.

### Por sprint

- Priorizar documentos que reduzem duvidas recorrentes.
- Revisar documentacao de funcionalidades alteradas.
- Atualizar backlog documental.
- Medir documentos pendentes de revisao.

### Por release

- Revisar runbooks de release, update e rollback.
- Confirmar artefatos gerados.
- Validar comandos.
- Garantir que suporte e QA possuem informacoes necessarias.

## Indicadores uteis

- Quantidade de documentos com status `Pendente`.
- Quantidade de documentos revisados por dono tecnico.
- Numero de duvidas recorrentes transformadas em documentacao.
- Tempo para novo desenvolvedor realizar build inicial.
- Numero de PRs com checklist documental preenchido.
- Numero de runbooks criticos validados antes da release.
- Warnings de MkDocs, Doxygen ou validadores de Markdown.

## Matriz de skills esperadas

| Skill | Nivel esperado | Uso pratico |
| --- | --- | --- |
| Leitura de codigo C/C++ | Intermediario | Confirmar comportamento real e localizar contratos |
| Arquitetura de software | Intermediario a avancado | Explicar modulos, dependencias e decisoes |
| Markdown e Git | Intermediario | Trabalhar com docs as code e PRs |
| Doxygen | Basico a intermediario | Documentar interfaces proximas do codigo |
| MkDocs | Basico a intermediario | Publicar documentacao navegavel |
| Entrevista tecnica | Avancado | Extrair conhecimento dos desenvolvedores |
| Analise de processos | Intermediario | Mapear fluxos e responsabilidades |
| QA e validacao | Basico a intermediario | Transformar documentacao em passos verificaveis |
| Comunicacao tecnica | Avancado | Escrever com clareza para publicos diferentes |
| Gestao de backlog | Intermediario | Priorizar lacunas documentais |
| Uso responsavel de IA | Intermediario | Gerar rascunhos com verificacao humana |
| Engenharia de contexto | Intermediario a avancado | Preparar informacao reutilizavel para humanos e IA |

## Fluxo pratico recomendado

1. Escolher tema por risco ou recorrencia.
2. Ler codigo, scripts e documentos existentes.
3. Criar rascunho com fatos confirmados e pendencias.
4. Entrevistar desenvolvedor com perguntas objetivas.
5. Atualizar documento.
6. Solicitar revisao tecnica.
7. Publicar via PR.
8. Linkar documento em indice, Redmine ou tarefa relacionada.
9. Revisar periodicamente conforme mudancas de codigo.

## Definicao de pronto

Um documento tecnico esta pronto quando:

- A necessidade de uso esta clara.
- O conteudo foi validado por fonte confiavel.
- Pendencias estao visiveis.
- O documento esta versionado.
- O documento foi linkado na estrutura oficial.
- O time consegue usar o conteudo sem explicacao oral adicional.



