# Processo Redmine, GitLab e Docs as Code

## Objetivo

Definir um fluxo simples, rastreavel e revisavel para controlar demandas no Redmine e executar mudancas no GitLab, incluindo codigo, documentacao, revisao, validacao, release e fechamento.

A regra central e:

```text
Redmine controla o trabalho.
GitLab controla a mudanca.
Merge Request valida e registra a entrega.
Docs as Code garante documentacao versionada e revisada.
```

## Principios

- Toda demanda relevante deve nascer no Redmine.
- Toda mudanca deve ser feita em branch propria.
- Push direto em `main` ou `stable` nao deve ser permitido.
- Toda integracao deve passar por Merge Request.
- O MR deve informar o que mudou, como validar e quais riscos existem.
- Documentacao deve ser atualizada no mesmo fluxo quando houver impacto.
- Redmine deve ser atualizado com branch, MR, build, evidencias e status.
- Release deve ser promovido de forma controlada.

## Fluxo resumido

```text
1. Abrir issue no Redmine
2. Triar, priorizar e incluir em sprint ou backlog
3. Desenvolvedor valida entendimento da demanda
4. Criar branch no GitLab vinculada a issue
5. Implementar codigo e/ou documentacao
6. Abrir Merge Request para main
7. Revisar, testar e aprovar
8. Fazer merge em main
9. Atualizar Redmine e disponibilizar para QA/release
10. Quando aplicavel, abrir MR de promocao de `main` para `stable`
11. No MR/merge para `stable`, gerar tag e registrar evidencias do release
12. Gerar o release a partir da `stable`
13. Fechar issue no Redmine
```

## Papel de cada ferramenta

| Ferramenta | Papel |
| --- | --- |
| Redmine | Demanda, prioridade, sprint, responsavel, prazo, status e historico gerencial |
| GitLab Branch | Isolar a mudanca relacionada a uma issue |
| GitLab Merge Request | Revisao tecnica, discussao, CI, aprovacao e merge |
| GitLab CI | Validacao automatica de build, testes, MkDocs e Doxygen |
| MkDocs | Portal oficial da documentacao |
| Doxygen | Referencia tecnica gerada a partir do codigo |
| GitLab Issue, se usada | Evitar duplicidade; preferir Redmine como origem oficial |

## Branches principais

| Branch | Uso |
| --- | --- |
| `main` | Desenvolvimento integrado e base para novas branches |
| `stable` | Base de release validado |

Fluxo recomendado:

```text
branch de trabalho -> MR para main -> validacao -> MR de main para stable -> merge em stable -> tag/evidencias -> release a partir da stable
```

## Quando abrir issue no Redmine

Abrir issue para qualquer trabalho que precise ser planejado, rastreado ou revisado.

Exemplos:

- Bug.
- Nova funcionalidade.
- Ajuste tecnico relevante.
- Documento novo.
- Atualizacao de documento existente.
- ADR.
- Runbook.
- Melhoria de processo.
- Tarefa de validacao ou revisao humana.

Nao precisa abrir issue separada para:

- Ajuste trivial dentro de uma tarefa ja existente.
- Correcao pequena durante revisao do mesmo MR.
- Pequenos ajustes de texto dentro da mesma entrega.

## Padrao de issue no Redmine

### Titulo

Use titulo claro e acionavel.

Exemplos:

```text
[Docs] Documentar build release do B8
[Bug] Corrigir falha na atualizacao via USB
[Feature] Ajustar regras de lista de canais Claro
[Review] Validar documentacao de assinatura Nagra
```

### Descricao minima

```md
## Objetivo
Descrever o que precisa ser feito.

## Contexto
Explicar motivo, origem da demanda e impacto.

## Escopo
O que entra nesta tarefa.

## Fora de escopo
O que nao sera tratado agora.

## Criterios de aceite
- [ ] Criterio 1
- [ ] Criterio 2
- [ ] Criterio 3

## Evidencias ou fontes
- Arquivos relacionados:
- Documento ou processo relacionado:
- Link externo, se houver:

## Documentacao
- [ ] Nao exige documentacao
- [ ] Atualizar documentacao existente
- [ ] Criar novo documento
- [ ] Atualizar Doxygen
```

### Campos minimos para bug reportado por QA/teste

Para bug de release, QA ou teste de bancada, preencher no minimo:

- Release/build testado.
- Ambiente de teste.
- Equipamento, chipset ou plataforma.
- Passos para reproduzir.
- Resultado atual.
- Resultado esperado.
- Frequencia do problema.
- Evidencias: video, foto, log, banco, lista ou arquivo usado.
- Comparacao com release anterior, quando possivel.

## Classificacao recomendada no Redmine

### Tipo ou tracker

- Bug: defeito ou regressao.
- Task: trabalho planejado comum.
- Feature: nova capacidade.
- Docs: criacao ou ajuste de documentacao.
- Review: revisao humana, tecnica ou operacional.
- Improvement: melhoria sem urgencia.

Se o Redmine atual nao tiver esses trackers, adaptar para os tipos existentes.

### Prioridade

| Prioridade | Uso recomendado |
| --- | --- |
| Alta | Bloqueia release, build, operacao critica, QA ou conhecimento essencial |
| Media | Importante, mas nao bloqueante |
| Baixa | Melhoria incremental ou organizacao futura |

### Status sugeridos

Fluxo geral:

```text
Novo -> Em analise -> Pronto para desenvolvimento -> Em desenvolvimento -> Em revisao -> Em validacao -> Resolvido -> Fechado
```

Fluxo para documentacao:

```text
Novo -> Em levantamento -> Em redacao -> Em revisao -> Publicado -> Fechado
```

Estados adicionais uteis:

| Status | Quando usar |
| --- | --- |
| Reaberto | Problema persiste ou houve regressao |
| Bloqueado | Falta informacao, ambiente, decisao ou dependencia |
| Em validacao | Correcao disponivel para QA ou teste |

## Sprint no Redmine

A sprint deve agrupar demandas aprovadas para o ciclo.

Ao montar a sprint:

- Definir nome com periodo, release ou objetivo.
- Incluir bugs, features, documentacao e melhorias aprovadas.
- Confirmar prioridade de execucao.
- Confirmar responsaveis.
- Confirmar prazo previsto.
- Evitar tarefa sem criterio de aceite.
- Evitar bug sem release/build afetado, quando aplicavel.

Exemplo:

```text
Sprint R1.2.3 - Correcao de bugs release candidato
```

## Validacao de entendimento pelo desenvolvedor

Antes de criar branch ou iniciar alteracao, o desenvolvedor deve confirmar que entendeu a demanda.

Checklist minimo:

- Entendi o problema ou objetivo.
- Entendi o release/build afetado, se aplicavel.
- Entendi o ambiente onde o problema foi reproduzido.
- Entendi os passos para reproduzir ou sei o que falta.
- Entendi o resultado atual e o resultado esperado.
- Entendi as evidencias existentes.
- Entendi quais modulos ou fluxos podem estar envolvidos.
- Entendi como QA ou o revisor devera validar a entrega.
- Entendi se existe impacto em documentacao.

Se houver duvida, comentar no Redmine antes de iniciar:

```text
Antes de iniciar, preciso confirmar:
- Qual build exato foi testado?
- O problema acontece sempre ou foi intermitente?
- Qual lista/banco foi usado?
- Existe video ou log do momento da falha?
```

## Criacao da branch no GitLab

Criar branch a partir da branch base combinada pelo time, normalmente `main`.

Padrao recomendado:

```text
<tipo>/redmine-<id>-<resumo-curto>
```

Exemplos:

```text
fix/redmine-8492-editar-chave-novo-satelite
docs/redmine-124-build-release
feature/redmine-7332-fotos-configuracoes
hotfix/redmine-8510-reboot-instala-facil
```

Tambem e aceitavel, se o time ja usa esse padrao:

```text
feature/<redmine-id>-<descricao-curta>
bugfix/<redmine-id>-<descricao-curta>
hotfix/<redmine-id>-<descricao-curta>
```

Boas praticas:

- Uma branch deve resolver uma issue ou um grupo pequeno de issues relacionadas.
- Evitar branch grande demais.
- Manter o nome da branch ligado ao ID Redmine.
- Atualizar a branch com a base antes do merge quando necessario.

## Commits

Use mensagens objetivas e vinculadas ao Redmine.

Formato recomendado:

```text
<tipo>: resumo curto refs #<id>
```

Exemplos:

```text
docs: document release build refs #124
fix: validate USB update package refs #123
feat: add channel list rule handling refs #125
refactor: simplify HAL tuner setup refs #126
```

Tipos comuns:

- `fix`: correcao de bug.
- `feat`: nova funcionalidade.
- `docs`: documentacao.
- `test`: testes.
- `refactor`: refatoracao sem mudanca funcional.
- `build`: build, CMake, toolchain ou CI.
- `chore`: manutencao.

## Merge Request no GitLab

Todo trabalho relevante deve passar por Merge Request.

### Titulo do MR

```text
[Redmine #123] Corrigir validacao de update USB
```

Ou, se o time preferir o padrao antigo:

```text
#123 - Modulo: resumo da alteracao
```

### Descricao sugerida

```md
## Redmine
Refs #123

## Objetivo
Descrever a entrega.

## O que mudou
- Item 1
- Item 2

## Motivo
- Causa, contexto ou justificativa da alteracao.

## Como validar
- [ ] Build executado
- [ ] Teste manual executado
- [ ] QA informado, se aplicavel
- [ ] MkDocs validado, se aplicavel
- [ ] Doxygen validado, se aplicavel

## Documentacao
- [ ] Nao exige atualizacao de documentacao
- [ ] Documentacao atualizada
- [ ] Comentarios Doxygen atualizados
- [ ] Conteudo apoiado por IA revisado por humano

## Riscos
- Fluxos que podem regredir.

## Evidencias
- Logs, prints, videos, build, pacote ou observacoes.
```

## Revisao do MR

### Responsabilidades do autor

- Explicar claramente a mudanca.
- Manter o MR pequeno o suficiente para revisao.
- Linkar o Redmine.
- Atualizar documentacao quando aplicavel.
- Informar como testar.
- Responder comentarios de revisao.
- Garantir que CI passou ou justificar falha.
- Atualizar o Redmine com branch, MR, build e evidencias relevantes.

### Responsabilidades do revisor

- Validar se o MR esta ligado a uma tarefa do Redmine.
- Conferir se o escopo bate com a tarefa.
- Checar riscos e efeitos colaterais.
- Verificar se o plano de teste esta claro.
- Verificar se documentacao foi tratada corretamente.
- Pedir ADR quando houver decisao duradoura.
- Aprovar apenas quando criterios de aceite estiverem atendidos.

## Documentacao no fluxo Docs as Code

Usar a politica leve:

| Nivel | Quando usar | Acao |
| --- | --- | --- |
| 0 | Sem impacto em conhecimento reutilizavel | Marcar que nao exige documentacao |
| 1 | Informacao temporaria | Nota no MR ou Redmine |
| 2 | Pequena mudanca em processo, comando ou regra | Atualizar documento existente |
| 3 | Tema recorrente, critico ou duradouro | Criar documento novo |

Exemplos de mudancas que exigem documentacao:

- Alteracao de build, toolchain, script ou flag CMake.
- Mudanca em release, update, assinatura, OTA ou TPM.
- Nova regra de Claro/Sky, canais, audio, CC ou diagnostico.
- Nova interface compartilhada entre modulos.
- Mudanca em procedimento de QA ou suporte.

A documentacao oficial deve ser alterada em Markdown dentro de `docs/`, revisada por MR e publicada via MkDocs. Comentarios tecnicos de codigo devem ser tratados via Doxygen quando aplicavel.

## Validacoes recomendadas

### Codigo

Executar conforme o tipo da mudanca:

```bash
cmake --build build
```

Ou outro comando definido pelo projeto.

### Documentacao MkDocs

Quando alterar `docs/` ou `mkdocs.yml`:

```bash
mkdocs build --strict
```

### Doxygen

Quando alterar comentarios Doxygen, headers relevantes ou `Doxyfile`:

```bash
doxygen Doxyfile
```

No inicio, warnings podem ser tratados como relatorio. Depois que a base amadurecer, alguns warnings podem virar bloqueantes.

## Atualizacao do Redmine apos correcao

Depois da correcao, o desenvolvedor deve atualizar a tarefa com:

- Branch.
- Link do MR.
- Commits relevantes.
- Build ou pacote onde a correcao esta disponivel.
- Resumo do que foi feito.
- Como QA deve validar.
- Riscos ou pendencias.
- Link da documentacao, se aplicavel.

Status sugerido:

- `Em validacao`: quando ainda precisa de QA.
- `Resolvido`: quando ja foi validado ou entregue no build correto.
- `Fechado`: quando release/entrega foi concluida e registrada.

## Release

### Preparacao

Antes de promover release:

- Confirmar quais tarefas Redmine entram no release.
- Confirmar que os MRs correspondentes foram integrados em `main`.
- Confirmar que QA validou os itens obrigatorios.
- Confirmar que nao existem bugs bloqueantes abertos.
- Gerar changelog ou lista de tarefas entregues.
- Confirmar se documentacao de release, update ou operacao foi atualizada.

### Promocao para stable

O release oficial deve ser gerado a partir da branch `stable`. A promocao acontece por MR de `main` para `stable`. No momento desse MR e do merge em `stable`, devem ser registradas a tag e as evidencias do release.

```text
main pronta para release
-> abrir MR de main para stable
-> revisar lista de tarefas Redmine
-> validar build/release candidato
-> merge em stable
-> gerar tag na stable
-> registrar evidencias do release no MR e no Redmine
-> gerar pacote de release a partir da stable
-> atualizar tarefas Redmine
```

Padrao de tag sugerido:

```text
v<versao>
```

Exemplo:

```text
v1.2.3
```

### Artefatos de release

Como o release e gerado a partir da `stable`, registrar no Redmine e no MR de promocao:

- `.b8` para atualizacao manual pelo menu do receptor.
- `.bin` para atualizacao automatica ao iniciar o receptor.
- Assinatura Nagra.
- Pacote OTA `.ts`.
- Branch base do release: `stable`.
- Tag gerada na `stable`.
- Build, data, responsavel e evidencias de validacao.

## Fechamento da issue no Redmine

Antes de fechar:

- MR aprovado e mergeado.
- CI aprovado ou justificativa registrada.
- Criterios de aceite atendidos.
- QA/revisor validou quando aplicavel.
- Documentacao atualizada ou marcado que nao se aplica.
- Evidencias anexadas ou linkadas.
- Versao/release informada, quando aplicavel.

Comentario de fechamento sugerido:

```md
Entrega concluida.

MR: <link>
Branch: <nome>
Build/release: <identificacao>
Validacao: <resumo>
Documentacao: <link para docs ou nao aplicavel>
Observacoes: <se houver>
```

## Papeis e responsabilidades

| Papel | Faz | Nao faz |
| --- | --- | --- |
| Testador/QA | Abre bug com evidencias, valida build, confirma correcao | Nao deve abrir bug sem release, passos ou resultado esperado |
| Desenvolvedor | Analisa, cria branch, corrige, testa, abre MR e atualiza Redmine | Nao faz push direto em `main` ou `stable` |
| Revisor/Maintainer | Revisa MR, aprova merge e controla integracao | Nao aprova MR sem contexto minimo e plano de teste |
| Release Manager | Organiza release, promove para `stable`, gera tag e pacote | Nao promove release sem lista de tarefas e validacao |
| Owner/Gestao | Prioriza sprint, acompanha status e aprova encerramento | Nao substitui o fluxo tecnico do time |
| Responsavel de documentacao | Garante docs as code, MkDocs, Doxygen e revisao humana | Nao publica conteudo critico sem validacao tecnica |

## Troubleshooting

| Situacao | Causa provavel | Acao recomendada |
| --- | --- | --- |
| Bug sem informacao suficiente | Tarefa aberta sem dados minimos | Devolver para complemento antes de desenvolver |
| Dev nao entendeu 100% da tarefa | Passos, ambiente, evidencia ou resultado esperado incompletos | Comentar no Redmine com perguntas objetivas |
| MR sem contexto suficiente | Descricao incompleta | Atualizar MR com Redmine, escopo, teste e riscos |
| Alteracao feita direto em branch principal | Fluxo ignorado | Reorganizar em branch e MR antes de integrar |
| Tarefa corrigida mas QA nao consegue validar | Falta de build, passos ou evidencia | Atualizar Redmine com build e roteiro de validacao |
| Release saiu fora do controle | Promocao fora da `stable` ou sem lista de tarefas | Retomar fluxo oficial e registrar tag/build no Redmine |
| Documentacao desatualizada apos mudanca | Checklist de docs ignorado | Atualizar `docs/` no MR ou abrir tarefa Redmine vinculada |
| Conteudo gerado por IA parece incorreto | Falta de revisao humana | Marcar como pendente e solicitar revisao do responsavel |

## Boas praticas gerais

- Redmine deve ser a origem da demanda.
- Todo MR relevante deve citar o ID Redmine.
- Evitar MRs grandes e misturados.
- Separar refactor de mudanca funcional quando possivel.
- Atualizar documentacao no mesmo MR quando a mudanca exigir.
- Usar labels no GitLab para facilitar triagem.
- Registrar pendencias explicitamente, nao deixar conhecimento implicito.
- Nao fechar issue sem evidencia minima de validacao.
- Preferir criterios de aceite objetivos.
- Aplicar docs as code sem burocratizar mudancas triviais.

## Antipadroes a evitar

- Implementar sem issue Redmine para trabalho relevante.
- Abrir MR sem contexto ou sem criterio de validacao.
- Misturar muitas demandas independentes no mesmo MR.
- Atualizar documentacao fora do fluxo de revisao.
- Deixar Redmine fechado sem link do MR.
- Usar IA para gerar conteudo tecnico sem revisao humana.
- Criar processo pesado para mudancas triviais.
- Promover release sem lista de tarefas, QA e tag.

## Exemplo completo de documentacao

```text
1. Redmine #124 criado: [Docs] Documentar build release do B8
2. Branch criada: docs/redmine-124-build-release
3. Documento alterado: docs/plano/getting-started/build-release.md
4. Validacao local: mkdocs build --strict
5. MR aberto: [Redmine #124] Documentar build release
6. Revisor tecnico valida comandos e conteudo
7. MR aprovado e mergeado em main
8. Redmine #124 fechado com link do MR e do documento publicado
```

## Exemplo completo de bug/release

```text
1. QA abre Redmine #8492 com build, ambiente, passos, resultado esperado e evidencias
2. Tarefa e triada e entra na sprint
3. Dev valida entendimento e cria bugfix/redmine-8492-editar-chave-novo-satelite
4. Dev corrige, testa e abre MR para main
5. Revisor valida escopo, risco e plano de teste
6. MR e mergeado em main
7. Redmine passa para Em validacao com build e roteiro de QA
8. QA valida correcao
9. Release manager abre MR de main para stable quando aplicavel
10. No MR/merge para stable, tag e evidencias do release sao registradas
11. Pacote e gerado a partir da stable
12. Redmine e fechado com evidencias
```


