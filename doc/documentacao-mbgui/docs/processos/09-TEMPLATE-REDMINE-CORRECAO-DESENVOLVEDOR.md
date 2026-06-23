# Template Redmine - Descrição Tarefa Finalizada

Use este modelo no Redmine depois de finalizar a tarefa. O objetivo e registrar o que foi corrigido, onde foi alterado, como testar e quais riscos precisam ser observados no novo build.

## Documentos relacionados

- [Fluxo de Desenvolvimento: Redmine, GitLab, MR e Release](fluxo-gitlab.md)
- [Template Redmine - Bug do Testador](TEMPLATE-REDMINE-BUG-TESTADOR.md)

## Quando usar

Use este template no resultado da tarefa, depois que a correcao foi implementada e antes de enviar para validacao, release ou fechamento.

## 1. Resumo da correção

```text
Descreva em poucas linhas qual era o problema e qual foi a correcao aplicada.
Exemplo: Corrigido o fluxo de atualizacao da lista no Instala Facil para evitar acesso a ponteiro invalido quando a busca e cancelada antes de finalizar.
```

## 2. Causa identificada

```text
Explique de forma objetiva o motivo do bug.
```

## 3. Como validar a correção

### 3.1 Passos principais

1. `<passo 1>`
2. `<passo 2>`
3. `<passo 3>`
4. `<passo 4>`

### 3.2 Resultado esperado após a correção

```text
Descreva o comportamento esperado apos aplicar a correcao.
```

### 3.3 Testes realizados pelo desenvolvedor

Marque o que foi feito:

- [ ] Reproduzi o bug antes da correcao
- [ ] Validei o fluxo corrigido depois da correcao
- [ ] Validei regressão em fluxo relacionado
- [ ] Validei com reset de fabrica
- [ ] Validei com troca de operadora/satelite
- [ ] Nao consegui reproduzir localmente

Detalhes dos testes:

```text
Informe ambiente, build, equipamento, banco/lista e resultado dos testes feitos.
```

## 4. Riscos e pontos de atenção para QA

```text
Informe quais fluxos podem ter sido impactados e merecem teste de regressao.
Exemplo: Como a correcao alterou a chave de identificacao do canal, validar favoritos, regionalizacao, lista de canais e troca de operadora.
```

## 5. Evidencias anexadas

- **Video/foto da validacao:** `<anexo/link>`
- **Print da tela:** `<anexo/link>`
## 6. Observações finais

```text
Informe limitacoes, dependencias, pendencias ou novas tarefas que devem ser abertas.
```

##  7. Template para Redmine (wiki syntax)

```
*1. Resumo da correção*
_Descreva em poucas linhas qual era o problema e qual foi a correcao aplicada_

*2. Causa identificada*
_Descreva o comportamento esperado apos aplicar a correcao._

*3. Como validar a correção*
3.1 Passos principais*
# _a_
# _b_
# _c_
...

*3.2 Resultado esperado após a correção*
_Descreva o comportamento esperado apos aplicar a correcao._

*3.3 Testes realizados pelo desenvolvedor*
_Informe ambiente, build, equipamento, banco/lista e resultado dos testes feitos._

*4. Riscos e pontos de atenção para QA*
_Informe quais fluxos podem ter sido impactados e merecem teste de regressão._

*5. Evidencias anexadas*
* _Foto ou video do problema: <anexo/link>_
* _Print da tela: <anexo/link>_

*6. Observações finais* 
_Informe limitacoes, dependencias, pendencias ou novas tarefas que devem ser abertas._
```
