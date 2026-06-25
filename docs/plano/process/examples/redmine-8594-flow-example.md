# Exemplo pratico: fluxo da issue Redmine #8594

## Issue de origem

Fonte: `D:/Meus Documentos/Desktop/midiabox-b8-sw-8594.pdf`

Resumo da issue:

| Campo | Valor |
| --- | --- |
| Redmine | Bug #8594 |
| Titulo | Receptor Monta Lista de Canais Sozinho |
| Projeto | SW - B8 Century |
| Criado por | Rafael Santo |
| Data | 23/06/2026 13:44 |
| Status observado no PDF | Pending test |
| Prioridade | Normal |
| Responsavel | Tiago Gomes |
| Target version | v1.04 |
| Detected on Version | v1.04 |
| Percentual observado no PDF | 100% |

## Problema reportado

O receptor monta a lista de canais automaticamente quando e colocado em stand-by durante a tela do Instala Facil.

## Passos para reproduzir

1. Atualizar um receptor para a versao 1.04.
2. Realizar um padrao de fabrica no receptor.
3. Conectar o cabo de sinal ao receptor.
4. Aguardar o receptor exibir a tela do Instala Facil.
5. Colocar o receptor em stand-by enquanto permanece na tela do Instala Facil.
6. Retirar o receptor do stand-by.
7. Verificar o comportamento do receptor.

## Resultado atual

O receptor monta a lista de canais automaticamente.

## Resultado esperado

O receptor deve retornar para a tela do Instala Facil toda vez que sair do stand-by e nao possuir uma lista de canais instalada.

## Frequencia

Acontece sempre que o receptor e colocado em stand-by com a lista de canais vazia.

## Observacao funcional importante

Quando o receptor entra em stand-by sem que o LNBF tenha sido detectado na tela do Instala Facil, os canais nao funcionam apos o receptor ser religado. Quando o LNBF e detectado antes de entrar em stand-by, os canais abrem normalmente apos o retorno.

## Fluxo pratico aplicado

```text
Redmine #8594
-> triagem e entendimento
-> branch bugfix/redmine-8594-instala-facil-standby-lista-vazia
-> correcao no codigo
-> MR para main
-> validacao tecnica e funcional
-> merge em main
-> Redmine em validacao/Pending test
-> MR de main para stable para release v1.04
-> tag na stable
-> evidencias de release no MR e no Redmine
-> release gerado a partir da stable
-> fechamento da issue
```

## 1. Redmine: entrada da demanda

A issue ja possui os elementos minimos para iniciar o fluxo:

- versao detectada: v1.04;
- versao alvo: v1.04;
- passos para reproduzir;
- resultado atual;
- resultado esperado;
- frequencia;
- observacao adicional sobre LNBF.

Ponto de atencao: as evidencias visuais estao marcadas como nao aplicaveis. Como o problema e reproduzivel sempre, isso pode ser aceito, mas o roteiro de validacao precisa ser claro.

Status sugerido antes do desenvolvimento:

```text
Novo -> Em analise -> Pronto para desenvolvimento
```

## 2. Validacao de entendimento pelo desenvolvedor

Antes de codar, o desenvolvedor deve confirmar:

- O problema ocorre apenas quando a lista de canais esta vazia?
- O comportamento esperado e sempre voltar para Instala Facil ao sair do stand-by sem lista instalada?
- A deteccao de LNBF muda o fluxo ou apenas evidencia outro efeito?
- O problema afeta Claro, Sky ou ambos?
- A correcao deve impedir salvar lista vazia, impedir montagem automatica ou ambos?
- Qual tela/estado deve ser restaurado ao retornar do stand-by?

Comentario exemplo no Redmine, se ainda houvesse duvida:

```text
Antes de iniciar a correcao, preciso confirmar:
- O comportamento esperado vale para todos os perfis de operadora?
- Devemos bloquear somente o salvamento de lista vazia ou tambem impedir o disparo da busca automatica?
- Existe algum caso em que a montagem automatica ao sair do stand-by seja esperada?
```

## 3. Branch no GitLab

Branch sugerida:

```text
bugfix/redmine-8594-instala-facil-standby-lista-vazia
```

Alternativa no padrao com prefixo `fix`:

```text
fix/redmine-8594-instala-facil-standby-lista-vazia
```

Base da branch:

```text
main
```

## 4. Implementacao

Pelo historico do PDF, a correcao registrada foi:

```text
Colocada uma verificacao de lista vazia antes de salvar o lineup.
```

No fluxo real, a implementacao deveria ser acompanhada de uma revisao tecnica dos pontos afetados, por exemplo:

- fluxo de retorno do stand-by;
- estado da tela Instala Facil;
- verificacao de lista de canais vazia;
- persistencia/salvamento do lineup;
- interacao com deteccao de LNBF;
- impacto sobre busca automatica de canais.

Commit exemplo:

```text
fix: avoid saving empty lineup after standby refs #8594
```

Ou, em portugues:

```text
fix: evita salvar lista vazia apos standby refs #8594
```

## 5. Merge Request para main

Titulo sugerido:

```text
[Redmine #8594] Instala Facil: evitar montagem automatica de lista apos stand-by
```

Descricao sugerida:

```md
## Redmine
Refs #8594

## Objetivo
Corrigir comportamento em que o receptor monta a lista de canais automaticamente ao retornar do stand-by na tela do Instala Facil com lista vazia.

## O que mudou
- Adicionada verificacao de lista vazia antes de salvar o lineup.
- Preservado retorno para a tela do Instala Facil quando nao ha lista instalada.

## Motivo
O receptor nao deve criar/salvar lista de canais automaticamente ao sair do stand-by quando o usuario ainda esta no fluxo de Instala Facil e nao possui lista instalada.

## Como validar
- [ ] Atualizar receptor para v1.04 ou build com a correcao.
- [ ] Executar padrao de fabrica.
- [ ] Conectar cabo de sinal.
- [ ] Aguardar tela do Instala Facil.
- [ ] Colocar receptor em stand-by.
- [ ] Retirar receptor do stand-by.
- [ ] Confirmar que retorna para Instala Facil.
- [ ] Confirmar que lista de canais nao foi montada automaticamente.
- [ ] Repetir com LNBF detectado antes do stand-by.
- [ ] Repetir sem LNBF detectado antes do stand-by.

## Documentacao
- [x] Nao exige documentacao de usuario imediata
- [ ] Atualizar documentacao existente
- [ ] Criar novo documento
- [ ] Atualizar Doxygen

## Riscos
- Fluxo de Instala Facil.
- Persistencia da lista de canais.
- Retorno de stand-by.
- Cen?rios com LNBF detectado e nao detectado.

## Evidencias
- Build testado:
- Resultado QA:
- Logs, se houver:
```

## 6. Revisao tecnica

O revisor deve validar:

- A correcao esta restrita ao problema da issue.
- Nao ha regressao no fluxo normal de busca/lista de canais.
- O receptor nao salva lista vazia indevidamente.
- O estado da tela Instala Facil e restaurado corretamente.
- O comportamento com LNBF detectado e nao detectado foi considerado.
- O plano de teste e suficiente para QA.

## 7. Merge em main

Apos aprovacao:

```text
MR aprovado -> merge em main
```

Atualizacao sugerida no Redmine:

```md
Correcao integrada em main.

Branch: bugfix/redmine-8594-instala-facil-standby-lista-vazia
MR: <link do MR>
Commit: <hash>
Resumo: adicionada verificacao de lista vazia antes de salvar o lineup.
Status: Em validacao / Pending test
Como validar: seguir roteiro descrito no MR.
```

## 8. Validacao QA

Roteiro QA minimo:

| Cenario | Esperado |
| --- | --- |
| Lista vazia, tela Instala Facil, entra e sai de stand-by | Retorna para Instala Facil e nao monta lista automaticamente |
| LNBF nao detectado antes do stand-by | Nao deve salvar lista invalida/vazia; comportamento deve permanecer consistente |
| LNBF detectado antes do stand-by | Canais devem abrir normalmente apos retorno, sem regressao |
| Fluxo normal de busca/instalacao sem stand-by | Deve continuar funcionando |
| Padrao de fabrica + repeticao do fluxo | Resultado deve ser reproduzivel e consistente |

Status Redmine apos QA aprovado:

```text
Em validacao -> Resolvido
```

## 9. Promocao para release v1.04

Como a issue tem target version `v1.04`, ela deve entrar no conjunto de mudancas promovidas para release.

Fluxo:

```text
main com #8594 integrado
-> abrir MR de main para stable
-> revisar lista de tarefas do release v1.04
-> validar release candidato
-> merge em stable
-> gerar tag na stable
-> registrar evidencias no MR e no Redmine
-> gerar pacote a partir da stable
```

Tag exemplo:

```text
v1.04.x
```

A tag exata deve seguir o padrao de versionamento real do projeto.

## 10. Evidencias de release

No MR de promocao para `stable`, registrar:

```md
## Release
Versao: v1.04
Branch origem: main
Branch destino: stable
Tag: <tag gerada na stable>

## Issues incluidas
- Redmine #8594 - Receptor Monta Lista de Canais Sozinho

## Evidencias
- Build gerado:
- Pacote .b8:
- Pacote .bin, se aplicavel:
- OTA .ts, se aplicavel:
- Resultado QA:
- Responsavel:
- Data:
```

No Redmine #8594, registrar:

```md
Entrega validada e incluida no release.

MR correcao: <link>
MR main -> stable: <link>
Tag stable: <tag>
Build/release: <identificacao>
Resultado QA: aprovado
Documentacao: nao aplicavel para usuario final; fluxo coberto pelo processo Redmine/GitLab.
```

Status final:

```text
Resolvido -> Fechado
```

## 11. Impacto em documentacao

Para esta issue especifica, a principio nao parece necessario criar documento novo de produto, pois e uma correcao de bug localizada.

Classificacao pela politica leve:

```text
Nivel 1 ou Nivel 2
```

Recomendacao:

- Nivel 1: registrar no MR e Redmine se a correcao nao muda procedimento ou comportamento esperado ja conhecido.
- Nivel 2: atualizar documento existente se houver pagina sobre `Instala Facil`, `stand-by` ou `lista de canais`.

Se futuramente existir um documento como:

```text
docs/features/instala-facil.md
docs/features/standby.md
docs/features/channel-list.md
```

Adicionar uma nota de regra funcional:

```md
Quando o receptor retorna de stand-by durante o Instala Facil e ainda nao possui lista de canais instalada, ele deve permanecer no fluxo do Instala Facil e nao deve salvar/montar lista vazia automaticamente.
```

## Resumo do exemplo

```text
Issue #8594 nasce no Redmine
-> dev confirma entendimento
-> branch bugfix a partir da main
-> MR para main com correcao e roteiro de teste
-> QA valida o comportamento
-> main e promovida para stable no release v1.04
-> tag e evidencias sao registradas no MR/Redmine
-> release e gerado a partir da stable
-> issue e fechada com rastreabilidade completa
```

